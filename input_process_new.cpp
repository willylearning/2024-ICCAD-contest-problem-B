#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <cstdio>
#include "MeanShift.h"

using namespace std;

struct diesize {
    double x_left;
    double y_bottom;
    double x_right;
    double y_up;
};

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
    double Qpindelay;
    double GatePower;
};

struct Instance {
    string name;
    string flipFlopName;
    double x;
    double y;
};

struct Net {
    string name;
    int numPins;
    vector<string> pins;
};

struct PlacementRow {
    double startX;
    double startY;
    double siteWidth;
    double siteHeight;
    int totalNumOfSites;
};

// struct Qpindelay {
//     string flipflopname;
//     double value;
// };

struct TimingSlack {
    string instanceCellName;
    string pinName;
    double slack;
};

// struct GatePower {
//     string libCellName;
//     double powerConsumption;
// };

// void plotData(const vector<PlacementRow>& placementRows) {
//     FILE *gnuplotPipe = popen("gnuplot -persistent", "w");

//     // if (gnuplotPipe) {
//     //     fprintf(gnuplotPipe, "set title 'Sine Wave'\n");
//     //     fprintf(gnuplotPipe, "set xlabel 'X'\n");
//     //     fprintf(gnuplotPipe, "set ylabel 'Y'\n");
//     //     fprintf(gnuplotPipe, "plot '-' with lines title 'sin(x)'\n");

//     //     for (size_t i = 0; i < x.size(); ++i) {
//     //         fprintf(gnuplotPipe, "%f %f\n", x[i], y[i]);
//     //     }
//     if (gnuplotPipe) {
//         fprintf(gnuplotPipe, "set title 'Placement Points'\n");
//         fprintf(gnuplotPipe, "set xlabel 'X'\n");
//         fprintf(gnuplotPipe, "set ylabel 'Y'\n");
//         fprintf(gnuplotPipe, "set grid\n");
//         fprintf(gnuplotPipe, "set style data points\n");
//         fprintf(gnuplotPipe, "set palette model RGB defined ( 1 'red', 2 'blue')\n");
//         fprintf(gnuplotPipe, "plot '-' using 1:2:3 with points pt 7 lc palette notitle\n");

//         for (const auto& row : placementRows) {
//             for (int i = 0; i < row.totalNumOfSites; ++i) {
//                 double x = row.startX + i * row.siteWidth;
//                 double y = row.startY;
//                 fprintf(gnuplotPipe, "%f %f %d\n", x, y, 1);
//             }
//         }

//         fprintf(gnuplotPipe, "e\n");  // 结束数据输入
//         fflush(gnuplotPipe);          // 确保所有数据已写入
//         pclose(gnuplotPipe);          // 关闭 gnuplot 管道
//     } else {
//         cerr << "Error: Could not open gnuplot pipe.\n";
//     }
// }

vector<vector<double> > load_points(const char *filename, vector<string> &reg_name) {
    vector<vector<double>> points;
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        double x, y;
        string reg;

        // 使用 istringstream 从行中读取 x, y 和标签信息
        istringstream iss(line);
        char comma;
        if (iss >> x >> comma >> y >> reg) {
            vector<double> point;
            point.push_back(x);
            point.push_back(y);
            points.push_back(point);

            // 将标签添加到 reg_name 向量中
            reg_name.push_back(reg);
        }
    }
    return points;
}

void print_points(vector<vector<double> > points){
    for(int i=0; i<points.size(); i++){
        for(int dim = 0; dim<points[i].size(); dim++) {
            printf("%f ", points[i][dim]);
        }
        printf("\n");
    }
}

double CostCount(vector<FlipFlop>& flipFlops, map<string, double> &weights) {
    double cost = 0;
    double cost_a = 0;
    double cost_b = 0;
    double cost_r = 0;
    // double cost_r = 0;
    for(auto& flipFlop : flipFlops) {
        cost_a += flipFlop.Qpindelay;
        cost_b += flipFlop.GatePower;
    }
    cost = cost_a*weights["Alpha"] + cost_b*weights["Beta"];
    return cost;
}

