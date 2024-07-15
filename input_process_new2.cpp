#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <numeric>
#include <stdio.h>
#include <stdlib.h>
#include "MeanShift.h"
#include <unordered_map>

using namespace std;

//------------------------------
// Define and Struction
//------------------------------
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

//------------------------------
// Placement Function Declaration
//------------------------------

// 1. sort ff and instance
bool compareByBitsDescending(const FlipFlop &a, const FlipFlop &b) {
    return a.bits > b.bits;
}

bool compareInstancesByFlipFlopBits(const Instance &a, const Instance &b, const std::unordered_map<std::string, int>& flipFlopBitsMap) {
    return flipFlopBitsMap.at(a.type_name) > flipFlopBitsMap.at(b.type_name);
}

void sortInstance(vector<Instance>  &instances, vector<FlipFlop> &flipFlops) {
    sort(flipFlops.begin(), flipFlops.end(), compareByBitsDescending);
    unordered_map<std::string, int> flipFlopBitsMap;
    for (const auto &ff : flipFlops) {
        flipFlopBitsMap[ff.name] = ff.bits;
    }
    sort(instances.begin(), instances.end(),
        [&flipFlopBitsMap](const Instance &a, const Instance &b) {
            return compareInstancesByFlipFlopBits(a, b, flipFlopBitsMap);
        });
}

// 2.
struct Place {
    bool isPlace = 0;
    float placeLengthX;
    float placeLengthY;
    float bottomX;
    float bottomY;
};
vector<Place> place;
void createPlace(diesize &size, double &binWidth, double &binHeight, double &binMaxUtil, vector<Place> &place) {
    int columnCount = size.x_right/binWidth;
    int rowCount = size.y_up/binHeight;
    int placeNum = rowCount*columnCount;
    for (int i = 0; i < rowCount; ++i) {
        for (int j = 0; j < columnCount; ++j) {
            Place p;
            p.bottomX = size.x_left + binWidth*j;
            p.bottomY = size.y_bottom + binHeight*i;
            place.push_back(p);
        }
    }
}

void placementAlg(map<string, string> &name_type_map, map<string, string> &reg_map, vector<PlacementRow> &placementRows, vector<Instance>  &instances) {
    
}

//------------------------------
// Global Variable Declaration
//------------------------------
/* maps in the code
    map<string, int> type_bits_map, ex: type_bits_map['FF1'] = 1
    map<pair<double, double>, pair<string, string>> points_namebits_map, ex: points_namebits_map[(x, y)] = ('reg1', 'FF1')
    map<string, string> name_type_map, ex: name_type_map['reg1'] = 'FF1'
    map<string, string> reg_map, ex: reg_map['reg1'] = 'reg5'
*/
map<string, int>    type_bits_map;
map<string, string> name_type_map;

// Data structures to store parsed information
map<string, double> weights;
vector<Pin>         inputPins;
vector<Pin>         outputPins;
vector<FlipFlop>    flipFlops;
vector<Gate>        gates;
vector<Instance>    instances;
vector<Net>         nets;
double              binWidth = 0;
double              binHeight = 0;
double              binMaxUtil = 0;
vector<PlacementRow> placementRows;
vector<TimingSlack> timingSlacks;
vector<GatePower>   gatePowers;
diesize size;
vector<Qpindelay>   qpindelays; // Change to vector

// map< pair<double, double>, pair<string, string> > inst_map;

pair<string, int>   t;

set<int>            possible_bits;
int max_bit =       0;    // FlipFlops' max bit count

//------------------------------
// Functions
//------------------------------
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

bool CompareByBits(vector<double>& a, vector<double>& b, map<pair<double, double>, pair<string, string>> points_namebits_map) {
    return type_bits_map[points_namebits_map[make_pair(a[0], a[1])].second] > type_bits_map[points_namebits_map[make_pair(b[0], b[1])].second]; // sort points by ff's bits in decreasing order
}

