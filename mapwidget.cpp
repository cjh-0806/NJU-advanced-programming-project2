#include "mapwidget.h"
#include "ui_mapwidget.h"


MapWidget::MapWidget(QWidget *parent, Map m) :
    QWidget(parent), map(m),
    ui(new Ui::MapWidget)
{
    this->setFixedSize((map.get_m()+1) * UNIT_LENGTH, max(6, map.get_n()) * UNIT_LENGTH);
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

    tipTimer = new QTimer(this);
    tipTimer->setInterval(1000);

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

    //词缀库初始化
    for(int i = 0; i < 6; ++i)
        affixArr[i] = 0; //每个词缀数量为0

    //产生敌人的计时器
    enemyTimer = new QTimer(this);
    enemyTimer->setInterval(2000);
    enemyTimer->start();
    connect(enemyTimer, &QTimer::timeout, [&]()
    {
        srand((unsigned)time(NULL));
        if(enemyCount < ENEMY_MAX_NUM)
        {
            int index = rand() % map.get_num(); //随机选择一条路径作为敌人路径
            int enemyType = rand() % 3;
            if(enemyType == 0)
                enemyVec.push_back(new Enemy(100, 20, 1, ENEMY_PATH, map.get_road(index), false, false));
            else if(enemyType == 1)
                enemyVec.push_back(new Enemy(100, 20, 1, ENEMY1_PATH, map.get_road(index), true, false));
            else
                enemyVec.push_back(new Enemy(100, 20, 1, ENEMY2_PATH, map.get_road(index), false, true));
            enemyCount++;
        }
        if(life > 0 && enemyCount == ENEMY_MAX_NUM && enemyVec.empty()) //击败所有敌人，游戏成功
            this->close();
    });


    //游戏的主计时器
    gameTimer = new QTimer(this);
    gameTimer->setInterval(1000);
    gameTimer->start();
    connect(gameTimer, &QTimer::timeout, [&]()
    {
        //近战防御塔攻击
        for(auto tower : meleeTowerVec)
            for(auto enemy = enemyVec.begin(); enemy != enemyVec.end(); )
            {
                bool res = tower->attack(*enemy);
                if(res) //攻击有效
                {
                    if(tower->get_frozen() && !(*enemy)->get_frozen()) //冰冻效果
                        (*enemy)->set_frozen(true);
                    if(!(*enemy)->isAlive()) //敌人死亡，删去这个敌人，掉落词缀，加金币
                    {
                        money += 20;
                        moneyLabel->setText("金币数：" + QString::number(money));
                        int index = rand() % 4; //随机掉落我方词缀
                        affixArr[index]++;
                        //showTip(*enemy);
                        enemyVec.erase(enemy);
                    }
                    if(!tower->get_aoe()) //未安装群伤词缀，一次只攻击一个敌人
                        break;
                }
                else enemy++;
            }

        //远程防御塔攻击
        for(auto tower : remoteTowerVec)
            for(auto enemy = enemyVec.begin(); enemy != enemyVec.end(); )
            {
                bool res = tower->attack(*enemy);
                if(res) //攻击有效
                {
                    if(tower->get_bleed() && !(*enemy)->get_bleed()) //放血效果
                        (*enemy)->set_bleed(true);
                    if(!(*enemy)->isAlive()) //敌人死亡，删去这个敌人，掉落词缀，加金币
                    {
                        money += 20;
                        moneyLabel->setText("金币数：" + QString::number(money));
                        int index = rand() % 4; //随机掉落我方词缀
                        affixArr[index]++;
                        //showTip(*enemy);
                        enemyVec.erase(enemy);
                    }
                    break; //一次只攻击一个敌人
                }
                else enemy++;
            }

        //敌人移动
        for(auto enemy = enemyVec.begin(); enemy != enemyVec.end(); )
        {
            if((*enemy)->get_bleed()) //被放血
            {
                (*enemy)->bleedTimer++;
                (*enemy)->dec_hp();
                if((*enemy)->bleedTimer == 4) //已经被放血了四个时间单位，取消放血效果
                {
                    (*enemy)->bleedTimer = 0;
                    (*enemy)->set_bleed(false);
                    qDebug() << "取消放血效果";
                }
                if(!(*enemy)->isAlive()) //敌人死亡，删去这个敌人，掉落词缀，加金币
                {
                    money += 20;
                    moneyLabel->setText("金币数：" + QString::number(money));
                    int index = rand() % 4; //随机掉落我方词缀
                    affixArr[index]++;
                    //showTip(*enemy);
                    enemyVec.erase(enemy);
                    continue;
                }
            }
            if((*enemy)->get_frozen()) //被冰冻
            {
                (*enemy)->frozenTimer++;
                if((*enemy)->frozenTimer == 4) //已经被冰冻了四个时间单位，取消冰冻效果
                {
                    (*enemy)->frozenTimer = 0;
                    (*enemy)->set_frozen(false);
                    qDebug() << "取消冰冻效果";
                }
                enemy++;
                continue;
            }
            if(!(*enemy)->move(map)) //敌人走到路径尽头，删去这个敌人，生命值-1
            {
                enemyVec.erase(enemy);
                life--;
                lifeLabel->setText("生命值：" + QString::number(life));
                if(life == 0) //生命值为0，游戏失败
                    this->close();
            }
            else enemy++;
        }

        //敌人攻击近战防御塔
        for(auto enemy : enemyVec)
        {
            if(enemy->get_frozen()) //被冰冻
                continue;
            for(auto tower = meleeTowerVec.begin(); tower != meleeTowerVec.end(); )
            {
                enemy->attack(*tower);
                if(!(*tower)->isAlive())
                {
                    map.set_value((*tower)->get_x(), (*tower)->get_y(), ROAD_VALUE);
                    select.set_display(false);
                    meleeTowerVec.erase(tower);
                }
                else tower++;
            }
        }
        update();
    });
}


