#include "unit.h"

Unit::Unit(int x, int y, int _hp, int _atk, int _range, QString _path): pos(x, y)
{
    hp = _hp;
    atk = _atk;
    range = _range;
    alive = true;
    path = _path;
}

Enemy::Enemy(int _hp, int _atk, int _range, QString _path, QVector<Position> p)
    : Unit(p[0].x, p[0].y, _hp, _atk, _range, _path)
{
    enemyRoad = p;
}
/*
bool Enemy::inRange(Position _pos)
{
    if (abs(_pos.x - pos.x) <= 1 && abs(_pos.y - pos.y) <= 1)
        return true;
    else
        return false;
}

bool Enemy::attack(Unit* target)
{
    if(inRange(target->get_pos()))
    {
        target->hurted(this);
        return true;
    }
    else
        return false;
}

void Enemy::hurted(Unit* attacker)
{
    hp -= attacker->get_atk();
}
*/
bool Enemy::move()
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