// 3.1 Format of Input Data
void InputParsing(char *fname)
{
    string line;

    ifstream file(fname);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << fname << "'." << std::endl;
        exit(1); // Indicate error
    }

    while (getline(file, line)) {
        istringstream iss(line);
        string key;
        iss >> key;

        if (key == "Alpha" || key == "Beta" || key == "Gamma" || key == "Lambda" || key == "DisplacementDelay") {
            double value;
            iss >> value;
            weights[key] = value;
        } else if (key == "DieSize") {
            iss >> size.x_left >> size.y_bottom >> size.x_right >> size.y_up;
        } else if (key == "Input") {
            Pin pin;
            iss >> pin.name >> pin.x >> pin.y; // ex: INPUT1 1253995 0
            inputPins.push_back(pin);
        } else if (key == "Output") {
            Pin pin;
            iss >> pin.name >> pin.x >> pin.y; // ex: OUTPUT1 1296040 41105
            outputPins.push_back(pin);
        } else if (key == "FlipFlop") {
            FlipFlop flipFlop;
            iss >> flipFlop.bits >> flipFlop.name >> flipFlop.width >> flipFlop.height >> flipFlop.pinCount;
            // fill flipFlop.bits in the set "possible_bits" in increasing order
            possible_bits.insert(flipFlop.bits);
            // fill flipFlop.bits in type_bits_map, ex: type_bits_map['FF1'] = 1
            type_bits_map[flipFlop.name] = flipFlop.bits;
            // deal with each pin
            for(int i = 0; i < flipFlop.pinCount; ++i){
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
            iss >> instance.inst_name >> instance.type_name >> instance.x >> instance.y; // ex: Inst C1 G322 304470 661500
            t = splitString(instance.inst_name); // t.first is string, t.second is int
            // cout << t.first << endl;
            name_type_map[instance.inst_name] = instance.type_name;

            instances.push_back(instance);
        } else if (key == "Net") {
            Net net;
            iss >> net.name >> net.numPins; // ex: Net net36760 5
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
        } else if (key == "QpinDelay") {
            Qpindelay qpindelay;
            iss >> qpindelay.flipflopname >> qpindelay.value;
            qpindelays.push_back(qpindelay); // Add to vector
        } else if (key == "GatePower") {
            GatePower gatePower;
            iss >> gatePower.libCellName >> gatePower.powerConsumption;
            gatePowers.push_back(gatePower);
        } 
    }

    file.close();
}

void DisplayParsing(void) {
#if 0
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
#endif
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cerr << "Error: Please provide a filename as an argument." << std::endl;
        return 1; // Indicate error
    }

    InputParsing(argv[1]);
    DisplayParsing();

    vector<int> possible_bits_vec(possible_bits.begin(), possible_bits.end());
    reverse(possible_bits_vec.begin(), possible_bits_vec.end());
    max_bit = possible_bits_vec[0];

#if 0
    for (int num : possible_bits_vec) {
        cout << "possible_bits " << num << " ";
    }
