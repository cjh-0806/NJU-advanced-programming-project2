#include "mapwidget.h"
#include "ui_mapwidget.h"

#define UNIT_LENGTH 100
#define ENEMY_MAX_NUM 5

#define ENEMY1_PATH ":/pictures/enemy1.jpg"
#define MELEETOWER_PATH ":/pictures/tower1.jpg"
#define REMOTETOWER_PATH ":/pictures/tower2.jpg"


MapWidget::MapWidget(QWidget *parent, Map m) :
    QWidget(parent), map(m),
    ui(new Ui::MapWidget)
{
    this->setFixedSize(map.get_m() * UNIT_LENGTH, map.get_n() * UNIT_LENGTH);
    ui->setupUi(this);
    enemyCount = 0;

    //金币标签
    money = 450;
    moneyLabel = new QLabel(this);
    moneyLabel->setGeometry(20, 40, 200, 20);
    moneyLabel->setText("金币数：" + QString::number(money));

    //生命标签
    life = 10;
    lifeLabel = new QLabel(this);
    lifeLabel->setGeometry(20, 70, 200, 20);
    lifeLabel->setText("生命值：" + QString::number(life));

    //远程塔可安置点，有重复
    for(int i = 0; i < map.get_num(); ++i)
        for(int j = 0; j < map.get_road(i).size(); ++j)
        {
            Position p = map.get_pos(i, j);
            if(p.x - 1 >= 0 && map(p.x - 1, p.y) == GRASS_VALUE)
                rmtTowerPosVec.push_back(Position(p.x - 1, p.y));
            if(p.x + 1 < map.get_m() && map(p.x + 1, p.y) == GRASS_VALUE)
                rmtTowerPosVec.push_back(Position(p.x + 1, p.y));
            if(p.y - 1 >= 0 && map(p.x, p.y - 1) == GRASS_VALUE)
                rmtTowerPosVec.push_back(Position(p.x, p.y - 1));
            if(p.y + 1 < map.get_n() && map(p.x, p.y + 1) == GRASS_VALUE)
                rmtTowerPosVec.push_back(Position(p.x, p.y + 1));
        }


    //产生敌人的计时器
    enemyTimer = new QTimer(this);
    enemyTimer->start(2000);
    connect(enemyTimer, &QTimer::timeout, [&]()
    {
        srand((unsigned)time(NULL));
        if(enemyCount < ENEMY_MAX_NUM)
        {
            int index = rand() % map.get_num(); //随机选择一条路径作为敌人路径
            enemyVec.push_back(new Enemy(100, 20, 1, ENEMY1_PATH, map.get_road(index)));
            enemyCount++;
        }
        if(life > 0 && enemyVec.empty()) //击败所有敌人，游戏成功
        {
            enemyTimer->stop();
            QMessageBox::information(this, "结束", "游戏成功！");
            this->close();
        }
    });


    //游戏的主计时器
    gameTimer = new QTimer(this);
    gameTimer->start(1000);
    connect(gameTimer, &QTimer::timeout, [&]()
    {
        //敌人移动
        for(auto enemy = enemyVec.begin(); enemy != enemyVec.end(); )
        {
            if(!(*enemy)->isAlive()) //敌人死亡，删去这个敌人
            {
                enemyVec.erase(enemy);
                money += 20;
                moneyLabel->setText("金币数：" + QString::number(money));
            }
            else if(!(*enemy)->move(map)) //敌人走到路径尽头，删去这个敌人，生命值-1
            {
                enemyVec.erase(enemy);
                life--;
                lifeLabel->setText("生命值：" + QString::number(life));
                if(life == 0) //生命值为0，游戏失败
                {
                    QMessageBox::information(this, "结束", "游戏失败！");
                    this->close();
                }
            }
            else enemy++;
        }

        //近战防御塔攻击
        for(auto tower : meleeTowerVec)
            for(auto enemy : enemyVec)
                tower->attack(enemy);

        //远程防御塔攻击
        for(auto tower : remoteTowerVec)
            for(auto enemy : enemyVec)
                tower->attack(enemy);

        //敌人攻击近战防御塔
        for(auto enemy : enemyVec)
            for(auto tower = meleeTowerVec.begin(); tower != meleeTowerVec.end(); )
            {
                enemy->attack(*tower);
                if(!(*tower)->isAlive())
                {
                    map.set_value((*tower)->get_x(), (*tower)->get_y(), ROAD_VALUE);
                    meleeTowerVec.erase(tower);
                }
                else tower++;
            }
        update();
    });
}


MapWidget::~MapWidget()
{
    delete ui;
}

void MapWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QString grass_path(":/pictures/green.jpg");
    QString road_path(":/pictures/brown.jpg");
    //放置草地和路径
    int i, j;
    for(i = 0; i < map.get_m(); ++i)
        for(j = 0; j < map.get_n(); ++j)
        {
            int x = i * UNIT_LENGTH;
            int y = j * UNIT_LENGTH;
            if(map(i,j) == GRASS_VALUE || map(i, j) == REMOTETOWER_VALUE)
                painter.drawPixmap(x, y, UNIT_LENGTH, UNIT_LENGTH, grass_path);
            if(map(i,j) == ROAD_VALUE || map(i, j) == MELEETOWER_VALUE)
                painter.drawPixmap(x, y, UNIT_LENGTH, UNIT_LENGTH, road_path);
        }
    drawEnemy(painter);
    drawMeleeTower(painter);
    drawRemoteTower(painter);
}

void MapWidget::drawEnemy(QPainter& painter)
{
    for(auto enemy : enemyVec)
    {
        if(enemy->isAlive())
        {
            int x = enemy->get_x() * UNIT_LENGTH + 10;
            int y = enemy->get_y() * UNIT_LENGTH + 10;
            painter.drawPixmap(x, y, 80, 80, enemy->get_path());
        }
    }
}

void MapWidget::drawMeleeTower(QPainter& painter)
{
    for(auto tower : meleeTowerVec)
    {
        if(tower->isAlive())
        {
            int x = tower->get_x() * UNIT_LENGTH + 10;
            int y = tower->get_y() * UNIT_LENGTH + 10;
            painter.drawPixmap(x, y, 80, 80, tower->get_path());
        }
    }
}

void MapWidget::drawRemoteTower(QPainter& painter)
{
    for(auto tower : remoteTowerVec)
    {
        int x = tower->get_x() * UNIT_LENGTH + 10;
        int y = tower->get_y() * UNIT_LENGTH + 10;
        painter.drawPixmap(x, y, 80, 80, tower->get_path());
    }
}

void MapWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) //鼠标左键点击
        return;
    int mx = event->pos().x();
    int my = event->pos().y();
    qDebug() << "按下左键" << mx << my;
    bool flag = true;
    for(int i = 0; i < map.get_num() && flag == true; ++i)
        for(int j = 0; j < map.get_road(i).size() && flag == true; ++j) //对每个路径点
        {
            Position p = map.get_pos(i, j); //路径点坐标
            if((p.x * UNIT_LENGTH <= mx) && (mx <= (p.x+1) * UNIT_LENGTH) &&
               (p.y * UNIT_LENGTH <= my) && (my <= (p.y+1) * UNIT_LENGTH)) //鼠标位置在路径点内
            {
                if(map(p.x, p.y) == MELEETOWER_VALUE) //路径点已存在近战塔
                {
                    for(auto tower = meleeTowerVec.begin(); tower != meleeTowerVec.end(); ++tower)
                        if((*tower)->get_x() == p.x && (*tower)->get_y() == p.y)
                        {
                            meleeTowerVec.erase(tower);
                            map.set_value(p.x, p.y, ROAD_VALUE);
                            qDebug() << p.x << p.y << "处撤销近战塔";
                            money += 60;
                            moneyLabel->setText("金币数：" + QString::number(money));
                            break;
                        }
                }
                else if(money >= 100)
                {
                    meleeTowerVec.push_back(new MeleeTower(p.x, p.y, 100, 30, 1, MELEETOWER_PATH));
                    map.set_value(p.x, p.y, MELEETOWER_VALUE);
                    qDebug() << p.x << p.y << "处安置近战塔";
                    money -= 100;
                    moneyLabel->setText("金币数：" + QString::number(money));
                }
                flag = false;
            }
        }
    flag = true;
    for(auto p : rmtTowerPosVec) //鼠标位置在远程塔安置点内
    {
        if(flag == false) break;
        if((p.x * UNIT_LENGTH <= mx) && (mx <= (p.x+1) * UNIT_LENGTH) &&
          (p.y * UNIT_LENGTH <= my) && (my <= (p.y+1) * UNIT_LENGTH))
        {
            if(map(p.x, p.y) == REMOTETOWER_VALUE) //路径点已存在远程塔
            {
                for(auto tower = remoteTowerVec.begin(); tower != remoteTowerVec.end(); ++tower)
                    if((*tower)->get_x() == p.x && (*tower)->get_y() == p.y)
                    {
                        remoteTowerVec.erase(tower);
                        map.set_value(p.x, p.y, GRASS_VALUE);
                        qDebug() << p.x << p.y << "处撤销远程塔";
                        money += 96;
                        moneyLabel->setText("金币数：" + QString::number(money));
                        break;
                    }
            }
            else if(money >= 160)
            {
                remoteTowerVec.push_back(new RemoteTower(p.x, p.y, 100, 20, 3, REMOTETOWER_PATH));
                map.set_value(p.x, p.y, REMOTETOWER_VALUE);
                qDebug() << p.x << p.y << "处安置远程塔";
                money -= 160;
                moneyLabel->setText("金币数：" + QString::number(money));
            }
            flag = false;
        }
    }
    update();
}
