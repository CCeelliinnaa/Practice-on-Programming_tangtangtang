#include "tower.h"
#include "bullet.h"
#include <QPainter>
#include <QtMath>
#include <QTime>

int Tower::s_gameSpeed = 1;

// 炮塔类构造函数实现
Tower::Tower(Type type, const QPoint& pos)
    : towerType(type), position(pos), level(1), rotationAngle(0)
{
    // 不再对 COFFEE 偏移 X，所有炮塔的初始位置均为坑的左上角
    // 实际中心点 = position + (40,40)（坑大小为 80x80）
    // 根据不同的炮塔类型，初始化炮塔的属性和贴图
    switch (type) {
    case COFFEE:
        pixmap.load(":/images/images/late_night_beginner_cutout.png");
        upgradedPixmap.load(":/images/images/late_night_upgrade_cutout.png");
        range = 200;
        damage = 40;
        upgradeCost = 180;
        sellPrice = 50;
        break;
    case MUSIC:
        pixmap.load(":/images/images/monster_1.png");
        upgradedPixmap.load(":/images/images/monster_2.png");
        range = 200;
        damage = 48;
        upgradeCost = 180;
        sellPrice = 50;
        break;
    case AI:
        pixmap.load(":/images/images/ai_beginner.png");
        upgradedPixmap.load(":/images/images/ai_upgrade.png");
        range = 240;
        damage = 84;
        upgradeCost = 180;
        sellPrice = 50;
        break;
    case TEACHER:
        pixmap.load(":/images/images/help_beginner_cutout.png");
        upgradedPixmap.load(":/images/images/help_upgrade_cutout.png");
        range = 240;
        damage = 84;
        upgradeCost = 180;
        sellPrice = 50;
        break;
    }
    // 统一设置旋转中心为坑的中心（position + 40,40）
    rotationCenter = QPointF(position.x() + 40, position.y() + 40);
    // 初始化上次发射子弹的时间
    lastShotTime = QTime::currentTime();
}

// 炮塔类析构函数实现，释放子弹资源
Tower::~Tower()
{
    for (Bullet* bullet : bullets) {
        delete bullet;
    }
}

// 返回炮塔实际中心点（与 rotationCenter 相同）
QPointF Tower::centerPosition() const
{
    return rotationCenter;
}

// 绘制炮塔的方法实现
void Tower::draw(QPainter* painter, const QList<Enemy *>& enemies, Obstacle* obstacle)
{
    QPointF targetPos;
    Enemy* targetEnemy = nullptr;
    double maxTraveledDistance = 0;

    QPointF center = centerPosition();

    // 寻找射程范围内的目标（基于中心点计算距离）
    for (Enemy* enemy : enemies) {
        if ((enemy->getPosition() - center).manhattanLength() <= range) {
            double traveledDistance = enemy->getTraveledDistance();
            if (traveledDistance > maxTraveledDistance) {
                maxTraveledDistance = traveledDistance;
                targetEnemy = enemy;
            }
        }
    }
    if (targetEnemy) {
        targetPos = targetEnemy->getPosition();
    } else if (obstacle && isInRange(obstacle->getPosition())) {
        targetPos = obstacle->getPosition();
    }

    // 计算旋转角度（基于中心点）
    if (!targetPos.isNull()) {
        QPointF diff = targetPos - centerPosition();
        rotationAngle = qRadiansToDegrees(qAtan2(diff.x(), -diff.y()));
    }

    // 绘制炮塔
    painter->save();
    if (towerType == COFFEE) {
        painter->translate(rotationCenter);
        painter->rotate(rotationAngle);
        // 根据等级绘制，贴图偏移 -40 使其居中
        QPixmap scaledPix = (level > 1 ? upgradedPixmap : pixmap).scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter->drawPixmap(-scaledPix.width() / 2, -scaledPix.height() / 2, scaledPix);
    } else {
        // 非咖啡塔直接绘制在坑左上角，视觉上同样填满坑
        if (level > 1) {
            painter->drawPixmap(position.x(), position.y(), upgradedPixmap.scaled(80, 80, Qt::KeepAspectRatio));
        } else {
            painter->drawPixmap(position.x(), position.y(), pixmap.scaled(80, 80, Qt::KeepAspectRatio));
        }
    }
    painter->restore();
}

// 升级炮塔的方法实现
void Tower::upgrade()
{
    level++;
    damage += 36;
    range += 30;
    upgradeCost += 30;
    // 旋转中心不变（仍为坑中心），无需特别调整
}

