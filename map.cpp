#include "map.h"

Map::Map() //默认地图
{
    m = 12;
    n = 8;
    mapMatrix = new int*[m];
    int i, j;
    for(i = 0; i < m; ++i)
        mapMatrix[i] = new int[n];
    //初始化
    for(i = 0; i < m; ++i)
        for(j = 0; j < n; ++j)
            mapMatrix[i][j] = GRASS_VALUE;
    //添加路径
    QVector<Position> road;
    Position p1(11, 5); road.push_back(p1);
    Position p2(10, 5); road.push_back(p2);
    Position p3(10, 6); road.push_back(p3);
    Position p4(9, 6); road.push_back(p4);
    Position p5(8, 6); road.push_back(p5);
    Position p6(7, 6); road.push_back(p6);
    Position p7(7, 5); road.push_back(p7);
    Position p8(7, 4); road.push_back(p8);
    Position p9(6, 4); road.push_back(p9);
    Position p10(5, 4); road.push_back(p10);
    Position p11(4, 4); road.push_back(p11);
    Position p12(4, 5); road.push_back(p12);
    Position p13(4, 6); road.push_back(p13);
    Position p14(3, 6); road.push_back(p14);
    Position p15(2, 6); road.push_back(p15);
    Position p16(1, 6); road.push_back(p16);
    Position p17(0, 6); road.push_back(p17);
    roads.push_back(road);
    //路径节点设为1
    for(i = 0; i < (int)roads.size(); ++i)
        for(j = 0; j < (int)roads[i].size(); ++j)
            mapMatrix[roads[i][j].x][roads[i][j].y] = ROAD_VALUE;
    num = roads.size();
}

void Map::map2file(const char* src)
{
    ofstream fout(src);
    fout << m << ' ' << n << endl;
    fout << num << endl;
    int i, j;
    for(i = 0; i < (int)roads.size(); ++i)
    {
        for(j = 0; j < (int)roads[i].size(); ++j)
            fout << '(' << roads[i][j].x << ',' << roads[i][j].y << ") ";
        fout << endl;
    }
    fout.close();
}

void Map::file2map(const char *src)
{
    //释放原来的二维数组和路径容器
    int i, j;
    for(i = 0; i < m; ++i)
        delete[] mapMatrix[i];
    delete[] mapMatrix;
    roads.clear();
    //读取地图尺寸和路径条数
    ifstream fin(src);
    fin >> m >> n >> num;
    //重新生成二维数组并初始化
    mapMatrix = new int*[m];
    for(i = 0; i < m; ++i)
        mapMatrix[i] = new int[n];
    for(i = 0; i < m; ++i)
        for(j = 0; j < n; ++j)
            mapMatrix[i][j] = GRASS_VALUE;
    string line;
    while(getline(fin, line)) //读取一行路径存放到line变量中
    {
        QVector<Position> road;
        stringstream ss(line);
        string tmp;
        while(getline(ss, tmp, ' ')) //每个坐标按空格分割，存放到tmp变量中
        {
            int index = tmp.find(','); //找到横纵坐标分割处
            int x = stoi(tmp.substr(1, index - 1));
            int y = stoi(tmp.substr(index + 1, tmp.length() - index - 2));
            Position p(x, y);
            road.push_back(p);
        }
        roads.push_back(road);
    }
    //路径处设为1
    for(i = 0; i < (int)roads.size(); ++i)
        for(j = 0; j < (int)roads[i].size(); ++j)
            mapMatrix[roads[i][j].x][roads[i][j].y] = ROAD_VALUE;
    fin.close();
}
