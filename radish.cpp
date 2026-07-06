#include "radish.h"

Radish::Radish(const QPoint& position)
    : position(position), health(10)
{
    // 加载萝卜不同生命状态的贴图
    for (int i = 1; i <= 10; ++i) {
        QString filename = QString(":/images/images/gpaa_%1.png").arg(i);
        QPixmap pixmap;
        pixmap.load(filename);
        radishPixmaps.append(pixmap);
    }


}

Radish::~Radish()
{
}

void Radish::draw(QPainter* painter)
{
    if (health > 0) {
        // 绘制萝卜贴图
        QPixmap radishPixmap = radishPixmaps[health - 1].scaled(100, 100,Qt::KeepAspectRatio);
        painter->drawPixmap(position.x(), position.y(), radishPixmap);
    }
}

void Radish::takeDamage(double damage)
{
    health -= damage;
    if (health < 0) {
        health = 0;
    }
}

int Radish::getHealth() const
{
    return health;
}
//重置生命值
void Radish::resetHealth() {
    health = 10;
}
