#ifndef UNIT_H
#define UNIT_H

#include "map.h"

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
    Unit(int x, int y, QString _path);

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
    int sumatk;
    bool frozen; //是否被冰冻
    bool bleed; //是否被放血
    bool weaken; //是否被弱化
    bool flash; //是否携带闪现词缀
    bool speedup; //是否携带神速词缀
public:
    int frozenTimer;
    int bleedTimer;
    int weakenTimer;
    int flashTimer;
    Enemy(QString _path, QVector<Position> p, bool f, bool s);
    bool move(const Map& map);

    bool get_frozen() const { return frozen; }
    bool get_bleed() const { return bleed; }
    bool get_weaken() const { return weaken; }

    void set_frozen(bool b) { frozen = b; }
    void set_bleed(bool b) { bleed = b; }
    void add_weaken();
    void dec_weaken();
    void dec_hp();
};

class MeleeTower: public Unit //近战塔
{
    int affixCount;
    bool rage;
    bool frozen;
    bool aoe;
    bool avoid;
public:
    int avoidTimer;
    MeleeTower(int x, int y, QString _path);
    int get_count() const { return affixCount; }
    bool get_rage() const { return rage; }
    bool get_frozen() const { return frozen; }
    bool get_aoe() const { return aoe; }
    bool get_avoid() const { return avoid; }

    void add_count() { affixCount++; }
    void dec_count() { affixCount--; }
    void add_rage();
    void dec_rage();
    void set_frozen(bool b) { frozen = b; }
    void set_aoe(bool b) { aoe = b; }
    void set_avoid(bool b) { avoid = b; }
};

class RangedTower: public Unit //远程塔
{
    int affixCount;
    bool aoe;
    bool bleed;
    bool weaken;
public:
    RangedTower(int x, int y, QString _path);
    int get_count() const { return affixCount; }
    bool get_aoe() const { return aoe; }
    bool get_bleed() const { return bleed; }
    bool get_weaken() const { return weaken; }

    void add_count() { affixCount++; }
    void dec_count() { affixCount--; }
    void set_aoe(bool b) { aoe = b; }
    void set_bleed(bool b) { bleed = b; }
    void set_weaken(bool b) { weaken = b; }

};

#endif // UNIT_H
