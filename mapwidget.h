#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

#include <stdlib.h>
#include <ctime>
#include "map.h"
#include "unit.h"

namespace Ui {
class MapWidget;
}

class MapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent, Map m);
    ~MapWidget();
    void paintEvent(QPaintEvent*); //绘图事件
    void drawEnemy(QPainter&); //画出敌人
    void drawMeleeTower(QPainter&); //画出近战塔
    void drawRemoteTower(QPainter&); //画出远程塔
    void mousePressEvent(QMouseEvent*); //鼠标点击事件

private:
    Map map; //地图
    QVector<MeleeTower*> meleeTowerVec; //近战塔数组
    QVector<RemoteTower*> remoteTowerVec; //远程塔数组
    QVector<Enemy*> enemyVec; //敌人数组
    int enemyCount; //敌人数量
    Ui::MapWidget *ui;
};

#endif // MAPWIDGET_H