// 炮塔攻击敌人的方法实现
void Tower::attack(const QList<Enemy *>& enemies)
{
    int interval = 0;
    switch (towerType) {
    case COFFEE: interval = 500; break;
    case MUSIC:  interval = 1000; break;
    case AI:     interval = 1500; break;
    case TEACHER: interval = 2000; break;
    }

    if (lastShotTime.msecsTo(QTime::currentTime()) >= interval / s_gameSpeed) {
        Enemy* targetEnemy = nullptr;
        double maxTraveledDistance = 0;
        QPointF center = centerPosition();

        for (Enemy* enemy : enemies) {
            if ((enemy->getPosition() - center).manhattanLength() <= range) {
                double traveledDistance = enemy->getTraveledDistance();
                if (traveledDistance > maxTraveledDistance) {
                    maxTraveledDistance = traveledDistance;
                    targetEnemy = enemy;
                }
            }
        }

        if (targetEnemy) {
            QString bulletImagePath;
            int bulletWidth = 80, bulletHeight = 80;
            switch (towerType) {
            case COFFEE: bulletImagePath = ":/images/images/bullet_late_night_cutout.png"; bulletWidth = 20; bulletHeight = 20; break;
            case MUSIC:  bulletImagePath = ":/images/images/bullet.png"; break;
            case AI:    bulletImagePath = ":/images/images/ai_bullet_cutout.png"; break;
            case TEACHER: bulletImagePath = ":/images/images/bullet_seniors_cutout.png"; break;
            }

            QPointF center = centerPosition();
            QPointF targetPos = targetEnemy->getPosition();
            QPointF dir = targetPos - center;
            double len = qSqrt(dir.x()*dir.x() + dir.y()*dir.y());
            if (len > 0) {
                dir /= len;  // 单位向量
            }
            QPointF bulletStartPos = center + dir * 40; // 从中心向目标方向偏移 40 像素

            bullets.push_back(new Bullet(bulletStartPos.toPoint(), targetEnemy, damage, bulletImagePath, bulletWidth, bulletHeight, towerType == MUSIC));
            // 同时更新 rotationAngle 以便绘制旋转（如果需要保持炮管正确指向）
            rotationAngle = qRadiansToDegrees(qAtan2(dir.x(), -dir.y())); // 保持炮管绘制方向与目标一致
            lastShotTime = QTime::currentTime();
        }
    }
}

// 炮塔攻击障碍物的方法实现
void Tower::attackObstacle(Obstacle* obstacle)
{
    int interval = 0;
    switch (towerType) {
    case COFFEE: interval = 1500; break;
    case MUSIC:  interval = 2000; break;
    case AI:     interval = 2000; break;
    case TEACHER: interval = 2000; break;
    }

    if (lastShotTime.msecsTo(QTime::currentTime()) >= interval / s_gameSpeed) {
        if (obstacle && isInRange(obstacle->getPosition())) {
            QString bulletImagePath;
            int bulletWidth = 80, bulletHeight = 80;
            switch (towerType) {
            case COFFEE: bulletImagePath = ":/images/images/bullet_late_night_cutout.png"; bulletWidth = 20; bulletHeight = 20; break;
            case MUSIC:  bulletImagePath = ":/images/images/bullet.png"; break;
            case AI:    bulletImagePath = ":/images/images/ai_bullet_cutout.png"; break;
            case TEACHER: bulletImagePath = ":/images/images/bullet_seniors_cutout.png"; break;
            }

            QPointF center = centerPosition();
            QPointF targetPos = obstacle->getPosition();
            QPointF dir = targetPos - center;
            double len = qSqrt(dir.x()*dir.x() + dir.y()*dir.y());
            if (len > 0) {
                dir /= len;
            }
            QPointF bulletStartPos = center + dir * 40;
            bullets.push_back(new Bullet(bulletStartPos.toPoint(), obstacle, damage, bulletImagePath, bulletWidth, bulletHeight));
            rotationAngle = qRadiansToDegrees(qAtan2(dir.x(), -dir.y()));
            lastShotTime = QTime::currentTime();
        }
    }
}

// 更新子弹状态，与之前一致
void Tower::updateBullets()
{
    for (auto it = bullets.begin(); it != bullets.end(); ) {
        Bullet* bullet = *it;
        bullet->move();
        if (bullet->isHit()) {
            delete bullet;
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }
}

int Tower::getUpgradeCost() const { return upgradeCost; }
int Tower::getSellPrice() const { return sellPrice * 0.5; }
QPoint Tower::getPosition() const { return position; }   // 仍返回坑左上角（用于原有UI判定）

QPixmap Tower::getPixmap() const
{
    return (level > 1) ? upgradedPixmap : pixmap;
}

int Tower::getRange() const { return range; }

int Tower::getCost(Type type)
{
    switch (type) {
    case COFFEE:  return 100;
    case MUSIC:   return 120;
    case AI:      return 160;
    case TEACHER: return 180;
    default:     return 0;
    }
}

const std::vector<Bullet*>& Tower::getBullets() const { return bullets; }

bool Tower::isInRange(const QPointF& targetPos) const
{
    QPointF center = centerPosition();
    qreal dx = targetPos.x() - center.x();
    qreal dy = targetPos.y() - center.y();
    return qSqrt(dx*dx + dy*dy) <= range;
}

int Tower::get_level() const { return level; }

void Tower::setGameSpeed(int speed) {
    if (speed >= 1) s_gameSpeed = speed;
}

int Tower::gameSpeed() {
    return s_gameSpeed;
}