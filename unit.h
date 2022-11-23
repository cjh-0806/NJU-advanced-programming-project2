#ifndef UNIT_H
#define UNIT_H

#include "map.h"

#define UNIT_SPEED 100

class Unit //单位基类
{
protected:
    Position pos; //在二维数组里的坐标
    int hp; //血量
    int sumhp; //总血量
    int atk; //攻击值
    int range; //攻击范围
    bool alive; //是否存活
    QString path; //图片存放路径
public:
    Unit(int x, int y, int _hp, int _atk, int _range, QString _path);

    Position get_pos() const { return pos; }
    int get_x() const { return pos.x; }
    int get_y() const { return pos.y; }
    int get_hp() const { return hp; }
    int get_sumhp() const { return sumhp; }
    int get_atk() const { return atk; }
    QString get_path() const { return path; }
    bool isAlive() const { return alive; }

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