MapWidget::~MapWidget()
{
    delete ui;
}

void MapWidget::closeEvent(QCloseEvent* event)
{
    enemyTimer->stop();
    gameTimer->stop();
    if(life == 0)
    {
        QMessageBox::information(this, "结束", "游戏失败！");
        event->accept();
    }
    else if(life > 0 && enemyCount == ENEMY_MAX_NUM && enemyVec.empty()) //击败所有敌人，游戏成功
    {
        QMessageBox::information(this, "结束", "游戏成功！");
        event->accept();
    }
    else
    {
        QMessageBox::StandardButton button;
        button = QMessageBox::warning(this, "警告", "游戏未结束，确认关闭？", QMessageBox::Yes|QMessageBox::No);
        if(button == QMessageBox::Yes)
            event->accept();
        else
        {
            enemyTimer->start();
            gameTimer->start();
            event->ignore();
        }
    }
}

void MapWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    drawMap(painter);
    drawEnemy(painter);
    drawMeleeTower(painter);
    drawRemoteTower(painter);
    drawAffix(painter);
    drawSelectAffix(painter);
}

void MapWidget::drawMap(QPainter& painter) //画出地图
{
    int i, j;
    for(i = 0; i < map.get_m(); ++i)
        for(j = 0; j < map.get_n(); ++j)
        {
            int x = i * UNIT_LENGTH;
            int y = j * UNIT_LENGTH;
            if(map(i,j) == GRASS_VALUE || map(i, j) == REMOTETOWER_VALUE)
                painter.drawPixmap(x, y, UNIT_LENGTH, UNIT_LENGTH, QString::fromStdString(GRASS_PATH));
            if(map(i,j) == ROAD_VALUE || map(i, j) == MELEETOWER_VALUE)
                painter.drawPixmap(x, y, UNIT_LENGTH, UNIT_LENGTH, QString::fromStdString(ROAD_PATH));
        }
}

void MapWidget::drawEnemy(QPainter& painter) //画出敌人
{
    for(auto enemy : enemyVec)
    {
        if(enemy->isAlive())
        {
            int x = (enemy->get_x()+0.1) * UNIT_LENGTH;
            int y = (enemy->get_y()+0.1) * UNIT_LENGTH;
            painter.drawPixmap(x, y, 0.8*UNIT_LENGTH, 0.8*UNIT_LENGTH, enemy->get_path());
            if(enemy->get_frozen()) //添加冰冻效果
            {
                QPixmap pix1(":/pictures/frozen.jpg");
                QPixmap pix2(pix1.size());
                pix2.fill(Qt::transparent);
                QPainter temp(&pix2);
                temp.setCompositionMode(QPainter::CompositionMode_Source);
                temp.drawPixmap(0, 0, pix1);
                temp.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                temp.fillRect(pix2.rect(), QColor(0, 0, 0, 125)); //根据QColor中第四个参数设置透明度，0～255
                temp.end();
                painter.drawPixmap(x, y, 0.8*UNIT_LENGTH, 0.8*UNIT_LENGTH, pix2);
            }
            if(enemy->get_bleed()) //添加放血效果
            {
                QPixmap pix1(":/pictures/bleed.jpg");
                QPixmap pix2(pix1.size());
                pix2.fill(Qt::transparent);
                QPainter temp(&pix2);
                temp.setCompositionMode(QPainter::CompositionMode_Source);
                temp.drawPixmap(0, 0, pix1);
                temp.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                temp.fillRect(pix2.rect(), QColor(0, 0, 0, 125)); //根据QColor中第四个参数设置透明度，0～255
                temp.end();
                painter.drawPixmap(x, y, 0.8*UNIT_LENGTH, 0.8*UNIT_LENGTH, pix2);
            }
            if((enemy->get_hp() / enemy->get_sumhp()) < 1) //设置血条颜色
                painter.setBrush(QBrush(Qt::red));
            else
                painter.setBrush(QBrush(Qt::green));
            float rate = (float)(enemy->get_hp())/(float)(enemy->get_sumhp()); //计算比例
            painter.drawRect(x, y-10, rate*80, 10); //绘制矩形
        }
    }
}

