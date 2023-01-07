#include "mapwidget.h"
#include "ui_mapwidget.h"


MapWidget::MapWidget(QWidget *parent, Map m) :
    QWidget(parent), map(m),
    ui(new Ui::MapWidget)
{
    this->setFixedSize((map.get_m()+1) * UNIT_LENGTH, max(6, map.get_n()) * UNIT_LENGTH);
    ui->setupUi(this);
    enemyCount = 0;
    isPlay = volume = true;

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
                rangedTowerPosVec.push_back(Position(p.x - 1, p.y));
            if(p.x + 1 < map.get_m() && map(p.x + 1, p.y) == GRASS_VALUE)
                rangedTowerPosVec.push_back(Position(p.x + 1, p.y));
            if(p.y - 1 >= 0 && map(p.x, p.y - 1) == GRASS_VALUE)
                rangedTowerPosVec.push_back(Position(p.x, p.y - 1));
            if(p.y + 1 < map.get_n() && map(p.x, p.y + 1) == GRASS_VALUE)
                rangedTowerPosVec.push_back(Position(p.x, p.y + 1));
        }

    //词缀库初始化
    for(int i = 0; i < 6; ++i)
        affixArr[i] = 1; //每个词缀数量为1

    //金币增加的计时器
    moneyTimer = new QTimer(this);
    moneyTimer->setInterval(MONEY_INTERVAL);
    moneyTimer->start();
    connect(moneyTimer, &QTimer::timeout, [&]()
    {
        money += 10;
        moneyLabel->setText("金币数：" + QString::number(money));
    });

    //产生敌人的计时器
    srand((unsigned)time(NULL));
    enemyTimer = new QTimer(this);
    enemyTimer->setInterval(ENEMY_INTERVAL);
    enemyTimer->start();
    connect(enemyTimer, &QTimer::timeout, [&]()
    {
        if(enemyCount < ENEMY_MAX_NUM)
        {
            int index = rand() % map.get_num(); //随机选择一条路径作为敌人路径
            int enemyType = rand() % 4;
            if(enemyType == 0)
                enemyVec.push_back(new Enemy(ENEMY_PATH, map.get_road(index), false, false));
            else if(enemyType == 1)
                enemyVec.push_back(new Enemy(ENEMY1_PATH, map.get_road(index), true, false));
            else if(enemyType == 2)
                enemyVec.push_back(new Enemy(ENEMY2_PATH, map.get_road(index), false, true));
            else
                enemyVec.push_back(new Enemy(ENEMY3_PATH, map.get_road(index), true, true));
            if(volume) //音效
            {
                QSoundEffect* effect = new QSoundEffect(this);
                effect->setSource(QUrl("qrc:///sounds/enemy.wav"));
                effect->setLoopCount(1);
                effect->play();
            }
            enemyCount++;
        }
        if(life > 0 && enemyCount == ENEMY_MAX_NUM && enemyVec.empty()) //击败所有敌人，游戏成功
            this->close();
    });


    //游戏的主计时器
    gameTimer = new QTimer(this);
    gameTimer->setInterval(GAME_INTERVAL);
    gameTimer->start();
    connect(gameTimer, &QTimer::timeout, [&]()
    {
        //近战防御塔攻击
        meleeatkVec.clear();
        for(auto tower : meleeTowerVec)
        {
            if(tower->get_avoid())
            {
                tower->avoidTimer++;
                if(tower->avoidTimer == 10) //免伤效果时间已过
                {
                    tower->avoidTimer = 0;
                    tower->set_avoid(false);
                    tower->dec_count();
                }
            }
            for(auto enemy = enemyVec.begin(); enemy != enemyVec.end(); enemy++)
            {
                bool res = tower->attack(*enemy);
                if(res) //攻击有效
                {
                    if(tower->get_frozen()) //冰冻效果
                    {
                        (*enemy)->set_frozen(true);
                        (*enemy)->frozenTimer = 0;
                    }
                    meleeatkVec.push_back(new MeleeAttackEffect(tower, *enemy));
                    if(!tower->get_aoe()) //未安装群伤词缀，一次只攻击一个敌人
                        break;
                }
            }
        }

        //远程防御塔攻击
        rangedatkVec.clear();
        for(auto tower : rangedTowerVec)
            for(auto enemy = enemyVec.begin(); enemy != enemyVec.end(); enemy++)
            {
                bool res = tower->attack(*enemy);
                if(res) //攻击有效
                {
                    if(tower->get_bleed()) //放血效果
                    {
                        (*enemy)->set_bleed(true);
                        (*enemy)->bleedTimer = 0;
                    }
                    if(tower->get_weaken()) //弱化效果
                    {
                        (*enemy)->add_weaken();
                        (*enemy)->weakenTimer = 0;
                    }
                    rangedatkVec.push_back(new RangedAttackEffect(tower, *enemy));
                    if(!tower->get_aoe()) //未安装群伤词缀，一次只攻击一个敌人
                        break;
                }
            }

        //敌人移动
        for(auto enemy = enemyVec.begin(); enemy != enemyVec.end(); )
        {
            if((*enemy)->get_bleed()) //被放血
            {
                (*enemy)->bleedTimer++;
                (*enemy)->dec_hp();
                if((*enemy)->bleedTimer == 5) //放血效果时间已过
                {
                    (*enemy)->bleedTimer = 0;
                    (*enemy)->set_bleed(false);
                }
            }
            if(!(*enemy)->isAlive()) //敌人死亡，删去这个敌人，掉落词缀，加金币
            {
                money += 20;
                moneyLabel->setText("金币数：" + QString::number(money));
                int index = rand() % 6; //随机掉落我方词缀
                affixArr[index]++;
                enemyVec.erase(enemy);
                if(volume)
                {
                    QSoundEffect* effect = new QSoundEffect(this);
                    effect->setSource(QUrl("qrc:///sounds/die.wav"));
                    effect->setLoopCount(1);
                    effect->play();
                }
                continue;
            }
            if((*enemy)->get_frozen()) //被冰冻
            {
                (*enemy)->frozenTimer++;
                if((*enemy)->frozenTimer == 5) //冰冻效果时间已过
                {
                    (*enemy)->frozenTimer = 0;
                    (*enemy)->set_frozen(false);
                }
                enemy++;
                continue;
            }
            if((*enemy)->get_weaken()) //被弱化
            {
                (*enemy)->weakenTimer++;
                if((*enemy)->weakenTimer == 5) //弱化效果时间已过
                {
                    (*enemy)->weakenTimer = 0;
                    (*enemy)->dec_weaken();
                }
            }
            //敌人攻击近战防御塔
            for(auto tower = meleeTowerVec.begin(); tower != meleeTowerVec.end(); )
            {
                if((*tower)->get_avoid())
                {
                    tower++;
                    continue;
                }
                (*enemy)->attack(*tower);
                if(!(*tower)->isAlive())
                {
                    map.set_value((*tower)->get_x(), (*tower)->get_y(), ROAD_VALUE);
                    select.set_display(false);
                    meleeTowerVec.erase(tower);
                }
                else tower++;
            }
            if(!(*enemy)->move(map)) //敌人走到路径尽头，删去这个敌人，生命值-1
            {
                enemyVec.erase(enemy);
                life--;
                lifeLabel->setText("生命值：" + QString::number(life));
                if(volume)
                {
                    QSoundEffect* effect = new QSoundEffect(this);
                    effect->setSource(QUrl("qrc:///sounds/move.wav"));
                    effect->setLoopCount(1);
                    effect->play();
                }
                if(life == 0) //生命值为0，游戏失败
                    this->close();
            }
            else enemy++;
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
    moneyTimer->stop();
    if(life == 0)
    {
        if(volume)
        {
            QSoundEffect* effect = new QSoundEffect(this);
            effect->setSource(QUrl("qrc:///sounds/defeat.wav"));
            effect->setLoopCount(1);
            effect->play();
        }
        QMessageBox::information(this, "结束", "游戏失败！");
        event->accept();
    }
    else if(life > 0 && enemyCount == ENEMY_MAX_NUM && enemyVec.empty()) //击败所有敌人，游戏成功
    {
        if(volume)
        {
            QSoundEffect* effect = new QSoundEffect(this);
            effect->setSource(QUrl("qrc:///sounds/victory.wav"));
            effect->setLoopCount(1);
            effect->play();
        }
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
            moneyTimer->start();
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
    drawRangedTower(painter);
    drawAffix(painter);
    drawSelectAffix(painter);
    drawAttackEffect(painter);
    drawButton(painter);
}

void MapWidget::drawMap(QPainter& painter) //画出地图
{
    int i, j;
    for(i = 0; i < map.get_m(); ++i)
        for(j = 0; j < map.get_n(); ++j)
        {
            int x = i * UNIT_LENGTH;
            int y = j * UNIT_LENGTH;
            if(map(i,j) == GRASS_VALUE || map(i, j) == RANGEDTOWER_VALUE)
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
            if(enemy->get_weaken())
                painter.drawPixmap(x+0.2*UNIT_LENGTH, y+0.2*UNIT_LENGTH, 0.4*UNIT_LENGTH, 0.4*UNIT_LENGTH, enemy->get_path());
            else
                painter.drawPixmap(x, y, 0.8*UNIT_LENGTH, 0.8*UNIT_LENGTH, enemy->get_path());
            if(enemy->get_frozen()) //添加冰冻效果
            {
                QPixmap pix1(":/pictures/frozen.png");
                QPixmap pix2(pix1.size());
                pix2.fill(Qt::transparent);
                QPainter temp(&pix2);
                temp.setCompositionMode(QPainter::CompositionMode_Source);
                temp.drawPixmap(0, 0, pix1);
                temp.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                temp.fillRect(pix2.rect(), QColor(0, 0, 0, 125)); //根据QColor中第四个参数设置透明度，0～255
                temp.end();
                if(enemy->get_weaken())
                    painter.drawPixmap(x+0.2*UNIT_LENGTH, y+0.2*UNIT_LENGTH, 0.4*UNIT_LENGTH, 0.4*UNIT_LENGTH, pix2);
                else
                    painter.drawPixmap(x, y, 0.8*UNIT_LENGTH, 0.8*UNIT_LENGTH, pix2);
            }
            if(enemy->get_bleed()) //添加放血效果
            {
                QPixmap pix1(":/pictures/bleed.png");
                painter.drawPixmap(x, y, 0.8*UNIT_LENGTH, 0.8*UNIT_LENGTH, pix1);
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
            if(tower->get_avoid()) //添加免伤效果
            {
                QPixmap pix1(":/pictures/avoid.png");
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
            if((tower->get_hp() / tower->get_sumhp()) < 1) //设置血条颜色
                painter.setBrush(QBrush(Qt::red));
            else
                painter.setBrush(QBrush(Qt::green));
            float rate = (float)(tower->get_hp())/(float)(tower->get_sumhp()); //计算比例
            painter.drawRect(x, y-10, rate*80, 10); //绘制矩形
        }
    }
}

void MapWidget::drawRangedTower(QPainter& painter) //画出远程塔
{
    for(auto tower : rangedTowerVec)
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
        case 3:painter.drawText(x, y + 0.8*UNIT_LENGTH, "免伤：" + QString::number(affixArr[i])); break;
        case 4:painter.drawText(x, y + 0.8*UNIT_LENGTH, "放血：" + QString::number(affixArr[i])); break;
        case 5:painter.drawText(x, y + 0.8*UNIT_LENGTH, "弱化：" + QString::number(affixArr[i])); break;
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
        bool setAffix[4] = {false, false, false, false};
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
        if(meleeTowerVec[select.get_index()]->get_avoid())
        {
            painter.drawText(select.get_x()+3*select.get_height(), select.get_y()-10, "√");
            setAffix[3] = true;
        }
        //显示词缀图片
        for(int i = 0; i < 4; ++i)
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
        painter.drawPixmap(select.get_x()+select.get_height()*4, select.get_y(), select.get_height(), select.get_height(), select.affixPaths[4]);

    }
    else //远程塔
    {
        bool setAffix[3] = {false, false, false};
        int affixIndex[3] = {2, 4, 5};
        //已安装这类词缀在上方显示√
        if(rangedTowerVec[select.get_index()]->get_aoe())
        {
            painter.drawText(select.get_x(), select.get_y()-10, "√");
            setAffix[0] = true;
        }
        if(rangedTowerVec[select.get_index()]->get_bleed())
        {
            painter.drawText(select.get_x()+select.get_height(), select.get_y()-10, "√");
            setAffix[1] = true;
        }
        if(rangedTowerVec[select.get_index()]->get_weaken())
        {
            painter.drawText(select.get_x()+2*select.get_height(), select.get_y()-10, "√");
            setAffix[2] = true;
        }
        //显示词缀图片
        for(int i = 0; i < 3; ++i)
        {
            if((rangedTowerVec[select.get_index()]->get_count() == 2) || //词缀槽已满
              (rangedTowerVec[select.get_index()]->get_count() < 2 && (setAffix[i] == true || affixArr[affixIndex[i]] == 0))) //已安装词缀或词缀库里无该词缀
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
        painter.drawPixmap(select.get_x()+3*select.get_height(), select.get_y(), select.get_height(), select.get_height(), select.affixPaths[3]);

    }
}

void MapWidget::drawAttackEffect(QPainter &painter) //画出远程塔攻击效果
{
    for(auto a: meleeatkVec)
    {
        if(a->enemy->isAlive() && a->tower->inRange(a->enemy->get_pos()))
        {
            int x1 = (a->tower->get_x()+0.5) * UNIT_LENGTH;
            int y1 = (a->tower->get_y()+0.5) * UNIT_LENGTH;
            int x2 = (a->enemy->get_x()+0.5) * UNIT_LENGTH;
            int y2 = (a->enemy->get_y()+0.5) * UNIT_LENGTH;
            painter.setPen(QPen(Qt::green, 4)); //设置画笔形式
            painter.drawLine(x1, y1, x2, y2);
        }
    }
    for(auto a: rangedatkVec)
    {
        if(a->enemy->isAlive() && a->tower->inRange(a->enemy->get_pos()))
        {
            int x1 = (a->tower->get_x()+0.5) * UNIT_LENGTH;
            int y1 = (a->tower->get_y()+0.5) * UNIT_LENGTH;
            int x2 = (a->enemy->get_x()+0.5) * UNIT_LENGTH;
            int y2 = (a->enemy->get_y()+0.5) * UNIT_LENGTH;
            painter.setPen(QPen(Qt::darkMagenta, 4));
            painter.drawLine(x1, y1, x2, y2);
        }
    }
}

void MapWidget::drawButton(QPainter &painter) //画出暂停键和音量键
{
    int x = (map.get_m()-1) * UNIT_LENGTH;
    int y = 0.1*UNIT_LENGTH;
    if(isPlay)
        painter.drawPixmap(x, y, 0.4*UNIT_LENGTH, 0.4*UNIT_LENGTH, QString::fromStdString(":/pictures/start.png"));
    else
        painter.drawPixmap(x, y, 0.4*UNIT_LENGTH, 0.4*UNIT_LENGTH, QString::fromStdString(":/pictures/stop.png"));
    painter.drawPixmap(x+0.5*UNIT_LENGTH, y, 0.4*UNIT_LENGTH, 0.4*UNIT_LENGTH, QString::fromStdString(":/pictures/sound.png"));
    if(!volume)
    {
        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(x+0.5*UNIT_LENGTH, y, x+0.9*UNIT_LENGTH, y+0.4*UNIT_LENGTH);
    }
}

void MapWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) //鼠标左键点击
        return;
    int mx = event->pos().x();
    int my = event->pos().y();
    qDebug() << "按下左键" << mx << my;

    int sound = 0;
    int buttonX = (map.get_m()-1) * UNIT_LENGTH; int buttonY = 0.1*UNIT_LENGTH;
    if(buttonX <= mx && mx < buttonX+0.4*UNIT_LENGTH && buttonY <= my && my < buttonY+0.4*UNIT_LENGTH) //暂停键
    {
        isPlay = 1 - isPlay;
        if(isPlay)
        {
            enemyTimer->start();
            gameTimer->start();
            moneyTimer->start();
        }
        else
        {
            enemyTimer->stop();
            gameTimer->stop();
            moneyTimer->stop();
        }
    }
    else if(buttonX+0.5*UNIT_LENGTH <= mx && mx < buttonX+0.9*UNIT_LENGTH && buttonY <= my && my < buttonY+0.4*UNIT_LENGTH) //音量键
        volume = 1 - volume;

    else if(select.get_display() && select.get_x() <= mx && mx < select.get_x() + select.get_length()
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
                        sound = 1;
                        affixArr[0]--;
                    }
                }
                else
                {
                    meleeTowerVec[select.get_index()]->dec_rage();
                    meleeTowerVec[select.get_index()]->dec_count();
                    sound = 2;
                }
            }
            else if(select.get_x() + select.get_height() <= mx && mx < select.get_x() + select.get_height()*2) //冰系词缀
            {
                if(!meleeTowerVec[select.get_index()]->get_frozen())
                {
                    if(meleeTowerVec[select.get_index()]->get_count() < 2)
                    {
                        meleeTowerVec[select.get_index()]->set_frozen(true);
                        meleeTowerVec[select.get_index()]->add_count();
                        sound = 1;
                        affixArr[1]--;
                    }
                }
                else
                {
                    meleeTowerVec[select.get_index()]->set_frozen(false);
                    meleeTowerVec[select.get_index()]->dec_count();
                    sound = 2;
                }
            }
            else if(select.get_x() + select.get_height()*2 <= mx && mx < select.get_x() + select.get_height()*3) //群伤词缀
            {
                if(!meleeTowerVec[select.get_index()]->get_aoe())
                {
                    if(meleeTowerVec[select.get_index()]->get_count() < 2)
                    {
                        meleeTowerVec[select.get_index()]->set_aoe(true);
                        meleeTowerVec[select.get_index()]->add_count();
                        sound = 1;
                        affixArr[2]--;
                    }
                }
                else
                {
                    meleeTowerVec[select.get_index()]->set_aoe(false);
                    meleeTowerVec[select.get_index()]->dec_count();
                    sound = 2;
                }
            }
            else if(select.get_x() + select.get_height()*3 <= mx && mx < select.get_x() + select.get_height()*4) //免伤词缀
            {
                if(!meleeTowerVec[select.get_index()]->get_avoid())
                {
                    if(meleeTowerVec[select.get_index()]->get_count() < 2)
                    {
                        meleeTowerVec[select.get_index()]->set_avoid(true);
                        meleeTowerVec[select.get_index()]->add_count();
                        sound = 1;
                        affixArr[3]--;
                    }
                }
                else
                {
                    meleeTowerVec[select.get_index()]->set_avoid(false);
                    meleeTowerVec[select.get_index()]->dec_count();
                    sound = 2;
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
                sound = 2;
            }
        }
        else //远程塔
        {
            if(select.get_x() <= mx && mx < select.get_x() + select.get_height()) //群伤词缀
            {
                if(!rangedTowerVec[select.get_index()]->get_aoe())
                {
                    if(rangedTowerVec[select.get_index()]->get_count() < 2)
                    {
                        rangedTowerVec[select.get_index()]->set_aoe(true);
                        rangedTowerVec[select.get_index()]->add_count();
                        sound = 1;
                        affixArr[2]--;
                    }
                }
                else
                {
                    rangedTowerVec[select.get_index()]->set_aoe(false);
                    rangedTowerVec[select.get_index()]->dec_count();
                    sound = 2;
                }
            }
            else if(select.get_x() + select.get_height() <= mx && mx < select.get_x() + select.get_height()*2) //放血词缀
            {
                if(!rangedTowerVec[select.get_index()]->get_bleed())
                {
                    if(rangedTowerVec[select.get_index()]->get_count() < 2)
                    {
                        rangedTowerVec[select.get_index()]->set_bleed(true);
                        rangedTowerVec[select.get_index()]->add_count();
                        sound = 1;
                        affixArr[4]--;
                    }
                }
                else
                {
                    rangedTowerVec[select.get_index()]->set_bleed(false);
                    rangedTowerVec[select.get_index()]->dec_count();
                    sound = 2;
                }
            }
            else if(select.get_x() + select.get_height()*2 <= mx && mx < select.get_x() + select.get_height()*3) //弱化词缀
            {
                if(!rangedTowerVec[select.get_index()]->get_weaken())
                {
                    if(rangedTowerVec[select.get_index()]->get_count() < 2)
                    {
                        rangedTowerVec[select.get_index()]->set_weaken(true);
                        rangedTowerVec[select.get_index()]->add_count();
                        sound = 1;
                        affixArr[5]--;
                    }
                }
                else
                {
                    rangedTowerVec[select.get_index()]->set_weaken(false);
                    rangedTowerVec[select.get_index()]->dec_count();
                    sound = 2;
                }
            }
            else //撤销塔
            {
                map.set_value(rangedTowerVec[select.get_index()]->get_x(), rangedTowerVec[select.get_index()]->get_y(), GRASS_VALUE);
                qDebug() << rangedTowerVec[select.get_index()]->get_x() << rangedTowerVec[select.get_index()]->get_y() << "处撤销远程塔";
                rangedTowerVec.erase(rangedTowerVec.begin() + select.get_index());
                money += 96;
                moneyLabel->setText("金币数：" + QString::number(money));
                select.set_display(false);
                sound = 2;
            }
        }
        if(sound == 1 && volume)
        {
            QSoundEffect* effect = new QSoundEffect(this);
            effect->setSource(QUrl("qrc:///sounds/add_affix.wav"));
            effect->setLoopCount(1);
            effect->play();
        }
        if(sound == 2 && volume)
        {
            QSoundEffect* effect = new QSoundEffect(this);
            effect->setSource(QUrl("qrc:///sounds/unload.wav"));
            effect->setLoopCount(1);
            effect->play();
        }
    }

    else //鼠标位置在非词缀选择框内
    {
        int sound = 0;
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
                        sound = 1;
                    }
                    else if(money >= 100)
                    {
                        meleeTowerVec.push_back(new MeleeTower(p.x, p.y, MELEETOWER_PATH));
                        map.set_value(p.x, p.y, MELEETOWER_VALUE);
                        qDebug() << p.x << p.y << "处安置近战塔";
                        money -= 100;
                        moneyLabel->setText("金币数：" + QString::number(money));
                        sound = 2;
                    }
                    flag = false;
                }
            }

        flag = true;
        for(auto p : rangedTowerPosVec) //鼠标位置在远程塔安置点内
        {
            if(flag == false) break;
            if((p.x * UNIT_LENGTH <= mx) && (mx < (p.x+1) * UNIT_LENGTH) &&
              (p.y * UNIT_LENGTH <= my) && (my < (p.y+1) * UNIT_LENGTH))
            {
                if(map(p.x, p.y) == RANGEDTOWER_VALUE) //路径点已存在远程塔
                {
                    for(int index = 0; index < rangedTowerVec.size(); ++index)
                        if(rangedTowerVec[index]->get_x() == p.x && rangedTowerVec[index]->get_y() == p.y)
                        {
                            if(select.get_type() == RANGEDTOWER_VALUE && index == select.get_index() && select.get_display()) //点击同一个塔
                                select.set_display(false);
                            else
                            {
                                select.change(RANGEDTOWER_VALUE, rangedTowerVec[index]->get_pos(), index); //换显示框内容
                                select.set_display(true);
                            }
                            break;
                        }
                    sound = 1;
                }
                else if(money >= 160)
                {
                    rangedTowerVec.push_back(new RangedTower(p.x, p.y, RANGEDTOWER_PATH));
                    map.set_value(p.x, p.y, RANGEDTOWER_VALUE);
                    qDebug() << p.x << p.y << "处安置远程塔";
                    money -= 160;
                    moneyLabel->setText("金币数：" + QString::number(money));
                    sound = 2;
                }
                flag = false;
            }
        }
        if(volume)
        {
            if(sound == 0)
            {
                QSoundEffect* effect = new QSoundEffect(this);
                effect->setSource(QUrl("qrc:///sounds/unavailable.wav"));
                effect->setLoopCount(1);
                effect->play();
            }
            else if(sound == 1)
            {
                QSoundEffect* effect = new QSoundEffect(this);
                effect->setSource(QUrl("qrc:///sounds/click.wav"));
                effect->setLoopCount(1);
                effect->play();
            }
            else
            {
                QSoundEffect* effect = new QSoundEffect(this);
                effect->setSource(QUrl("qrc:///sounds/load.wav"));
                effect->setLoopCount(1);
                effect->play();
            }
        }
    }

    update();
}
