#ifndef MAP_H
#define MAP_H

#include <iostream>
#include <QVector>
#include <fstream>
#include <sstream>
#include <QDebug>
using namespace std;

#define GRASS_VALUE 0
#define ROAD_VALUE 1
#define MELEETOWER_VALUE 2
#define RANGEDTOWER_VALUE 3

#define UNIT_LENGTH 100

struct Position
{
    int x;
    int y;
    Position(int _x, int _y) : x(_x), y(_y) {}
};

class Map
{
    int m;
    int n;
    int** mapMatrix; //地图的二维矩阵
    int num; //路径条数
    QVector<QVector<Position>> roads; //存放若干条路径
public:
    Map();
    int get_m() const { return m; }
    int get_n() const { return n; }
    int get_num() const { return num; }
    int operator()(int i, int j) const { return mapMatrix[i][j]; } //返回二维数组中的值
    void set_value(int i, int j, int data) { mapMatrix[i][j] = data; } //修改二维数组里的值

    QVector<Position> get_road(int i) const { return roads[i]; } //返回第i条路径
    Position get_pos(int i, int j) const { return roads[i][j]; } //返回第i条路径上第j个坐标点

    void map2file(const char* src);
    void file2map(const char* src);
};

#endif // MAP_H