int main(int argc, char *argv[]) {
    ifstream file(argv[1]);
    string line;

    // Data structures to store parsed information
    map<string, double> weights;
    vector<Pin> inputPins;
    vector<Pin> outputPins;
    vector<FlipFlop> flipFlops;
    vector<Instance> instances;
    vector<Net> nets;
    double binWidth = 0;
    double binHeight = 0;
    double binMaxUtil = 0;
    vector<PlacementRow> placementRows;
    vector<TimingSlack> timingSlacks;
    // vector<GatePower> gatePowers;
    diesize size;
    // vector<Qpindelay> qpindelays; // Change to vector
    
    // get the information of ff
    int tnsCount = 0;
    int powerCount = 0;
    string flipflopname;
    double value;
    double cost = 0;

    while (getline(file, line)) {
        istringstream iss(line);
        string key;
        iss >> key;

        if (key == "DieSize") {
            iss >> size.x_left >> size.y_bottom >> size.x_right >> size.y_up;
        } else if (key == "Alpha" || key == "Beta" || key == "Gamma" || key == "Lambda" || key == "DisplacementDelay") {
            double value;
            iss >> value;
            weights[key] = value;
        } else if (key == "Input") {
            Pin pin;
            iss >> pin.name >> pin.x >> pin.y;
            inputPins.push_back(pin);
        } else if (key == "Output") {
            Pin pin;
            iss >> pin.name >> pin.x >> pin.y;
            outputPins.push_back(pin);
        } else if (key == "FlipFlop") {
            FlipFlop flipFlop;
            iss >> flipFlop.bits >> flipFlop.name >> flipFlop.width >> flipFlop.height >> flipFlop.pinCount;
            for (int i = 0; i < flipFlop.pinCount; ++i) {
                getline(file, line);
                istringstream pinIss(line);
                Pin pin;
                string type;
                pinIss >> type >> pin.name >> pin.x >> pin.y;
                flipFlop.pins.push_back(pin);
            }
            flipFlops.push_back(flipFlop);
        } else if (key == "Inst") {
            Instance instance;
            iss >> instance.name >> instance.flipFlopName >> instance.x >> instance.y; // read instance
            instances.push_back(instance);
        } else if (key == "Net") {
            Net net;
            iss >> net.name >> net.numPins;
            for (int i = 0; i < net.numPins; ++i) {
                getline(file, line);
                istringstream pinIss(line);
                string pin;
                string connect;
                pinIss >> pin >> connect;
                net.pins.push_back(connect);
            }
            nets.push_back(net);
        } else if (key == "BinWidth") {
            iss >> binWidth;
        } else if (key == "BinHeight") {
            iss >> binHeight;
        } else if (key == "BinMaxUtil") {
            iss >> binMaxUtil;
        } else if (key == "PlacementRows") {
            PlacementRow placementRow;
            iss >> placementRow.startX >> placementRow.startY >> placementRow.siteWidth >> placementRow.siteHeight >> placementRow.totalNumOfSites;
            placementRows.push_back(placementRow);
        } else if (key == "TimingSlack") {
            TimingSlack timingSlack;
            iss >> timingSlack.instanceCellName >> timingSlack.pinName >> timingSlack.slack;
            timingSlacks.push_back(timingSlack);
        } else if (key == "GatePower") {
            // GatePower gatePower;
            // iss >> gatePower.libCellName >> gatePower.powerConsumption;
            // gatePowers.push_back(gatePower);
            iss >> flipflopname >> value;
            if(flipflopname == flipFlops[powerCount].name) {
                flipFlops[powerCount].GatePower = value;
                powerCount++;
            }
        } else if (key == "QpinDelay") {
            // Qpindelay qpindelay;
            // iss >> qpindelay.flipflopname >> qpindelay.value;
            // qpindelays.push_back(qpindelay); // Add to vector
            iss >> flipflopname >> value;
            if (flipflopname == flipFlops[tnsCount].name){
                flipFlops[tnsCount].Qpindelay = value;
                tnsCount++;
            }
        }
    }
    // plotData(placementRows);
    vector<vector<double>> points;
    vector<string> reg_name;
    // ofstream csvFile("testcase1.csv");
    for(const auto& instance : instances){
        // Find the corresponding FlipFlop
        FlipFlop* correspondingFlipFlop = nullptr;
        for (auto& flipFlop : flipFlops) {
            if (flipFlop.name == instance.flipFlopName) {
                correspondingFlipFlop = &flipFlop;
                break;
            }
        }

        if(correspondingFlipFlop){
            // Bottom-left corner of the cell push into points
            vector<double> point;
            point.push_back(instance.x);
            point.push_back(instance.y);
            points.push_back(point);

            reg_name.push_back(instance.name);
        }
    }
    cost = CostCount(flipFlops, weights);
    cout << "\nCost: "<< binWidth << endl;
    return 0;

}
