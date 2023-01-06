#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <QLabel>
#include <QMessageBox>
#include <QFont>
#include <QSoundEffect>

#include <stdlib.h>
#include <ctime>
#include "map.h"
#include "unit.h"
#include "selectaffix.h"

#define ENEMY_MAX_NUM 50

#define RAGE_AFFIX 0 //狂暴词缀
#define FROZEN_AFFIX 1 //冰冻词缀
#define AOE_AFFIX 2 //群伤词缀
#define AVOID_AFFIX 3 //免伤词缀
#define BLEED_AFFIX 4 //放血词缀
#define WEAKEN_AFFIX 5 //弱化词缀

#define GRASS_PATH ":/pictures/green.jpg"
#define ROAD_PATH ":/pictures/brown.jpg"
#define ENEMY_PATH ":/pictures/enemy.png"
#define ENEMY1_PATH ":/pictures/enemy1.png"
#define ENEMY2_PATH ":/pictures/enemy2.png"
#define MELEETOWER_PATH ":/pictures/tower1.png"
#define RANGEDTOWER_PATH ":/pictures/tower2.png"

class RangedAttackEffect
{
public:
    RangedTower* tower;
    Enemy* enemy;
public:
    RangedAttackEffect(RangedTower* _tower, Enemy* _enemy)
    {
        tower = _tower;
        enemy = _enemy;
    }
};

class MeleeAttackEffect
{
public:
    MeleeTower* tower;
    Enemy* enemy;
public:
    MeleeAttackEffect(MeleeTower* _tower, Enemy* _enemy)
    {
        tower = _tower;
        enemy = _enemy;
    }
};

namespace Ui {
class MapWidget;
}

class MapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent, Map m);
    ~MapWidget();
    void closeEvent(QCloseEvent*);
    void paintEvent(QPaintEvent*); //绘图事件
    void drawMap(QPainter&); //画出地图
    void drawEnemy(QPainter&); //画出敌人
    void drawMeleeTower(QPainter&); //画出近战塔
    void drawRangedTower(QPainter&); //画出远程塔
    void drawAffix(QPainter&); //画出词缀库
    void drawSelectAffix(QPainter&); //画出词缀选择框
    void drawAttackEffect(QPainter&); //画出远程塔攻击效果
    void drawButton(QPainter&); //画出暂停键和音量键

    void mousePressEvent(QMouseEvent*); //鼠标点击事件
    void showTip(Enemy* e);

private:
    Map map; //地图
    int money; //金币数
    QLabel* moneyLabel; //金币标签
    int life; //生命值
    QLabel* lifeLabel; //生命标签
    bool isPlay; //是否在运行
    bool volume; //是否有音量

    QVector<Position> rangedTowerPosVec; //远程塔安置点
    int affixArr[6]; //词缀库，六种词缀
    SelectAffix select; //词缀选择框

    QVector<MeleeTower*> meleeTowerVec; //近战塔数组
    QVector<RangedTower*> rangedTowerVec; //远程塔数组
    QVector<Enemy*> enemyVec; //敌人数组

    QVector<MeleeAttackEffect*> meleeatkVec;
    QVector<RangedAttackEffect*> rangedatkVec;

    int enemyCount; //敌人数量

    QTimer* enemyTimer; //产生敌人的计时器
    QTimer* gameTimer; //游戏的主计时器
    QTimer* moneyTimer; //金币增加的计时器

    Ui::MapWidget *ui;

};

#endif // MAPWIDGET_H
