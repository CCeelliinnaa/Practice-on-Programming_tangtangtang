#ifndef ENEMY_H
#define ENEMY_H

#include <QPoint>
#include <QPainter>
#include <QPixmap>
#include <QVector> // 用于存储路径点
#include <QTime>

class Enemy
{
public:
    enum Type {
        Monster1,
        Monster2,
        Monster3,
        Monster4,
        Monster5,
        MonsterBoss
    };

    Enemy(Type type);
    ~Enemy();

    void move();
    void draw(QPainter* painter);
    void takeDamage(int damage);
    int getHealth() const;
    QPointF getPosition() const;
    const QVector<QPointF>& getPathPoints() const;
    Type getType() const;
    void setSpeed(int newSpeed);
    double getTraveledDistance() const;
    void switchPixmap();
    int getMaxHealth() const;
    void addHealth(int amount);
    void applySlow(double factor, int durationMs); // 减速效果
    bool isSlowed() const;
    double getOriginalSpeed() const;
private:
    Type type;
    QPointF position;
    double speed;
    double originalSpeed;       // 原始速度
    int health;
    int maxHealth;
    QPixmap enemyPixmap;
    QPixmap enemyPixmap2;
    bool isFirstPixmap;
    QVector<QPointF> pathPoints;
    int currentPathIndex;
    bool m_isSlowed;            // 是否处于减速状态
    QTime m_slowEndTime;       // 减速结束时间
};

#endif // ENEMY_H
