#include "unit.h"

Unit::Unit(int x, int y, int _hp, int _atk, int _range, QString _path)
    : pos(x, y), hp(_hp), atk(_atk), range(_range), path(_path)
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

Enemy::Enemy(int _hp, int _atk, int _range, QString _path, QVector<Position> p)
    : Unit(p[0].x, p[0].y, _hp, _atk, _range, _path)
{
    enemyRoad = p;
}

bool Enemy::move(const Map& map)
{
    if(enemyRoad.empty()) //走到路径尽头
    {
        alive = false;
        return false;
    }
    if(map(enemyRoad[0].x, enemyRoad[0].y) == MELEETOWER_VALUE)
        return true;
    //更新坐标
    pos.x = enemyRoad[0].x;
    pos.y = enemyRoad[0].y;
    enemyRoad.erase(enemyRoad.begin()); //删去路径初始点
    return true;
}

MeleeTower::MeleeTower(int x, int y, int _hp, int _atk, int _range, QString _path)
    : Unit(x, y, _hp, _atk, _range, _path)
{

}

RemoteTower::RemoteTower(int x, int y, int _hp, int _atk, int _range, QString _path)
    : Unit(x, y, _hp, _atk, _range, _path)
{

}
