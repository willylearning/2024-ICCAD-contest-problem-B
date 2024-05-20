#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace std;

struct diesize {
    int x_left;
    int y_bottom;
    int x_right;
    int y_up;
};

struct Pin {
    string name;
    int x;
    int y;
};

struct FlipFlop {
    int bits;
    string name;
    int width;
    int height;
    int pinCount;
    vector<Pin> pins;
};

struct Instance {
    string name;
    string flipFlopName;
    int x;
    int y;
};

struct Net {
    string name;
    int numPins;
    vector<string> pins;
};

struct PlacementRow {
    int startX;
    int startY;
    int siteWidth;
    int siteHeight;
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

int main() {
    ifstream file("sampleCase");
    string line;

    // Data structures to store parsed information
    map<string, double> weights;
    vector<Pin> inputPins;
    vector<Pin> outputPins;
    vector<FlipFlop> flipFlops;
    vector<Instance> instances;
    vector<Net> nets;
    int binWidth = 0;
    int binHeight = 0;
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
            iss >> instance.name >> instance.flipFlopName >> instance.x >> instance.y;
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


    return 0;

}
