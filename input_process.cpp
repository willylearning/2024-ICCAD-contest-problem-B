#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
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

struct Qpindelay {
    string flipflopname;
    double value;
};

struct TimingSlack {
    string instanceCellName;
    string pinName;
    double slack;
};

struct GatePower {
    string libCellName;
    double powerConsumption;
};

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
    vector<GatePower> gatePowers;
    diesize size;
    vector<Qpindelay> qpindelays; // Change to vector

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
            GatePower gatePower;
            iss >> gatePower.libCellName >> gatePower.powerConsumption;
            gatePowers.push_back(gatePower);
        } else if (key == "QpinDelay") {
            Qpindelay qpindelay;
            iss >> qpindelay.flipflopname >> qpindelay.value;
            qpindelays.push_back(qpindelay); // Add to vector
        }
    }

    // Output parsed data
    cout << "Weights:" << endl;
    for (const auto& pair : weights) {
        cout << pair.first << ": " << pair.second << endl;
    }

    cout << "\ndiesize:" << endl;
    cout << "x_left : "<< size.x_left << ", y_bottom : "<< size.y_bottom <<", x_right : "<< size.x_right <<", y_up : "<< size.y_up << endl;

    cout << "\nInput Pins:" << endl;
    for (const auto& pin : inputPins) {
        cout << "Name: " << pin.name << ", X: " << pin.x << ", Y: " << pin.y << endl;
    }

    cout << "\nOutput Pins:" << endl;
    for (const auto& pin : outputPins) {
        cout << "Name: " << pin.name << ", X: " << pin.x << ", Y: " << pin.y << endl;
    }

    cout << "\nFlipflop:" << endl;

    for (const auto& flipflop : flipFlops) {
        cout << "bits: " << flipflop.bits << ", name: " << flipflop.name << ", width: " << flipflop.width << ", height: " << flipflop.height << endl;
        cout << "Pins:" << endl;
        for (const auto& pin : flipflop.pins) {
            cout << "Name: " << pin.name << ", X: " << pin.x << ", Y: " << pin.y << endl;
        }
    }

    cout << "\nInstances:" << endl;
    for (const auto& instance : instances) {
        cout << "Name: " << instance.name << ", FlipFlop Name: " << instance.flipFlopName << ", X: " << instance.x << ", Y: " << instance.y << endl;
    }

    cout << "\nNets:" << endl;
    for (const auto& net : nets) {
        cout << "Name: " << net.name << ", Number of Pins: " << net.numPins << endl;
        cout << "Pins:" << endl;
        for (const auto& pin : net.pins) {
            cout << "   " << pin << endl;
        }
    }

    cout << "\nQpinDelay:" << endl;
    for (const auto& delay : qpindelays) {
        cout << "flipflopname: " << delay.flipflopname << ", value: " << delay.value << endl;
    }
    
    

    cout << "\nTimingSlacks:" << endl;
    for (const auto& timingSlack : timingSlacks) {
        cout << "Instance Cell Name: " << timingSlack.instanceCellName << ", Pin Name: " << timingSlack.pinName << ", Slack: " << timingSlack.slack << endl;
    }

    cout << "\nGatePowers:" << endl;
    for (const auto& gatePower : gatePowers) {
        cout << "Library Cell Name: " << gatePower.libCellName << ", Power Consumption: " << gatePower.powerConsumption << endl;
    }

    cout << "\nBinWidth:"<< binWidth << endl;
    cout << "BinHeight:"<< binHeight << endl;
    cout << "BinMaxUtil:"<< binMaxUtil << endl;

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

    // Cluster
    MeanShift *msp = new MeanShift();
    double kernel_bandwidth = 1500;

    vector<Cluster> clusters = msp->cluster(points, kernel_bandwidth); // generate clustering result

    FILE *fp = fopen("result1.csv", "w");
    if(!fp){
        perror("Couldn't write result.csv");
        exit(0);
    }

    printf("\n====================\n");
    printf("Found %lu clusters\n", clusters.size());
    printf("====================\n\n");

    int reg_cnt = 0;
    int reg_tmp = 0;
    cout << "reg_name size : " << reg_name.size() << "\n";

    for(int cluster = 0; cluster < clusters.size(); cluster++){
        printf("Cluster %i:\n", cluster);
        fprintf(fp, "Cluster %i:\n", cluster); // need to write clusters into result

        int cluster_reg_cnt = clusters[cluster].original_points.size();
        while(cluster_reg_cnt >= 1){
            if(cluster_reg_cnt >= 4){ // can bank into 4-bit FF
                for(int i=0; i<4; i++){
                    reg_cnt++;
                    clusters[cluster].original_reg_idx.push_back(reg_cnt);
                    clusters[cluster].shifted_reg_idx.push_back(reg_name.size() + 1 + cluster);
                    // cout << "reg" << reg_cnt << " map reg" << reg_name.size() + 1 + cluster << " ";
                }
                cluster_reg_cnt -= 4;
            }else if(cluster_reg_cnt >= 2){ // can bank into 2-bit FF
                for(int i=0; i<2; i++){
                    reg_cnt++;
                    clusters[cluster].original_reg_idx.push_back(reg_cnt);
                    clusters[cluster].shifted_reg_idx.push_back(reg_name.size() + 1 + cluster);
                    // cout << "reg" << reg_cnt << " map reg" << reg_name.size() + 1 + cluster << " ";
                }
                cluster_reg_cnt -= 2;
            }else if(cluster_reg_cnt >= 1){ // cannot bank
                reg_cnt++;
                clusters[cluster].original_reg_idx.push_back(reg_cnt);
                clusters[cluster].shifted_reg_idx.push_back(reg_name.size() + 1 + cluster);
                // cout << "reg" << reg_cnt << " map reg" << reg_name.size() + 1 + cluster << " ";
                cluster_reg_cnt--;
            }
        }

        for(int point = 0; point < clusters[cluster].original_points.size(); point++){
            cout << "reg" << clusters[cluster].original_reg_idx[point] << " map reg" << clusters[cluster].shifted_reg_idx[point] << " ";
            for(int dim = 0; dim < clusters[cluster].original_points[point].size(); dim++){
                // fprintf(fp, "%s ", reg_name[point]);
                printf("%f ", clusters[cluster].original_points[point][dim]);
                fprintf(fp, "%f ", clusters[cluster].original_points[point][dim]);
            }
            printf("-> ");
            fprintf(fp, "-> ");
            for(int dim = 0; dim < clusters[cluster].shifted_points[point].size(); dim++){
                printf("%f ", clusters[cluster].shifted_points[point][dim]);
                fprintf(fp, dim?" %f":"%f", clusters[cluster].shifted_points[point][dim]);
            }
            printf("\n");
            fprintf(fp, "\n");
        }
        printf("\n");
    }
    fclose(fp);

    return 0;

}
