#ifndef SELECTAFFIX_H
#define SELECTAFFIX_H

#include <QString>
#include <QVector>

#include <iostream>
#include <algorithm>
#include "map.h"
using namespace std;

class SelectAffix
{
    int type; //近战塔2，远程塔3
    int index; //塔下标
    bool display; //是否显示
    bool gray; //灰色
    int length;
    int height;
    Position pos;

public:
    QString* affixPaths;
    SelectAffix();
    void change(int, Position, int);

    int get_type() const { return type; }
    int get_index() const { return index; }
    bool get_display() const { return display; }
    bool get_gray() const { return gray; }

    Position get_pos() const { return pos; }
    int get_x() const { return pos.x; }
    int get_y() const { return pos.y; }

    int get_length() const { return length; }
    int get_height() const { return height; }

    void set_display(bool d) { display = d; }
    void set_gray(bool g) { gray = g; }
};

#endif // SELECTAFFIX_H
