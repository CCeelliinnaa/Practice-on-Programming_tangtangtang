#include "enemy.h"
#include"QtMath"
Enemy::Enemy(Type type)
    : type(type), isFirstPixmap(true), currentPathIndex(0), m_isSlowed(false)
{
    switch (type) {
    case Monster1:
        speed = 4;
        originalSpeed = 4;
        health = 500;
        maxHealth = 500;
        enemyPixmap.load(":/images/images/ddl.png");
        enemyPixmap2.load(":/images/images/monster1_2.png"); // 加载第二张贴图
        pathPoints << QPointF(400, 200)   // 起点
                   << QPointF(930, 200)  // 右移
                   << QPointF(930, 400)  // 下移
                   << QPointF(210, 400)  // 左移
                   << QPointF(210, 600)  // 下移
                   << QPointF(700, 600); // 终点
        position = pathPoints[0];
        break;
    case Monster2:
        speed = 6;
        originalSpeed = 6;
        health = 500;
        maxHealth = 500;
        enemyPixmap.load(":/images/images/pre_1.png");
        enemyPixmap2.load(":/images/images/monster2_2.png"); // 加载第二张贴图
        pathPoints << QPointF(400, 240)   // 起点
                   << QPointF(900, 240)  // 右移
                   << QPointF(900, 410)  // 下移
                   << QPointF(210, 410)  // 左移
                   << QPointF(210, 650)  // 下移
                   << QPointF(700, 650); // 终点
        position = pathPoints[0];
        break;
    case Monster3:
        speed = 4;
        originalSpeed = 4;
        health = 600;
        maxHealth = 600;
        enemyPixmap.load(":/images/images/mid_term.png");
        enemyPixmap2.load(":/images/images/monster3_2.png"); // 加载第二张贴图
        pathPoints << QPointF(400, 200)   // 起点
                   << QPointF(930, 200)  // 右移
                   << QPointF(930, 400)  // 下移
                   << QPointF(210, 400)  // 左移
                   << QPointF(210, 650)  // 下移
                   << QPointF(700, 650); // 终点
        position = pathPoints[0];
        break;
    case Monster4:
            speed = 3.5;
            originalSpeed = 3.5;
            health = 600;
            maxHealth = 600;
            enemyPixmap.load(":/images/images/pre.png"); // 新怪兽第一张贴图
            enemyPixmap2.load(":/images/images/monster4_2.png"); // 新怪兽第二张贴图
            pathPoints << QPointF(400, 200)   // 起点
                       << QPointF(920, 200)  // 右移
                       << QPointF(920, 420)  // 下移
                       << QPointF(220, 420)  // 左移
                       << QPointF(220, 620)  // 下移
                       << QPointF(700, 620); // 终点
            position = pathPoints[0];
            break;
        case Monster5:
            speed = 4;
            originalSpeed = 4;
            health = 600;
            maxHealth = 600;
            enemyPixmap.load(":/images/images/ddl_1.png"); // 新怪兽第一张贴图
            enemyPixmap2.load(":/images/images/monster5_2.png"); // 新怪兽第二张贴图
            pathPoints << QPointF(400, 200)   // 起点
                       << QPointF(910, 200)  // 右移
                       << QPointF(910, 400)  // 下移
                       << QPointF(230, 400)  // 左移
                       << QPointF(230, 600)  // 下移
                       << QPointF(700, 600); // 终点
            position = pathPoints[0];
            break;
    case MonsterBoss:
        speed = 2;
        originalSpeed = 2;
        health = 8000;
        maxHealth = 8000;
        enemyPixmap.load(":/images/images/fail.png");
        enemyPixmap2.load(":/images/images/monsterboss.png"); // 加载第二张贴图
        pathPoints << QPointF(400, 200)   // 起点
                   << QPointF(900, 200)  // 右移
                   << QPointF(900, 400)  // 下移
                   << QPointF(210, 400)  // 左移
                   << QPointF(210, 650)  // 下移
                   << QPointF(700, 650); // 终点
        position = pathPoints[0];
        break;
    }
    enemyPixmap = enemyPixmap.scaled(80, 80, Qt::KeepAspectRatio);
    enemyPixmap2 = enemyPixmap2.scaled(80, 80, Qt::KeepAspectRatio);
}

Enemy::~Enemy()
{
}

