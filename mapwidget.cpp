#include "mapwidget.h"
#include "ui_mapwidget.h"

#define UNIT_LENGTH 100
#define ENEMY_MAX_NUM 50

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

    //产生敌人的计时器
    QTimer* enemyTimer = new QTimer(this);
    enemyTimer->start(2000);
    connect(enemyTimer, &QTimer::timeout, [&]()
    {
        srand((unsigned)time(NULL));
        if(enemyCount < ENEMY_MAX_NUM)
        {
            int index = rand() % map.get_num(); //随机选择一条路径作为敌人路径
            enemyVec.push_back(new Enemy(100, 5, 1, ENEMY1_PATH, map.get_road(index)));
            enemyCount++;
        }
    });

    //游戏的主计时器
    QTimer* gameTimer = new QTimer(this);
    gameTimer->start(1000);
    connect(gameTimer, &QTimer::timeout, [&]()
    {
        //敌人移动
        for(auto enemy = enemyVec.begin(); enemy != enemyVec.end(); enemy++)
        {
            if(!(*enemy)->move()) //敌人走到路径尽头，删去这个敌人
            {
                delete *enemy;
                enemyVec.erase(enemy);
            }
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
        int x = enemy->get_x() * UNIT_LENGTH + 10;
        int y = enemy->get_y() * UNIT_LENGTH + 10;
        painter.drawPixmap(x, y, 80, 80, enemy->get_path());
    }
}

void MapWidget::drawMeleeTower(QPainter& painter)
{
    for(auto tower : meleeTowerVec)
    {
        int x = tower->get_x() * UNIT_LENGTH + 10;
        int y = tower->get_y() * UNIT_LENGTH + 10;
        painter.drawPixmap(x, y, 80, 80, tower->get_path());
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
    qDebug() << "按下左键" << mx << my << endl;
    for(int i = 0; i < map.get_num(); ++i)
        for(int j = 0; j < map.get_road(i).size(); ++j)
        {
            Position p = map.get_pos(i, j); //路径点
            if((p.x * UNIT_LENGTH <= mx) && (mx <= (p.x+1) * UNIT_LENGTH) &&
               (p.y * UNIT_LENGTH <= my) && (my <= (p.y+1) * UNIT_LENGTH)) //鼠标位置在路径点内
            {
                if(map(p.x, p.y) == MELEETOWER_VALUE) //路径点已存在近战塔
                    return;
                meleeTowerVec.push_back(new MeleeTower(p.x, p.y, 100, 5, 1, MELEETOWER_PATH));
                map.set_value(p.x, p.y, MELEETOWER_VALUE);
                qDebug() << p.x << p.y << "处安置近战塔";
            }
            QVector<Position> nearPos; //存放路径点的四邻接点
            if(p.x - 1 >= 0)
                nearPos.push_back(Position(p.x - 1, p.y));
            if(p.x + 1 < map.get_m())
                nearPos.push_back(Position(p.x + 1, p.y));
            if(p.y - 1 >= 0)
                nearPos.push_back(Position(p.x, p.y - 1));
            if(p.y + 1 < map.get_n())
                nearPos.push_back(Position(p.x, p.y + 1));
            for(auto np : nearPos) //鼠标位置在四邻接点内
            {
                if(map(np.x, np.y) == GRASS_VALUE &&
                  (np.x * UNIT_LENGTH <= mx) && (mx <= (np.x+1) * UNIT_LENGTH) &&
                  (np.y * UNIT_LENGTH <= my) && (my <= (np.y+1) * UNIT_LENGTH))
                {
                    remoteTowerVec.push_back(new RemoteTower(np.x, np.y, 100, 5, 3, REMOTETOWER_PATH));
                    map.set_value(np.x, np.y, REMOTETOWER_VALUE);
                    qDebug() << np.x << np.y << "处安置远程塔";
                }
            }
        }
    update();
}
