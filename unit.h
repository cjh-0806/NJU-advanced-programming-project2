#ifndef UNIT_H
#define UNIT_H

#include "map.h"

#define UNIT_SPEED 100

class Unit //单位基类
{
protected:
    Position pos; //在二维数组里的坐标
    int hp;
    int atk;
    int range;
    bool alive;
    QString path; //图片存放路径
public:
    Unit(int x, int y, int _hp, int _atk, int _range, QString _path);

    Position get_pos() const { return pos; }
    int get_x() const { return pos.x; }
    int get_y() const { return pos.y; }
    int get_atk() const { return atk; }
    bool isAlive() const { return alive; }
    QString get_path() const { return path; }

    bool inRange(Position _pos);
    bool attack(Unit* target);
    void hurted(Unit* attacker);
};

class Enemy: public Unit //敌人
{
    QVector<Position> enemyRoad; //敌人经过路径
public:
    Enemy(int _hp, int _atk, int _range, QString _path, QVector<Position> p);

    bool move(const Map& map);
};

class MeleeTower: public Unit //近战塔
{
public:
    MeleeTower(int x, int y, int _hp, int _atk, int _range, QString _path);
};

class RemoteTower: public Unit //远程塔
{
public:
    RemoteTower(int x, int y, int _hp, int _atk, int _range, QString _path);
};

#endif // UNIT_H