//怪物根据预设的路径点进行移动
void Enemy::move()
{
    // 减速状态检查：如果减速时间已结束，恢复原始速度
    if (m_isSlowed && QTime::currentTime() >= m_slowEndTime) {
        speed = originalSpeed;
        m_isSlowed = false;
    }

    // 检查是否还有路径点需要移动（确保不会越界）
    if (currentPathIndex < pathPoints.size()) {
        // 获取当前要移动到的目标点
        QPointF target = pathPoints[currentPathIndex];
        // 处理敌人在 X 轴上的移动
        if (position.x() < target.x()) {
            // 如果敌人在目标点左侧，则向右移动
            // 使用平滑移动算法std::min确保不会超过目标点（防止移动过头）
            position.setX(std::min(position.x() + speed, target.x()));
        }
        else if (position.x() > target.x()) {
            // 如果敌人在目标点右侧，则向左移动
            // 使用std::max确保不会超过目标点
            position.setX(std::max(position.x() - speed, target.x()));
        }
        // 处理敌人在 Y 轴上的移动（逻辑与 X 轴相同）
        if (position.y() < target.y()) {
            position.setY(std::min(position.y() + speed, target.y()));
        }
        else if (position.y() > target.y()) {
            position.setY(std::max(position.y() - speed, target.y()));
        }
        // 当敌人到达当前目标点时，更新索引指向下一个路径点
        if (position == target) {
            currentPathIndex++;
        }
    }
    switchPixmap(); // 移动时切换贴图以实现动态效果
}

void Enemy::draw(QPainter* painter)
{
    QPixmap currentPixmap = enemyPixmap;//贴图状态
    int width = 80, height = 80;
    if (type == MonsterBoss) {
        width = 120;
        height = 120;
    }
    painter->drawPixmap(position.x(), position.y(), currentPixmap.scaled(width, height, Qt::KeepAspectRatio));

    // 绘制血条
    int barWidth = width;
    int barHeight = 6;
    int barX = position.x();
    int barY = position.y() - barHeight - 2;
    double healthRatio = static_cast<double>(health) / maxHealth;

    // 血条背景（灰色）
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(80, 80, 80, 180));
    painter->drawRect(barX, barY, barWidth, barHeight);

    // 血条前景（绿色→黄色→红色）
    QColor healthColor;
    if (healthRatio > 0.6) {
        healthColor = QColor(0, 200, 0);
    } else if (healthRatio > 0.3) {
        healthColor = QColor(255, 165, 0);
    } else {
        healthColor = QColor(255, 0, 0);
    }
    painter->setBrush(healthColor);
    painter->drawRect(barX, barY, static_cast<int>(barWidth * healthRatio), barHeight);

    // 减速效果视觉提示：蓝色半透明覆盖
    if (m_isSlowed) {
        painter->setBrush(QColor(0, 100, 255, 80));
        painter->setPen(Qt::NoPen);
        painter->drawRect(static_cast<int>(position.x()), static_cast<int>(position.y()), width, height);
    }
}

void Enemy::takeDamage(int damage)
{
    health -= damage;
}

int Enemy::getHealth() const
{
    return health;
}

QPointF Enemy::getPosition() const
{
    return position;
}
const QVector<QPointF>& Enemy::getPathPoints() const
{
    return pathPoints;
}
Enemy::Type Enemy::getType() const
{
    return type;
}
void Enemy::setSpeed(int newSpeed)
{
    speed = newSpeed;
}
double Enemy::getTraveledDistance() const
{
    // 初始化一个变量 distance 用于存储敌人已经走过的总距离，初始值为 0
    double distance = 0;

    // 遍历敌人已经走过的路径点，currentPathIndex 表示敌人当前所在路径点的索引
    for (int i = 0; i < currentPathIndex; ++i) {
        // 获取当前路径点 p1
        QPointF p1 = pathPoints[i];
        // 获取下一个路径点 p2
        QPointF p2 = pathPoints[i + 1];
        // 计算两点之间的欧几里得距离，并累加到总距离 distance 中
        // 欧几里得距离公式为：sqrt((x2 - x1)^2 + (y2 - y1)^2)
        distance += qSqrt(qPow(p2.x() - p1.x(), 2) + qPow(p2.y() - p1.y(), 2));
    }

    // 检查敌人是否还未到达路径的最后一个点
    if (currentPathIndex < pathPoints.size() - 1) {
        // 获取敌人当前所在的路径点 p1
        QPointF p1 = pathPoints[currentPathIndex];
        // 获取敌人的当前位置 p2
        QPointF p2 = position;
        // 计算从当前路径点到敌人当前位置的欧几里得距离，并累加到总距离 distance 中
        distance += qSqrt(qPow(p2.x() - p1.x(), 2) + qPow(p2.y() - p1.y(), 2));
    }

    // 返回敌人已经走过的总距离
    return distance;
}
void Enemy::switchPixmap()
{
    isFirstPixmap = !isFirstPixmap; // 切换贴图标记
}

int Enemy::getMaxHealth() const
{
    return maxHealth;
}

void Enemy::addHealth(int amount)
{
    health += amount;
    maxHealth += amount;
}

void Enemy::applySlow(double factor, int durationMs)
{
    speed = originalSpeed * factor;
    m_isSlowed = true;
    m_slowEndTime = QTime::currentTime().addMSecs(durationMs);
}

bool Enemy::isSlowed() const
{
    return m_isSlowed;
}

double Enemy::getOriginalSpeed() const
{
    return originalSpeed;
}
