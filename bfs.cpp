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
#include <queue>
// #include <struct>

using namespace std;

class Box {
public:
    double x, y, width, height;
    string name;
    bool visit = false;

    // 構造函數
    Box(double x, double y, double width, double height, string name)
        : x(x), y(y), width(width), height(height), name(name) {}
};

struct Cluster {
    std::vector<double> mode;
    std::vector<std::vector<double>> original_points;
    std::vector<std::vector<double>> shifted_points;
    std::vector<int> original_reg_idx;
    std::vector<int> shifted_reg_idx;
};

template<typename T>
class FixedQueue {
private:
    vector<T> q;
    size_t size;
public:
    FixedQueue(size_t s) : size(s) {
        q.reserve(size);
    }

    void push(const T& t) {
        if (q.size() >= size) {
            q.erase(q.begin());
        }
        q.push_back(t);
    }

    T pop() {
        T t = q.front();
        q.erase(q.begin());
        return t;
    }

    T& front() {
        return q.front();
    }

    T& back() {
        return q.back();
    }

    bool empty() const {
        return q.empty();
    }

    size_t getSize() const {
        return q.size();
    }

    void clear() {
        q.clear();
    }
};

// void push(vector<Box>& q) {
//     if (q.size() >= size) {
//         q.erase(q.begin());
//     }
//     q.push_back(t);
//     }

// void pop() {
//     double a = q.front();
//     q.erase(q.begin());
//     return a;
//     }

bool compareByX(Box& a, Box& b) {
    return a.x < b.x;
}

void bfs_algorithm(vector<Box>& queue, vector<Box>& placement) {
    sort(queue.begin(), queue.end(), compareByX);
    vector<Box> virtualQueue;
    virtualQueue.reserve(queue.size());
    for(int i = 0; i<queue.size(); i++){
        // virtualQueue[i] = queue[i];
        virtualQueue.push_back(queue[i]);
    }
    // start point
    queue[0].visit = true;
    // while()

    // 使用 queue 进行 BFS
    std::queue<Box*> bfsQueue;
    
    // start point
    if (!queue.empty()) {
        queue[0].visit = true;
        bfsQueue.push(&queue[0]);
    }

    while (!bfsQueue.empty()) {
        Box* currentBox = bfsQueue.front();
        bfsQueue.pop();

        // 处理 currentBox
        placement.push_back(*currentBox);

        // 遍历 virtualQueue 找到未访问的邻居节点并标记访问
        for (auto& box : virtualQueue) {
            if (!box.visit) {
                box.visit = true;
                bfsQueue.push(&box);
            }
        }
    }
}

int main(int argc, char **argv) {
    vector<Box> boxes;
    vector<Box> placement;
}




