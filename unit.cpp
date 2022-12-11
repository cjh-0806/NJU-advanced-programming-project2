#include "unit.h"

Unit::Unit(int x, int y, int _hp, int _atk, int _range, QString _path)
    : pos(x, y), hp(_hp), sumhp(_hp), atk(_atk), range(_range), path(_path)
{
    alive = true;
}

bool Unit::inRange(Position _pos)
{
    if(abs(pos.x-_pos.x) <= range && abs(pos.y-_pos.y) <= range)
        return true;
    else return false;
}

bool Unit::attack(Unit* target)
{
    if(alive && target->isAlive() && inRange(target->get_pos()))
    {
        target->hurted(this);
        return true;
    }
    else return false;
}

void Unit::hurted(Unit* attacker)
{
    if(!alive)
        return;
    hp -= attacker->get_atk();
    if(hp <= 0)
        alive = false;
}

Enemy::Enemy(int _hp, int _atk, int _range, QString _path, QVector<Position> p, bool f, bool s)
    : Unit(p[0].x, p[0].y, _hp, _atk, _range, _path)
{
    enemyRoad = p;
    frozen = bleed = false;
    flash = f;
    speedup = s;
    frozenTimer = bleedTimer = flashTimer = 0;
}

bool Enemy::move(const Map& map)
{
    if(enemyRoad.empty()) //走到路径尽头
    {
        alive = false;
        return false;
    }
    if(map(enemyRoad[0].x, enemyRoad[0].y) == MELEETOWER_VALUE)
    {
        if(!flash) //没有闪现词缀
            return true;
        else //携带闪现词缀，越过近战塔
        {
            pos.x = enemyRoad[0].x;
            pos.y = enemyRoad[0].y;
            enemyRoad.erase(enemyRoad.begin()); //删去路径初始点
            if(enemyRoad.empty()) //走到路径尽头
            {
                alive = false;
                return false;
            }
        }
    }
    //更新坐标
    pos.x = enemyRoad[0].x;
    pos.y = enemyRoad[0].y;
    enemyRoad.erase(enemyRoad.begin()); //删去路径初始点
    if(speedup) //携带神速词缀，再走一格
    {
        if(enemyRoad.empty()) //走到路径尽头
        {
            alive = false;
            return false;
        }
        //更新坐标
        pos.x = enemyRoad[0].x;
        pos.y = enemyRoad[0].y;
        enemyRoad.erase(enemyRoad.begin()); //删去路径初始点
    }
    return true;
}

void Enemy::dec_hp()
{
    hp -= 5;
    if(hp <= 0)
        alive = false;
}

MeleeTower::MeleeTower(int x, int y, int _hp, int _atk, int _range, QString _path)
    : Unit(x, y, _hp, _atk, _range, _path)
{
    affixCount = 0;
    rage = frozen = aoe = false;
}

void MeleeTower::add_rage()
{
    rage = true;
    range *= 2;
    atk *= 2;
}

void MeleeTower::dec_rage()
{
    rage = false;
    range /= 2;
    atk /= 2;
}

void MeleeTower::add_frozen()
{
    frozen = true;
}

void MeleeTower::dec_frozen()
{
    frozen = false;
}

void MeleeTower::add_aoe()
{
    aoe = true;
}

void MeleeTower::dec_aoe()
{
    aoe = false;
}

RemoteTower::RemoteTower(int x, int y, int _hp, int _atk, int _range, QString _path)
    : Unit(x, y, _hp, _atk, _range, _path)
{
    affixCount = 0;
    bleed = false;
}

void RemoteTower::add_bleed()
{
    bleed = true;
}

void RemoteTower::dec_bleed()
{
    bleed = false;
}
