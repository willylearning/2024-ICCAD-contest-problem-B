#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
// #include <struct>

using namespace std;

class Box {
public:
    double x, y, width, height, x_new, y_new;
    string name;

    // 構造函數
    Box(int x, int y, int width, int height, int x_new, int y_new, string name)
        : x(x), y(y), x_new(x_new), y_new(y_new), width(width), height(height), name(name) {}
};

bool compareByX(const Box& a, const Box& b) {
    return a.x < b.x;
}

void ligalization(vector<Box>& points, vector<Box>& placement, double StartPointX, double StartPointY, double PlaceWidth, double PlaceHeight) {
    sort(points.begin(), points.end(), compareByX);
    for(int i = 0; i < points.size(); i++) {
        if(i == 0) {
            placement[0].x = StartPointX;
            placement[0].y = StartPointY;
            double next = placement[0].x + PlaceWidth;
            double dis = placement[0].x + points[0].width;
            while(int(next/dis) != 0) {
                if(int(next/dis) == 0)
                    break;
                next += PlaceWidth;
            } 
            placement[0].x_new = next;
        } else {
            placement[i].x = placement[i-1].x_new;
            placement[i].y = points[i].y;
            double next = placement[i].x + PlaceWidth;
            double dis = placement[i].x + points[i].width;
            while(int(next/dis) != 0) {
                if(int(next/dis) == 0)
                    break;
                next += PlaceWidth;
            } 
            placement[i].x_new = next;
        }
        
    }
}


int main(int argc, char **argv) {
    vector<Box> boxes;
    vector<Box> placement;
}


