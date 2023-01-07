#include "unit.h"

Unit::Unit(int x, int y, QString _path)
    : pos(x, y), path(_path)
{
    alive = true;
    hp = sumhp = 100;
    atk = 10;
    range = 1;
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


Enemy::Enemy(QString _path, QVector<Position> p, bool f, bool s)
    : Unit(p[0].x, p[0].y, _path)
{
    enemyRoad = p;
    sumatk = 10;
    frozen = bleed = weaken = false;
    flash = f;
    speedup = s;
    frozenTimer = bleedTimer = flashTimer = weakenTimer = 0;
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
        else //携带闪现词缀
        {
            flashTimer++;
            if(flashTimer == 10) //冷却结束，越过近战塔
            {
                flashTimer = 0;
                pos.x = enemyRoad[0].x;
                pos.y = enemyRoad[0].y;
                enemyRoad.erase(enemyRoad.begin()); //删去路径初始点
                if(enemyRoad.empty()) //走到路径尽头
                {
                    alive = false;
                    return false;
                }
            }
            else
                return true;
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
        if(map(enemyRoad[0].x, enemyRoad[0].y) != MELEETOWER_VALUE)
        {
            pos.x = enemyRoad[0].x;
            pos.y = enemyRoad[0].y;
            enemyRoad.erase(enemyRoad.begin()); //删去路径初始点
        }
    }
    return true;
}

void Enemy::add_weaken()
{
    weaken = true;
    atk = sumatk / 2;
}

void Enemy::dec_weaken()
{
    weaken = false;
    atk = sumatk;
}

void Enemy::dec_hp()
{
    hp -= 5;
    if(hp <= 0)
        alive = false;
}


MeleeTower::MeleeTower(int x, int y, QString _path)
    : Unit(x, y, _path)
{
    affixCount = 0;
    rage = frozen = aoe = avoid = false;
    avoidTimer = 0;
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


RangedTower::RangedTower(int x, int y, QString _path)
    : Unit(x, y, _path)
{
    range = 3;
    affixCount = 0;
    aoe = bleed = weaken = false;
}

