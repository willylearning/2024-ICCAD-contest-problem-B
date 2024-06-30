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

struct PinInformation {
    string instName;
    string libPinName;
};

struct Vertex {
    vector<PinInformation> preprocessors;
    vector<PinInformation> adjacences;
    bool color = 0;
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

struct PlacementRow {
    double startX;
    double startY;
    double siteWidth;
    double siteHeight;
    int totalNumOfSites;
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

struct PlaceDot {
    double placeX;
    double placeY;
    bool placedOrNot = 0;
};

void place(PinInformation &adjpoint, map<string, FlipFlop>& FlipFlops, map<string, Instance>& outInstance, vector<PlacementRow>& placementRows) {
    string inst = adjpoint.instName;
    string lib = adjpoint.libPinName;
    if() {
        outInstance[inst]
    }

}

void bfs_algorithm(map<string, FlipFlop>& FlipFlops, map<string, Instance>& outInstance, map<PinInformation, Vertex>& vertex, const PinInformation &pin, vector<PlacementRow>& placementRows) {
    int connectCount = vertex[pin].adjacences.size();
    vertex[pin].color = 1;
    while (connectCount == 0) {
        // check the color of connection
        PinInformation adjpoint = vertex[pin].adjacences[connectCount-1];
        if (vertex[adjpoint].color == 1) {
            connectCount--;
            break;
        } else {
            vertex[adjpoint].color = 1;
            place(adjpoint, FlipFlops, outInstance);
            connectCount--;
        }
    }
    
}

int main(int argc, char **argv) {
    map<int, NewNet> connectionline;
    map<string, FlipFlop> FlipFlops;
    map<string, Instance> outInstance;
    map<PinInformation, Vertex> vertex;
    vector<PlacementRow> placementRows;
}