void MapWidget::drawMeleeTower(QPainter& painter) //画出近战塔
{
    for(auto tower : meleeTowerVec)
    {
        if(tower->isAlive())
        {
            int x = (tower->get_x()+0.1) * UNIT_LENGTH;
            int y = (tower->get_y()+0.1) * UNIT_LENGTH;
            painter.drawPixmap(x, y, 0.8*UNIT_LENGTH, 0.8*UNIT_LENGTH, tower->get_path());
            if((tower->get_hp() / tower->get_sumhp()) < 1) //设置血条颜色
                painter.setBrush(QBrush(Qt::red));
            else
                painter.setBrush(QBrush(Qt::green));
            float rate = (float)(tower->get_hp())/(float)(tower->get_sumhp()); //计算比例
            painter.drawRect(x, y-10, rate*80, 10); //绘制矩形
        }
    }
}

void MapWidget::drawRemoteTower(QPainter& painter) //画出远程塔
{
    for(auto tower : remoteTowerVec)
    {
        int x = (tower->get_x()+0.1) * UNIT_LENGTH;
        int y = (tower->get_y()+0.1) * UNIT_LENGTH;
        painter.drawPixmap(x, y, 0.8*UNIT_LENGTH, 0.8*UNIT_LENGTH, tower->get_path());
    }
}

void MapWidget::drawAffix(QPainter& painter) //画出词缀库
{
    int x = (map.get_m()+0.1) * UNIT_LENGTH;
    int y = 0.1 * UNIT_LENGTH;
    for(int i = 0; i < 6; ++i)
    {
        QString path = ":/pictures/affix" + QString::number(i) +".jpg";
        painter.drawPixmap(x, y, 0.6*UNIT_LENGTH, 0.6*UNIT_LENGTH, path);
        QFont font("宋体", 8);
        painter.setFont(font);
        switch(i)
        {
        case 0:painter.drawText(x, y + 0.8*UNIT_LENGTH, "狂暴：" + QString::number(affixArr[i])); break;
        case 1:painter.drawText(x, y + 0.8*UNIT_LENGTH, "冰系：" + QString::number(affixArr[i])); break;
        case 2:painter.drawText(x, y + 0.8*UNIT_LENGTH, "群伤：" + QString::number(affixArr[i])); break;
        case 3:painter.drawText(x, y + 0.8*UNIT_LENGTH, "放血：" + QString::number(affixArr[i])); break;
        case 4:painter.drawText(x, y + 0.8*UNIT_LENGTH, "闪现：" + QString::number(affixArr[i])); break;
        case 5:painter.drawText(x, y + 0.8*UNIT_LENGTH, "神速：" + QString::number(affixArr[i])); break;
        }
        y += UNIT_LENGTH;
    }
}

