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
    QVector<Position> road1;
    road1.push_back(Position(11, 5));
    road1.push_back(Position(10, 5));
    road1.push_back(Position(10, 6));
    road1.push_back(Position(9, 6));
    road1.push_back(Position(8, 6));
    road1.push_back(Position(7, 6));
    road1.push_back(Position(7, 5));
    road1.push_back(Position(7, 4));
    road1.push_back(Position(6, 4));
    road1.push_back(Position(5, 4));
    road1.push_back(Position(4, 4));
    road1.push_back(Position(4, 5));
    road1.push_back(Position(4, 6));
    road1.push_back(Position(3, 6));
    road1.push_back(Position(2, 6));
    road1.push_back(Position(1, 6));
    road1.push_back(Position(0, 6));
    roads.push_back(road1);
    QVector<Position> road2;
    road2.push_back(Position(11, 2));
    road2.push_back(Position(10, 2));
    road2.push_back(Position(9, 2));
    road2.push_back(Position(9, 1));
    road2.push_back(Position(9, 0));
    road2.push_back(Position(8, 0));
    road2.push_back(Position(7, 0));
    road2.push_back(Position(7, 1));
    road2.push_back(Position(7, 2));
    road2.push_back(Position(6, 2));
    road2.push_back(Position(5, 2));
    road2.push_back(Position(4, 2));
    road2.push_back(Position(3, 2));
    road2.push_back(Position(2, 2));
    road2.push_back(Position(1, 2));
    road2.push_back(Position(1, 1));
    road2.push_back(Position(1, 0));
    roads.push_back(road2);
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
    getline(fin, line);
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

Map& Map::operator=(const Map& map)
{
    if (this == &map)
        return *this;
    this->m = map.m;
    this->n = map.n;
    this->num = map.num;
    this->roads = map.roads;
    if(mapMatrix != NULL)
    {
        delete mapMatrix;
        mapMatrix = NULL;
    }
    mapMatrix = new int*[m];
    int i, j;
    for(i = 0; i < m; ++i)
        mapMatrix[i] = new int[n];
    //初始化
    for(i = 0; i < m; ++i)
        for(j = 0; j < n; ++j)
            mapMatrix[i][j] = GRASS_VALUE;
    for(i = 0; i < (int)roads.size(); ++i)
        for(j = 0; j < (int)roads[i].size(); ++j)
            mapMatrix[roads[i][j].x][roads[i][j].y] = ROAD_VALUE;
    return *this;
}
