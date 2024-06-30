#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
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

struct Gate {
    string name;
    double width;
    double height;
    int pinCount; // need to record
    vector<Pin> pins;
};

struct Instance {
    string inst_name;
    string type_name;
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

pair<string, int> splitString(const string& str) {
    size_t pos = str.find_last_not_of("0123456789");
    if (pos == string::npos || pos == str.length() - 1) {
        // 如果字符串没有数字部分或没有非数字部分
        return {str, -1}; 
    }
    string nonNumericPart = str.substr(0, pos + 1);
    int numericPart = stoi(str.substr(pos + 1));
    return {nonNumericPart, numericPart};
}

bool CompareByBits(vector<double>& a, vector<double>& b, map<pair<double, double>, pair<string, int>> points_namebits_map) {
    return points_namebits_map[make_pair(a[0], a[1])].second > points_namebits_map[make_pair(b[0], b[1])].second; // sort points by ff's bits in decreasing order
}

int main(int argc, char *argv[]) {
    ifstream file(argv[1]);
    string line;

    // Data structures to store parsed information
    map<string, double> weights;
    vector<Pin> inputPins;
    vector<Pin> outputPins;
    vector<FlipFlop> flipFlops;
    vector<Gate> gates;
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

    // map< pair<double, double>, pair<string, string> > inst_map;
    map<string, int> ffname_bits_map;
    pair<string, int> t;
    int max_bit = 0;

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
            if(flipFlop.bits > max_bit){
                max_bit = flipFlop.bits;
            }
            // fill in ffname_bits_map
            ffname_bits_map[flipFlop.name] = flipFlop.bits;
            // deal with each pin
            for (int i = 0; i < flipFlop.pinCount; ++i) {
                getline(file, line);
                istringstream pinIss(line);
                Pin pin;
                string type;
                pinIss >> type >> pin.name >> pin.x >> pin.y;
                flipFlop.pins.push_back(pin);
            }
            flipFlops.push_back(flipFlop);
        } else if (key == "Gate") {
            Gate gate;
            iss >> gate.name >> gate.width >> gate.height >> gate.pinCount;
            for (int i = 0; i < gate.pinCount; ++i) {
                getline(file, line);
                istringstream pinIss(line);
                Pin pin;
                string type;
                pinIss >> type >> pin.name >> pin.x >> pin.y;
                gate.pins.push_back(pin);
            }
            gates.push_back(gate);
        } else if (key == "Inst") {
            Instance instance;
            iss >> instance.inst_name >> instance.type_name >> instance.x >> instance.y; // read instance
            t = splitString(instance.inst_name); // t.first is string, t.second is int
            cout << t.first << endl;

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

    cout << "\nGate:" << endl;

    for (const auto& gate : gates) {
        cout << ", name: " << gate.name << ", width: " << gate.width << ", height: " << gate.height << endl;
        cout << "Pins:" << endl;
        for (const auto& pin : gate.pins) {
            cout << "Name: " << pin.name << ", X: " << pin.x << ", Y: " << pin.y << endl;
        }
    }

    cout << "\nInstances:" << endl;
    for (const auto& instance : instances) {
        cout << "Name: " << instance.inst_name << ", FlipFlop Name: " << instance.type_name << ", X: " << instance.x << ", Y: " << instance.y << endl;
        
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
    map<pair<double, double>, pair<string, int>> points_namebits_map;

    // ofstream csvFile("testcase1.csv");
    int n;
    for(const auto& instance : instances){
        // Find the corresponding FlipFlop
        for(auto& flipFlop : flipFlops){
            if(instance.type_name == flipFlop.name){
                // Bottom-left corner of the cell push into points
                vector<double> point;
                point.push_back(instance.x);
                point.push_back(instance.y);
                points.push_back(point);
                reg_name.push_back(instance.inst_name);

                n = ffname_bits_map[instance.type_name];
                points_namebits_map[make_pair(instance.x, instance.y)] = make_pair(instance.inst_name, n);
            }
        }
    }

    // Cluster
    MeanShift *msp = new MeanShift();
    double kernel_bandwidth = 10;

    vector<Cluster> clusters = msp->cluster(points, kernel_bandwidth); // generate clustering result

    ofstream fout("result.csv");
    if(!fout){
        perror("Couldn't write result.csv");
        exit(0);
    }

    cout << "\n====================\n";
    cout << "Found " << clusters.size() <<" clusters\n";
    cout << "====================\n\n";

    int reg_cnt = 0;
    int reg_tmp = 0;
    cout << "reg_name size : " << reg_name.size() << "\n";

    map<string, string> reg_map;

    for(int cluster = 0; cluster < clusters.size(); cluster++){
        sort(clusters[cluster].original_points.begin(), clusters[cluster].original_points.end(), 
            [&points_namebits_map](vector<double>& a, vector<double>& b){
                return CompareByBits(a, b, points_namebits_map);
            });
    }

    // for (const auto& cluster : clusters) {
    //     for (const auto& point : cluster.original_points) {
    //         cout << "(" << point[0] << ", " << point[1] << ") ";
    //     }
    //     cout << endl;
    // }

    int new_idx = reg_name.size() + 1;
    for(int cluster = 0; cluster < clusters.size(); cluster++){
        cout << "Cluster " << cluster << ":\n";
        fout << "Cluster " << cluster << ":\n";

        // banking process
        // construct a map between original_points[point] and its reg_type
        int bitcnt = 0;
        vector<string> strvec;
        
        for(int point = 0; point < clusters[cluster].original_points.size(); point++){
            // check which inst_name the point corresponds to in instances 
            string str = points_namebits_map[make_pair(clusters[cluster].original_points[point][0], clusters[cluster].original_points[point][1])].first;
            int b = points_namebits_map[make_pair(clusters[cluster].original_points[point][0], clusters[cluster].original_points[point][1])].second;
            if(b == max_bit){
                reg_map[str] = t.first + to_string(new_idx);
                new_idx++;
            }else{
                if(bitcnt + b == max_bit){ // can bank into the max flipflop
                    cout << "2" << endl;
                    strvec.push_back(str);
                    for(int i=0; i<strvec.size(); i++){
                        reg_map[strvec[i]] = t.first + to_string(new_idx);
                    }
                    new_idx++;
                    bitcnt = 0; // reset bitcnt
                    vector <string>().swap(strvec); // clear strvec
                }else if(bitcnt + b < max_bit && point != clusters[cluster].original_points.size()-1){
                    cout << "1" << endl;
                    bitcnt += b;
                    strvec.push_back(str);
                }else if(bitcnt + b < max_bit && point == clusters[cluster].original_points.size()-1){ // last element
                    cout << "3" << endl;
                    strvec.push_back(str);
                    for(int i=0; i<strvec.size(); i++){
                        reg_map[strvec[i]] = t.first + to_string(new_idx);
                    }
                    new_idx++;
                    
                }
            }

            for(int dim = 0; dim < clusters[cluster].original_points[point].size(); dim++){
                cout << clusters[cluster].original_points[point][dim] << " ";
                fout << clusters[cluster].original_points[point][dim] << " ";
            }

            cout << "-> ";
            fout << "-> ";

            for(int dim = 0; dim < clusters[cluster].shifted_points[point].size(); dim++){
                cout << clusters[cluster].shifted_points[point][dim] << " ";
                fout << clusters[cluster].shifted_points[point][dim] << " ";
            }
            cout << "\n";
            fout << "\n";
        }
        cout << "\n";
    }

    for(int cluster = 0; cluster < clusters.size(); cluster++){
        for(int point = 0; point < clusters[cluster].original_points.size(); point++){
            string str = points_namebits_map[make_pair(clusters[cluster].original_points[point][0], clusters[cluster].original_points[point][1])].first;
            cout << str << " map " << reg_map[str] << endl;
        }
    }

    // cout << "max bit is " << max_bit << endl;

    fout.close();

    return 0;

}
