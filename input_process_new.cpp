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

map<string, string> lines;
map<string, string> opplines;

struct connection {
    double x_left;
    double y_bottom;
    double x_right;
    double y_up;
};

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

void parsePin(const std::string& pinStr, std::string& instName, std::string& libPinName) {
    size_t pos = pinStr.find('/');
    if (pos != std::string::npos) {
        instName = pinStr.substr(0, pos);
        libPinName = pinStr.substr(pos + 1);
    } else {
        instName = pinStr;
        libPinName = "";
    }
}

void printFlipFlops(const std::map<std::string, FlipFlop>& flipFlops) {
    for (const auto& pair : flipFlops) {
        const FlipFlop& ff = pair.second;
        std::cout << "FlipFlop: " << ff.name << ", ";
        std::cout << "  Bits: " << ff.bits << ", ";
        std::cout << "  Width: " << ff.width << ", ";
        std::cout << "  Height: " << ff.height << ", ";
        std::cout << "  Pin Count: " << ff.pinCount << ", ";
        std::cout << "  Pins:, ";
        for (const auto& pin : ff.pins) {
            std::cout << "    Name: " << pin.name << ", X: " << pin.x << ", Y: " << pin.y << "\n";
        }
    }
}

// void printFinalFlipFlops(const std::map<string, vector<Instance>> outInstance) {
//     for (const auto& pair : outInstance) {
//         std::cout << "Inst " << ff.name << ", ";
//         std::cout << "  Bits: " << ff.bits << ", ";
//         std::cout << "  Width: " << ff.width << ", ";
//         std::cout << "  Height: " << ff.height << ", ";
//         std::cout << "  Pin Count: " << ff.pinCount << ", ";
//         std::cout << "  Pins:, ";
//     }
// }

void printOutInstance(const std::map<std::string, Instance>& outInstance) {
    for (const auto& pair : outInstance) {
        const std::string& key = pair.first;
        const Instance& instance = pair.second;
        // const Instance& ff = pair.second;
        // std::cout << "Key: " << key << "\n";
        std::cout << "Inst " ;
        std::cout << instance.name;
        std::cout << " " << instance.flipFlopName;
        std::cout << " " << instance.x << " " << instance.y << "\n";
        // std::cout << "  Shifted X: " << instance.shiftedx << ", Shifted Y: " << instance.shiftedy << "\n";
        
    }
}
// void printOutInstances(const std::map<std::string, Instance>& outInstance) {
//     for (const auto& pair : outInstance) {
//         const std::string& key = pair.first;
//         const Instance& instance = pair.second;

//         std::cout << "FlipFlop Name: " << key << std::endl;
//         std::cout << "  Instance Name: " << instance.name << std::endl;
//         std::cout << "  X: " << instance.x << std::endl;
//         std::cout << "  Y: " << instance.y << std::endl;
//         std::cout << "  Shifted X: " << instance.shiftedx << std::endl;
//         std::cout << "  Shifted Y: " << instance.shiftedy << std::endl;
//         std::cout << std::endl;
//     }
// }

