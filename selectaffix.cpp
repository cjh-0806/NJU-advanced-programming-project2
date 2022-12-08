#include "selectaffix.h"

SelectAffix::SelectAffix()
    :type(0), index(-1), display(false),
     length(UNIT_LENGTH*0.5), height(UNIT_LENGTH*0.5), pos(0, 0)
{
    affixPaths = nullptr;
}

void SelectAffix::change(int n, Position towerPos, int i)
{
    if(affixPaths != nullptr)
        delete[] affixPaths;
    type = n;
    index = i;
    pos = Position(towerPos.x*UNIT_LENGTH, max(0.0, (towerPos.y-0.5)*UNIT_LENGTH));
    if(type == MELEETOWER_VALUE)
    {
        length = UNIT_LENGTH*2;
        affixPaths = new QString[4];
        affixPaths[0] = ":/pictures/affix0.jpg";
        affixPaths[1] = ":/pictures/affix1.jpg";
        affixPaths[2] = ":/pictures/affix2.jpg";
        affixPaths[3] = ":/pictures/unload.jpg";
    }
    else
    {
        length = UNIT_LENGTH;
        affixPaths = new QString[2];
        affixPaths[0] = ":/pictures/affix3.jpg";
        affixPaths[1] = ":/pictures/unload.jpg";
    }
}
