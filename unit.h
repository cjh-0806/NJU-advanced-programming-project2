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
    bool frozen; //是否被冰冻
    bool bleed; //是否被放血
public:
    int frozenTimer = 0;
    int bleedTimer = 0;
    Enemy(int _hp, int _atk, int _range, QString _path, QVector<Position> p);
    bool move(const Map& map);

    bool get_frozen() const { return frozen; }
    bool get_bleed() const { return bleed; }

    void set_frozen(bool b) { frozen = b; }
    void set_bleed(bool b) { bleed = b; }
};

class MeleeTower: public Unit //近战塔
{
    int affixCount;
    bool rage;
    bool frozen;
    bool aoe;
public:
    MeleeTower(int x, int y, int _hp, int _atk, int _range, QString _path);
    int get_count() const { return affixCount; }
    bool get_rage() const { return rage; }
    bool get_frozen() const { return frozen; }
    bool get_aoe() const { return aoe; }

    void add_count() { affixCount++; }
    void dec_count() { affixCount--; }
    void add_rage();
    void dec_rage();
    void add_frozen();
    void dec_frozen();
    void add_aoe();
    void dec_aoe();
};

class RemoteTower: public Unit //远程塔
{
    int affixCount;
    bool bleed;
public:
    RemoteTower(int x, int y, int _hp, int _atk, int _range, QString _path);
    int get_count() const { return affixCount; }
    bool get_bleed() const { return bleed; }

    void add_count() { affixCount++; }
    void dec_count() { affixCount--; }
    void add_bleed();
    void dec_bleed();
};

#endif // UNIT_H