#endif

    vector<vector<double>> points;
    map<pair<double, double>, pair<string, string>> points_namebits_map;

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

                n = type_bits_map[instance.type_name];
                points_namebits_map[make_pair(instance.x, instance.y)] = make_pair(instance.inst_name, instance.type_name);
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

    cout << "flipflops' size : " << instances.size() << "\n";

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

    int new_idx = instances.size() + 1;
    vector<Instance> new_instances;

    for(int cluster = 0; cluster < clusters.size(); cluster++){
        cout << "Cluster " << cluster << ":\n";
        fout << "Cluster " << cluster << ":\n";

        // banking process
        // construct a map between original_points[point] and its reg_type
        int bitcnt = 0;
        vector<string> strvec;
        vector<double> x_vec;
        vector<double> y_vec;
        

        for(int point = 0; point < clusters[cluster].original_points.size(); point++){
            // check which inst_name the point corresponds to in instances
            string str = points_namebits_map[make_pair(clusters[cluster].original_points[point][0], clusters[cluster].original_points[point][1])].first;
            int b = type_bits_map[points_namebits_map[make_pair(clusters[cluster].original_points[point][0], clusters[cluster].original_points[point][1])].second];

            if(b == max_bit){ // it's already a max-bit flipflop
                reg_map[str] = t.first + to_string(new_idx);
                new_idx++;
                // Banking : use the max-bit flipflop with the minimum area
                Instance new_instance;
                new_instance.inst_name = reg_map[str];
                // Still need to decide which FF is the best for doing banking
                for(auto& flipFlop : flipFlops){
                    if(flipFlop.bits == max_bit){
                        new_instance.type_name = flipFlop.name;
                    }
                }
                new_instance.x = clusters[cluster].original_points[point][0];
                new_instance.y = clusters[cluster].original_points[point][1];
                // push new_instance into the vector new_instances
                new_instances.push_back(new_instance);
            }else{
                if(bitcnt + b == max_bit){ // can bank into a max-bit flipflop
                    // cout << "2" << endl;
                    strvec.push_back(str);
                    for(int i=0; i<strvec.size(); i++){
                        reg_map[strvec[i]] = t.first + to_string(new_idx);
                    }
                    new_idx++;

                    // deal with the new flipflop's coordinate
                    x_vec.push_back(clusters[cluster].original_points[point][0]);
                    y_vec.push_back(clusters[cluster].original_points[point][1]);
                    double x_avg = std::accumulate(x_vec.begin(), x_vec.end(), 0.0) / x_vec.size();
                    double y_avg = std::accumulate(y_vec.begin(), y_vec.end(), 0.0) / y_vec.size();

                    // Banking : use the max-bit flipflop with the minimum area
                    Instance new_instance;
                    new_instance.inst_name = reg_map[str];
                    // Still need to decide which FF is the best for doing banking
                    for(auto& flipFlop : flipFlops){
                        if(flipFlop.bits == max_bit){
                            new_instance.type_name = flipFlop.name;
                        }
                    }
                    new_instance.x = x_avg;
                    new_instance.y = y_avg;
                    // push new_instance into the vector new_instances
                    new_instances.push_back(new_instance);

                    bitcnt = 0; // reset bitcnt
                    vector <string>().swap(strvec); // clear strvec
                    vector <double>().swap(x_vec); // clear x_vec
                    vector <double>().swap(y_vec); // clear y_vec

                }else if(bitcnt + b < max_bit && point != clusters[cluster].original_points.size()-1){
                    // cout << "1" << endl;
                    bitcnt += b;
                    strvec.push_back(str);
                    x_vec.push_back(clusters[cluster].original_points[point][0]);
                    y_vec.push_back(clusters[cluster].original_points[point][1]);

                }else if(bitcnt + b < max_bit && point == clusters[cluster].original_points.size()-1){ // remain elements
                    // cout << "3" << endl;
                    bitcnt += b; // ex: 7 = 2+2+1+1+1, max_bit = 8
                    strvec.push_back(str);
                    x_vec.push_back(clusters[cluster].original_points[point][0]);
                    y_vec.push_back(clusters[cluster].original_points[point][1]);

                    vector<double> x_vec_last;
                    vector<double> y_vec_last;

                    int tmpcnt = 0;
                    for(int i=0; i<strvec.size(); i++){
                        for(int pb : possible_bits_vec){
                            if(bitcnt >= pb){
                                tmpcnt += type_bits_map[name_type_map[strvec[i]]];
                                if(tmpcnt < pb){
                                    reg_map[strvec[i]] = t.first + to_string(new_idx);
                                    x_vec_last.push_back(x_vec[i]);
                                    y_vec_last.push_back(y_vec[i]);
                                    // for(auto x : x_vec_last){
                                    //     cout << x << endl;
                                    // }
                                    break;
                                }else if(tmpcnt == pb){
                                    reg_map[strvec[i]] = t.first + to_string(new_idx);
                                    x_vec_last.push_back(x_vec[i]);
                                    y_vec_last.push_back(y_vec[i]);
                                    double x_avg = std::accumulate(x_vec_last.begin(), x_vec_last.end(), 0.0) / x_vec_last.size();
                                    double y_avg = std::accumulate(y_vec_last.begin(), y_vec_last.end(), 0.0) / y_vec_last.size();
                                    // for(auto x : x_vec_last){
                                    //     cout << x << endl;
                                    // }
                                    // cout << x_vec[i] << endl;
                                    
                                    // new_instance forms
                                    Instance new_instance;
                                    new_instance.inst_name = reg_map[strvec[i]];
                                    // Still need to decide which FF is the best for doing banking
                                    for(auto& flipFlop : flipFlops){
                                        if(flipFlop.bits == pb){
                                            new_instance.type_name = flipFlop.name;
                                        }
                                    }
                                    new_instance.x = x_avg;
                                    new_instance.y = y_avg;
                                    // push new_instance into the vector new_instances
                                    new_instances.push_back(new_instance);
                                    // update new_idx, bitcnt, tmpcnt
                                    new_idx++;
                                    bitcnt -= pb;
                                    tmpcnt = 0;
                                    vector <double>().swap(x_vec_last); // clear x_vec
                                    vector <double>().swap(y_vec_last); // clear y_vec
                                    break;
                                }
                            }
                        }
                    }
                    // // Don't do banking
                    // strvec.push_back(str);
                    // for(int i=0; i<strvec.size(); i++){
                    //     reg_map[strvec[i]] = t.first + to_string(new_idx);
                    //     new_idx++;
                    //      // rename the flipflops that do not bank
                    //     Instance new_instance;
                    //     new_instance.inst_name = reg_map[strvec[i]];
                    //     for(auto& flipFlop : flipFlops){
                    //         if(flipFlop.bits == max_bit){
                    //             new_instance.type_name = flipFlop.name;
                    //         }
                    //     }
                    //     new_instances.push_back(new_instance);
                    // }

                }
            }
            cout << str << " " << b << "bits ";
            fout << str << " " << b << "bits ";

            cout << clusters[cluster].original_points[point][0] << " " << clusters[cluster].original_points[point][1] << " ";
            fout << clusters[cluster].original_points[point][0] << " " << clusters[cluster].original_points[point][1] << " ";

            cout << "-> ";
            fout << "-> ";

            cout << reg_map[str] << " ";
            fout << reg_map[str] << " ";

            // for(int dim = 0; dim < clusters[cluster].shifted_points[point].size(); dim++){
            //     cout << clusters[cluster].shifted_points[point][dim] << " ";
            //     fout << clusters[cluster].shifted_points[point][dim] << " ";
            // }
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

    for(auto new_instance : new_instances){
        cout << new_instance.inst_name << " " << new_instance.type_name << " " << new_instance.x << " " << new_instance.y << endl;
    }
    // cout << "max bit is " << max_bit << endl;

    fout.close();

    return 0;

}