void printNet(const vector<Net> &nets) {
    // cout << "Net Name: " << net.name << endl;
    // cout << "Number of Pins: " << net.numPins << endl;
    // cout << "Pins:" << endl;
    for (int i=0; i < nets.size(); ++i){
        for (const auto& pin1 : nets[i].pins) {
            // cout << "  " << pin << endl;
            for (const auto& pin2 : nets[i].pins) {
                if (pin1!=pin2) 
                    cout << pin1 <<" map " << pin2 << endl;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    ifstream file(argv[1]);
    string line;

    // Data structures to store parsed information
    map<string, double> weights;
    vector<Pin> inputPins;
    vector<Pin> outputPins;
    // vector<FlipFlop> flipFlops;
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
    string name;
    double value;
    double cost = 0;

    map<string, vector<Net>> connectionline;
    map<int, map<string, vector<Net>>> connectionlineNum;
    map<string, FlipFlop> FlipFlops;
    map<string, Instance> outInstance;
    // map<string,> finalConnectionLine;

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
            FlipFlop ff;
            iss >> ff.bits >> ff.name >> ff.width >> ff.height >> ff.pinCount;
            for (int i = 0; i < ff.pinCount; ++i) {
                getline(file, line);
                istringstream pinIss(line);
                Pin pin;
                string type;
                pinIss >> type >> pin.name >> pin.x >> pin.y;
                ff.pins.push_back(pin);
                FlipFlops.emplace(ff.name, ff);
            }
            // flipFlops.push_back(flipFlop);
        } else if (key == "Inst") {
            Instance instance;
            iss >> instance.name >> instance.flipFlopName >> instance.x >> instance.y; // read instance
            instances.push_back(instance);
            outInstance.emplace(instance.flipFlopName, instance);
        } else if (key == "NumNets") {
            // cout << key << "\n";
            iss >> value;
            // cout << value << "\n";
            for (int j = 0; j < value; ++j) {
            getline(file, line);
            istringstream NetIss(line);
            // map<string, vector<Net>> connectionline;
            // map<int, map<string, vector<Net>>> connectionlineNum;
            NetIss >> name;
            // cout << name << "\n";
            if (name == "Net") {
            Net net;
            NetIss >> net.name >> net.numPins;
            // cout << net.name << net.numPins << "\n";
            for (int i = 0; i < net.numPins; ++i) {
                getline(file, line);
                istringstream pinIss(line);
                string pin;
                string connect;
                string instName;
                string libPinName;
                pinIss >> pin >> connect;
                // size_t pos = connect.find('/');
                // cout << "pin " << pin << "connect " << connect << "\n";
                parsePin(connect, instName, libPinName);
                // cout << "instName " << instName << " libPinName " << libPinName << " libPinNamelength "<< libPinName.length() <<"\n";
                lines.emplace(instName, libPinName);
                opplines.emplace(instName, libPinName);
                net.pins.push_back(pin);
                net.pins.push_back(connect);
            }
            nets.push_back(net);
            }
            // connectionlineNum.emplace(j, connectionline);
            }
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
            // if(flipflopname == flipFlops[powerCount].name) {
            //     flipFlops[powerCount].GatePower = value;
            //     powerCount++;
            // }
            FlipFlops[flipflopname].GatePower = value;
        } else if (key == "QpinDelay") {
            // Qpindelay qpindelay;
            // iss >> qpindelay.flipflopname >> qpindelay.value;
            // qpindelays.push_back(qpindelay); // Add to vector
            iss >> flipflopname >> value;
            // if (flipflopname == flipFlops[tnsCount].name){
            //     flipFlops[tnsCount].Qpindelay = value;
            //     tnsCount++;
            // }
            FlipFlops[flipflopname].Qpindelay = value;
        }
    }
    // plotData(placementRows);
    vector<vector<double>> points;
    vector<string> reg_name;
    // ofstream csvFile("testcase1.csv");
    for(const auto& instance : instances){
        // Find the corresponding FlipFlop
        // FlipFlop* correspondingFlipFlop = nullptr;
        // for (auto& flipFlop : flipFlops) {
        //     if (flipFlop.name == instance.flipFlopName) {
        //         correspondingFlipFlop = &flipFlop;
        //         break;
        //     }
        // }

        // if(correspondingFlipFlop){
        //     // Bottom-left corner of the cell push into points
        //     vector<double> point;
        //     point.push_back(instance.x);
        //     point.push_back(instance.y);
        //     points.push_back(point);

        //     reg_name.push_back(instance.name);
        // }
    }
    // cost = CostCount(flipFlops, weights);
    // cout << "\nCost: "<< binWidth << endl;
    // printFlipFlops(FlipFlops);

    // map<string, FlipFlop> FinalFlipFlops;
    
    cout << "CellInst " << outInstance.size()<< "\n";
    printOutInstance(outInstance);
    printNet(nets); 
    return 0;

}



// Output:
// 1. A flip-flop placement solution
// 2. A list of pin mapping between each input flip-flop pins and the output flip-flop pins

// The output must have no cell overlaps and every flip-flop placed on the defined sites of the placement
// rows while satisfying max placement utilization ratio. The max placement utilization ratio is a cell density
// constraint implemented by dividing the design into several placement bins, and each bin would have a
// maximum ratio defining the percentage of area coverage allowed in each bin.