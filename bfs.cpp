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

// class Box {
// public:
//     double x, y, width, height;
//     string name;
//     bool visit = false;

//     // 構造函數
//     Box(double x, double y, double width, double height, string name)
//         : x(x), y(y), width(width), height(height), name(name) {}
// };

// struct Cluster {
//     std::vector<double> mode;
//     std::vector<std::vector<double>> original_points;
//     std::vector<std::vector<double>> shifted_points;
//     std::vector<int> original_reg_idx;
//     std::vector<int> shifted_reg_idx;
// };

// template<typename T>
// class FixedQueue {
// private:
//     vector<T> q;
//     size_t size;
// public:
//     FixedQueue(size_t s) : size(s) {
//         q.reserve(size);
//     }

//     void push(const T& t) {
//         if (q.size() >= size) {
//             q.erase(q.begin());
//         }
//         q.push_back(t);
//     }

//     T pop() {
//         T t = q.front();
//         q.erase(q.begin());
//         return t;
//     }

//     T& front() {
//         return q.front();
//     }

//     T& back() {
//         return q.back();
//     }

//     bool empty() const {
//         return q.empty();
//     }

//     size_t getSize() const {
//         return q.size();
//     }

//     void clear() {
//         q.clear();
//     }
// };

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


// struct record {
//     vector<string> name;
// }
// map<int, record> line;

// bool compareByX(Box& a, Box& b) {
//     return a.x < b.x;
// }

struct Pin {
    string name;
    double x;
    double y;
};

struct FlipFlop {
    int bits;
    string name;
    double width;
    double height;
    int pinCount; // need to record
    vector<Pin> pins;
    vector<Pin> shifted_pins;
    double Qpindelay;
    double GatePower;
};

struct Instance {
    string name;
    string flipFlopName;
    double x;
    double y;
    double shiftedx;
    double shiftedy;
};

struct NewNetElement {
    string firstPin;
    string firstInstance;
};

struct NewNet {
    string name;
    int numPins;
    NewNetElement firstPin;
    vector<NewNetElement> pins;
};

void bfs_algorithm(const map<int, NewNet>& connectionline,const map<string, FlipFlop>& FlipFlops,const map<string, Instance>& outInstance) {
    for(const auto& pair : connectionline) {
        
    }
}

int main(int argc, char **argv) {
    map<int, NewNet> connectionline;
    map<string, FlipFlop> FlipFlops;
    map<string, Instance> outInstance;
}