void MapWidget::drawSelectAffix(QPainter& painter) //画出词缀选择框
{
    if(!select.get_display())
        return;
    if(select.get_type() == MELEETOWER_VALUE) //近战塔
    {
        bool setAffix[3] = {false, false, false};
        //已安装这类词缀在上方显示√
        if(meleeTowerVec[select.get_index()]->get_rage())
        {
            painter.drawText(select.get_x(), select.get_y()-10, "√");
            setAffix[0] = true;
        }
        if(meleeTowerVec[select.get_index()]->get_frozen())
        {
            painter.drawText(select.get_x()+select.get_height(), select.get_y()-10, "√");
            setAffix[1] = true;
        }
        if(meleeTowerVec[select.get_index()]->get_aoe())
        {
            painter.drawText(select.get_x()+2*select.get_height(), select.get_y()-10, "√");
            setAffix[2] = true;
        }
        //显示词缀图片
        for(int i = 0; i < 3; ++i)
        {
            if((meleeTowerVec[select.get_index()]->get_count() == 2) || //词缀槽已满
              (meleeTowerVec[select.get_index()]->get_count() < 2 && (setAffix[i] == true || affixArr[i] == 0))) //已安装词缀或词缀库里无该词缀
            { //灰度显示
                QImage *image = new QImage(select.affixPaths[i]);
                QImage *gray_image = new QImage;
                *gray_image = image->convertToFormat(QImage::Format_Grayscale8,Qt::AutoColor);
                QPixmap *pixmap = new QPixmap();
                pixmap->convertFromImage(*gray_image);
                painter.drawPixmap(select.get_x()+i*select.get_height(), select.get_y(), select.get_height(), select.get_height(), *pixmap);
            }
            else
                painter.drawPixmap(select.get_x()+i*select.get_height(), select.get_y(), select.get_height(), select.get_height(), select.affixPaths[i]);
        }
        painter.drawPixmap(select.get_x()+select.get_height()*3, select.get_y(), select.get_height(), select.get_height(), select.affixPaths[3]);

    }
    else //远程塔
    {
        //已安装这类词缀在上方显示√
        if(remoteTowerVec[select.get_index()]->get_bleed())
            painter.drawText(select.get_x(), select.get_y()-10, "√");
        //显示词缀图片
        if(remoteTowerVec[select.get_index()]->get_bleed() || affixArr[3] == 0) //已安装词缀或词缀库里无该词缀
        { //灰度显示
            QImage *image = new QImage(select.affixPaths[0]);
            QImage *gray_image = new QImage;
            *gray_image = image->convertToFormat(QImage::Format_Grayscale8,Qt::AutoColor);
            QPixmap *pixmap = new QPixmap();
            pixmap->convertFromImage(*gray_image);
            painter.drawPixmap(select.get_x(), select.get_y(), select.get_height(), select.get_height(), *pixmap);
        }
        else
            painter.drawPixmap(select.get_x(), select.get_y(), select.get_height(), select.get_height(), select.affixPaths[0]);
        painter.drawPixmap(select.get_x()+select.get_height(), select.get_y(), select.get_height(), select.get_height(), select.affixPaths[1]);

    }
}


void MapWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) //鼠标左键点击
        return;
    int mx = event->pos().x();
    int my = event->pos().y();
    qDebug() << "按下左键" << mx << my;


    if(select.get_display() && select.get_x() <= mx && mx < select.get_x() + select.get_length()
            && select.get_y() <= my && my < select.get_y()+select.get_height()) //鼠标位置在词缀选择框内
    {
        if(select.get_type() == MELEETOWER_VALUE) //近战塔
        {
            if(select.get_x() <= mx && mx < select.get_x() + select.get_height()) //狂暴词缀
            {
                if(!meleeTowerVec[select.get_index()]->get_rage())
                {
                    if(meleeTowerVec[select.get_index()]->get_count() < 2)
                    {
                        meleeTowerVec[select.get_index()]->add_rage();
                        meleeTowerVec[select.get_index()]->add_count();
                        qDebug() << "安装狂暴词缀";
                        affixArr[0]--;
                    }
                }
                else
                {
                    meleeTowerVec[select.get_index()]->dec_rage();
                    meleeTowerVec[select.get_index()]->dec_count();
                    qDebug() << "卸下狂暴词缀";
                }
            }
            else if(select.get_x() + select.get_height() <= mx && mx < select.get_x() + select.get_height()*2) //冰系词缀
            {
                if(!meleeTowerVec[select.get_index()]->get_frozen())
                {
                    if(meleeTowerVec[select.get_index()]->get_count() < 2)
                    {
                        meleeTowerVec[select.get_index()]->add_frozen();
                        meleeTowerVec[select.get_index()]->add_count();
                        qDebug() << "安装冰系词缀";
                        affixArr[1]--;
                    }
                }
                else
                {
                    meleeTowerVec[select.get_index()]->dec_frozen();
                    meleeTowerVec[select.get_index()]->dec_count();
                    qDebug() << "卸下冰系词缀";
                }
            }
            else if(select.get_x() + select.get_height()*2 <= mx && mx < select.get_x() + select.get_height()*3) //群伤词缀
            {
                if(!meleeTowerVec[select.get_index()]->get_aoe())
                {
                    if(meleeTowerVec[select.get_index()]->get_count() < 2)
                    {
                        meleeTowerVec[select.get_index()]->add_aoe();
                        meleeTowerVec[select.get_index()]->add_count();
                        qDebug() << "安装群伤词缀";
                        affixArr[2]--;
                    }
                }
                else
                {
                    meleeTowerVec[select.get_index()]->dec_aoe();
                    meleeTowerVec[select.get_index()]->dec_count();
                    qDebug() << "卸下群伤词缀";
                }
            }
            else //撤销塔
            {
                map.set_value(meleeTowerVec[select.get_index()]->get_x(), meleeTowerVec[select.get_index()]->get_y(), ROAD_VALUE);
                qDebug() << meleeTowerVec[select.get_index()]->get_x() << meleeTowerVec[select.get_index()]->get_y() << "处撤销近战塔";
                meleeTowerVec.erase(meleeTowerVec.begin() + select.get_index());
                money += 60;
                moneyLabel->setText("金币数：" + QString::number(money));
                select.set_display(false);
            }
        }
        else //远程塔
        {
            if(select.get_x() <= mx && mx < select.get_x() + select.get_height()) //放血词缀
            {
                if(!remoteTowerVec[select.get_index()]->get_bleed())
                {
                    if(remoteTowerVec[select.get_index()]->get_count() < 2)
                    {
                        remoteTowerVec[select.get_index()]->add_bleed();
                        remoteTowerVec[select.get_index()]->add_count();
                        qDebug() << "安装放血词缀";
                        affixArr[3]--;
                    }
                }
                else
                {
                    remoteTowerVec[select.get_index()]->dec_bleed();
                    remoteTowerVec[select.get_index()]->dec_count();
                    qDebug() << "卸下放血词缀";
                }
            }
            else //撤销塔
            {
                map.set_value(remoteTowerVec[select.get_index()]->get_x(), remoteTowerVec[select.get_index()]->get_y(), GRASS_VALUE);
                qDebug() << remoteTowerVec[select.get_index()]->get_x() << remoteTowerVec[select.get_index()]->get_y() << "处撤销远程塔";
                remoteTowerVec.erase(remoteTowerVec.begin() + select.get_index());
                money += 96;
                moneyLabel->setText("金币数：" + QString::number(money));
                select.set_display(false);
            }
        }
    }

    else //鼠标位置在非词缀选择框内
    {
        bool flag = true;
        for(int i = 0; i < map.get_num() && flag == true; ++i)
            for(int j = 0; j < map.get_road(i).size() && flag == true; ++j) //对每个路径点
            {
                Position p = map.get_pos(i, j); //路径点坐标
                if((p.x * UNIT_LENGTH <= mx) && (mx < (p.x+1) * UNIT_LENGTH) &&
                   (p.y * UNIT_LENGTH <= my) && (my < (p.y+1) * UNIT_LENGTH)) //鼠标位置在路径点内
                {
                    if(map(p.x, p.y) == MELEETOWER_VALUE) //路径点已存在近战塔
                    {
                        for(int index = 0; index < meleeTowerVec.size(); ++index)
                            if(meleeTowerVec[index]->get_x() == p.x && meleeTowerVec[index]->get_y() == p.y)
                            {
                                if(select.get_type() == MELEETOWER_VALUE && index == select.get_index() && select.get_display()) //点击同一个塔
                                    select.set_display(false);
                                else
                                {
                                    select.change(MELEETOWER_VALUE, meleeTowerVec[index]->get_pos(), index); //换显示框内容
                                    select.set_display(true);
                                }
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
            if((p.x * UNIT_LENGTH <= mx) && (mx < (p.x+1) * UNIT_LENGTH) &&
              (p.y * UNIT_LENGTH <= my) && (my < (p.y+1) * UNIT_LENGTH))
            {
                if(map(p.x, p.y) == REMOTETOWER_VALUE) //路径点已存在远程塔
                {
                    for(int index = 0; index < remoteTowerVec.size(); ++index)
                        if(remoteTowerVec[index]->get_x() == p.x && remoteTowerVec[index]->get_y() == p.y)
                        {
                            if(select.get_type() == REMOTETOWER_VALUE && index == select.get_index() && select.get_display()) //点击同一个塔
                                select.set_display(false);
                            else
                            {
                                select.change(REMOTETOWER_VALUE, remoteTowerVec[index]->get_pos(), index); //换显示框内容
                                select.set_display(true);
                            }
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
    }

    update();
}

void MapWidget::showTip(Enemy* e)
{
    QLabel* tipLabel = new QLabel(this);
    tipLabel->setText("金币+10 掉落词缀");
    tipLabel->setGeometry(e->get_x()*UNIT_LENGTH, (e->get_y()-0.1)*UNIT_LENGTH, 200, 20);
    tipLabel->setVisible(1);
    tipTimer->start();
    connect(tipTimer, &QTimer::timeout, this, [&]()
    {
        tipTimer->stop();
        qDebug() << "消失";
        tipLabel->setVisible(0);
    });
}
