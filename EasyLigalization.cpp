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

void ligalization(vector<Box>& points, vector<Box>& placement, double start_point, double end_point, double PlaceWidth, double PlaceHeight) {
    sort(points.begin(), points.end(), compareByX);
    for(int i = 0; i < points.size(); i++) {
        if(i == 0) {
            placement[0].x = 
            placement[0].y = 
        } else {

        }
        
    }
}


int main(int argc, char **argv) {
    vector<Box> boxes;
    vector<Box> placement;
}


