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
    double x, y, width, height;
    string name;

    // 構造函數
    Box(int x, int y, int width, int height, string name)
        : x(x), y(y), width(width), height(height), name(name) {}
};

struct Cluster {
    std::vector<double> mode;
    std::vector<std::vector<double>> original_points;
    std::vector<std::vector<double>> shifted_points;
    std::vector<int> original_reg_idx;
    std::vector<int> shifted_reg_idx;
};

// 定義一個 Point 結構
struct Point {
    double x;
    double y;

    // 構造函數
    Point(double x_val, double y_val) : x(x_val), y(y_val) {}
};

void Abacus(vector<vector<int *>>& points, vector<vector<int *>>& shifted_points, vector<Box>& placement) {
    // vector<vector<int>> a = shifted_points;
    int threshold = 100; // The height of the placement block
    Point site(0, 0);
    // vector<double> r;
    sort(shifted_points.begin(), shifted_points.end(), [](const vector<int>& a, const vector<int>& b) {
        return a[0] < b[0];
    });
    for(int i=0; i< shifted_points.size(); i++) {
        double CostBest = INFINITY;
        for(int j=0; j< shifted_points.size(); j++) {
            double Cost1 = INFINITY;
            // Place cell ci into row r
            site = PlaceRow(placement, false, j); // find the row to place, not placing cell into a dead space
            // Cost1 = ; // Calculate the cost
            bool d = CheckDeadSpace(placement); // Check if an available dead space exists
            double Cost2 = INFINITY;
            if(d == 1) {
                site = PlaceRow(placement, false, j); // find the row to place, placing cell into the dead space
                // Cost2 = ; // Calculate the cost
            }
            if(Cost1>Cost2) {
                CostBest = Cost2;
            }else {
                CostBest = Cost1;
            }

        }

    }

}

bool CheckDeadSpace(vector<Box>& placement) {
    // TODO
    
}

// 定義一個函數來回傳 Point 結構
Point createPoint(int x, int y) {
    return Point(x, y);
}

Point PlaceRow(vector<Box>& placement, bool k, int n) {
    double height = 100;
    double width = 100;
    double x, y;
    vector<double> site = {(0, 0)};
    if(k == true) {
        // TODO

        // for (int i = 1; i <= n; ++i) {
        //     for (int j = width; j >= placement[i-1].width; --j) {
        //         for (int k = 0; k <= height; ++k) {
        //             if (dp[i-1][j - boxes[i-1].width].height != numeric_limits<int>::max()) {
        //                 int new_height = dp[i-1][j - boxes[i-1].width].height + boxes[i-1].height;
        //                 int new_dead_space = dp[i-1][j - boxes[i-1].width].dead_space + (rowWidth - j);
        //                 if (new_height <= columnHeight) {
        //                     dp[i][j].height = min(dp[i][j].height, new_height);
        //                     dp[i][j].dead_space = min(dp[i][j].dead_space, new_dead_space);
        //                 }
        //             }
        //         }
        //     }
        // }
    } else {
        double x_next = placement[n+1].x;
        double y_next = placement[n+1].y;
        double x_current = placement[n].x;
        double y_current = placement[n].y;
        double x_previous = placement[n-1].x+placement[n-1].width;
        double y_previous = placement[n-1].y+placement[n-1].height;
        if(x_current+placement[n-1].width > x_next){
            // 
        }else {

        }
        
    }
    return Point(x, y);
}

void CountCost(double r, bool k) {

}

void AddCell() {

}

// 定義單元結構
struct Cell {
    int x, y, width, height;
    Cell(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}
};

// 比較函數，用於按 x 坐標排序單元
bool compareByX(const Cell &a, const Cell &b) {
    return a.x < b.x;
}

// 判斷兩個單元是否重疊
bool isOverlap(const Cell &a, const Cell &b) {
    return !(a.x + a.width <= b.x || b.x + b.width <= a.x || a.y + a.height <= b.y || b.y + b.height <= a.y);
}

// 合法化算法
void legalize(vector<Cell> &cells, int rowHeight) {
    // 按 x 坐標排序單元
    sort(cells.begin(), cells.end(), compareByX);

    // 逐個處理單元，避免重疊
    for (size_t i = 0; i < cells.size(); ++i) {
        for (size_t j = 0; j < i; ++j) {
            if (isOverlap(cells[i], cells[j])) {
                // 移動單元 i，直到不再重疊
                cells[i].x = cells[j].x + cells[j].width;
                cells[i].y = cells[j].y;
            }
        }
    }
}

int main(int argc, char **argv) {
    vector<Box> boxes;
    vector<Box> placement;
}