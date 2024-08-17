#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <numeric>
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <cmath>
#include "MeanShift.h"

using namespace std;

#define _CJDBG              1   // separate instances into ffInstances and gateInstances
#define _DBG_PlacementMAP   0

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
    // clk belongs to which net
    // when reading net, if reading ffname/clk, then clk_map[ffname] = netname ( or a int )
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

// FlipFlop Lib parameters: FlipFlop <bits> <flipFlopName> <flipFlopWidth> <flipFlopHeight> <pinCount>
struct stFFValue {
  int    bits;      // no use
  double ffWidth;
  double ffHeight;
  int    pinCount;  // no use
  int    sitesC;    // convert ffWidth to site width count
  int    sitesR;    // convert ffHeight to site hieght count
};

// Gate values, Gate <gateName> <gateWidth> <gateHeight> <pinCount>
struct stGateValue {
  int gateWidth;
  int gateHeight;
  int pinCount;
};

//------------------------------
// Global Variable Declaration
//------------------------------
/* maps in the code
    map<string, int> type_bits_map, ex: type_bits_map['FF1'] = 1
    map<pair<double, double>, pair<string, string>> points_namebits_map, ex: points_namebits_map[(x, y)] = ('C1', 'FF1')
    map<string, string> name_type_map, ex: name_type_map['C1'] = 'FF1'
    map<string, string> reg_map, ex: reg_map['C1'] = 'C5'
    map<string, int> clk_net_map, ex: clk_net_map['C1'] = 5 (which means that clk of C1 is in 5th net)
    unordered_map<int, pair<double, string>> bits_minareaff_map
*/
map<string, int>    type_bits_map;
map<string, string> name_type_map;
map<string, int> clk_net_map;
unordered_map<int, pair<double, string>> bits_minareaff_map;

// Data structures to store parsed information
unordered_map<string, double> weights;
vector<Pin>         inputPins;
vector<Pin>         outputPins;
vector<FlipFlop>    flipFlops;
vector<Gate>        gates;

// Data structures for placement
map<string, stFFValue>   kmFFLib;   //  ffname: bits, ffWidth, ffHeight, pinCount, sitesC, sitesR
map<string, stGateValue> kmGateLib;

vector<Instance>    ffInstances;
vector<Instance>    gateInstances;
string              ffTag = "";


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

pair<string, int>   instName;
set<int>            possible_bits;
int max_bit =       0;    // FlipFlops' max bit count

int cost_area = 0;
double cost_power = 0;
double cost_tns = 0;

struct stPlacementMap { // Every placementMap is always rectangular
    // PlacementRows <startX> <startY> <siteWidth> <siteHeight> <totalNumOfSites>
    //uint8_t* byteMap;      // byteMap array for PlacementRows, 1-byte for a cell-site
    std::vector<std::vector<uint8_t>> byteMap;

    int bmRows;            // totalNumOfRows
    int bmCols;            // totalNumOfSites
    int startX, startY;    // lower-left (x,y) of PlacementMap
    int siteWidth, siteHeight; //assume siteWidth/siteHeight are the same for all sites
    int endX, endY;        // upper-right (x,y) of PlacementMap
};

vector<stPlacementMap>    placementMaps;

//------------------------------
// Function Declaration
//------------------------------
// PlacementMap functions
void pmCreatePlacementMap(void);
void pmShowPlacementMap(void);
void pmGateSetPlacementMap(Instance cell);
bool pmFFSearchPlacementMap(Instance cell, double x, double y);
bool pmIsEmpty(int r0, int c0, int sitesR, int sitesC);

int  SetNewInstPoint(Instance& new_instance);

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
    if(clk_net_map[points_namebits_map[make_pair(a[0], a[1])].first] != clk_net_map[points_namebits_map[make_pair(b[0], b[1])].first]){ // belong to different clk
        return clk_net_map[points_namebits_map[make_pair(a[0], a[1])].first] > clk_net_map[points_namebits_map[make_pair(b[0], b[1])].first];
    }
    return type_bits_map[points_namebits_map[make_pair(a[0], a[1])].second] > type_bits_map[points_namebits_map[make_pair(b[0], b[1])].second]; // sort points by ff's bits in decreasing order
}


// In the Placement(byte) Map, set the Gate Instance according to its x/y
void  pmGateSetPlacementMap(Instance gate){

    stGateValue gateValue = kmGateLib[gate.type_name];
    double gateEndX = gate.x + gateValue.gateWidth;     // the coordinate of Gate's upper-right corner
    double gateEndY = gate.y + gateValue.gateHeight;
    cout << "pmGate: " << gate.type_name << "@(" << gate.x << ", " << gate.y << "), " << gateValue.gateWidth << " x " << gateValue.gateHeight << endl;

    int idx;
    for(int idx=0; idx<placementMaps.size(); idx++){
        stPlacementMap &pm = placementMaps[idx];

        if ((gateEndX < pm.startX || pm.endX < gate.x)
         || (gateEndY < pm.startY || pm.endY < gate.y)) {
            // gate and placementMap are not overlapped
            continue;
        }

        // (c0,r0): indices of lower-left corner, (c1,r1): indices of upper-right corner
        // e.g: (x,y)=(10,10), (siteW,siteh)=(2,10), (gateW,gateH)=(5,10)
        int c0, r0, c1, r1;
        if (gate.x > pm.startX)
            c0 = floor((gate.x - pm.startX) / pm.siteWidth);
        else
            c0 = 0;

        if (gate.y > pm.startY)
            r0 = floor((gate.y - pm.startY) / pm.siteHeight);
        else
            r0 = 0;

        c1 = ceil((gateEndX - pm.startX) / pm.siteWidth) - 1;
        if (c1 >= pm.bmCols)
            c1 = pm.bmCols - 1;

        r1 = ceil((gateEndY - pm.startY) / pm.siteHeight) - 1;
        if (r1 >= pm.bmRows)
            r1 = pm.bmRows - 1;

#if _DBG_PlacementMAP
        cout << " set ByteMap: " << gate.type_name << "@(" << gate.x << ", " << gate.y << ") => (" << c0 << "," << r0 << ")" << endl;
#endif
        for (int r=r0; r<=r1; r++){
            for (int c=c0; c<=c1; c++){
                pm.byteMap[r][c] = 'G';
            }
        }

        return;

    }

}

bool pmIsEmpty(int idx, int c0, int r0, int sitesC, int sitesR){
    stPlacementMap &pm = placementMaps[idx];

    if ((c0+sitesC) > pm.bmCols)
        sitesC = pm.bmCols - c0;
    if ((r0+sitesR) > pm.bmRows)
        sitesR = pm.bmRows - r0;

    for (int r=r0; r< r0 + sitesR; r++){
        for (int c=c0; c < c0 + sitesC; c++){
            if (pm.byteMap[r][c]) { // not 0
#if _DBG_PlacementMAP
                cout << "!!NG (" << c0 << "," << r0 << ")" << endl;
#endif
                return false;
            }
        }
    }
#if _DBG_PlacementMAP
    cout << "Empty (" << c0 << "," << r0 << ")" << endl;
#endif
    return true;
}

// In Placement(byte) Map, Search the available spaces from (x,y) and set FF Instance
bool pmFFSearchPlacementMap(Instance& ffInstance){
    int idx = SetNewInstPoint(ffInstance);

    stPlacementMap &pm = placementMaps[idx];
    stFFValue ffValue = kmFFLib[ffInstance.type_name];

    // (c0,r0): indices of lower-left corner, (c1,r1): indices of upper-right corner
    // e.g: (x,y)=(10,10), (siteW,siteh)=(2,10), (gateW,gateH)=(5,10)
    uint8_t bits = (uint8_t)ffValue.bits + '0';
    int sitesC = ceil(ffValue.ffWidth / pm.siteWidth);
    int sitesR = ceil(ffValue.ffHeight / pm.siteHeight);
    int c0 = floor((ffInstance.x - pm.startX) / pm.siteWidth);       // initial search point (c0,r0)
    int r0 = floor((ffInstance.y - pm.startY) / pm.siteHeight);
    int ct = c0;        // target search point (ct,rt)
    int rt = r0;

    cout << "pmSrch: " << ffInstance.inst_name << " " << ffInstance.type_name << " (" << ffInstance.x << "," << ffInstance.y << ")" ;
    cout << " sitesC,sitesR=" << sitesC << "," << sitesR << endl;

    bool bEmpty = pmIsEmpty(idx, ct, rt, sitesC, sitesR);
    if (!bEmpty) {
        // Search in rectangular from lower-left (^,>,v,<)
        int iStart, iEnd;
        int dmax = max(max(c0, pm.bmCols-c0-1), max(r0, pm.bmRows-r0-1));
#if _DBG_PlacementMAP
        cout << "dmax=" << dmax << endl;
#endif
        for (int d=1; !bEmpty && d<=dmax ; d++) {
            //cout << "d=" << d << endl;  //cjdbg

            //s1: ^, from left-lower to (left-upper - 1)
            if ((c0 - d) >= 0) {
#if _DBG_PlacementMAP
                cout << "^" << endl;
#endif
                ct = c0 - d;
                (r0 > d) ? iStart = r0 - d : iStart = 0;
                ((r0 + d - 1) < pm.bmRows) ? iEnd = r0 + d - 1 : iEnd = pm.bmRows - 1;
                for (rt = iStart; rt <= iEnd; rt++) {
                    if (pmIsEmpty(idx, ct, rt, sitesC, sitesR)){
                        bEmpty = true;
                        break;
                    }
                }
            }

            //s2: >, from left-upper to (right-upper - 1)
            if (!bEmpty && ((r0 + d) < pm.bmRows)) {
#if _DBG_PlacementMAP
                cout << ">" << endl;
#endif
                rt = r0 + d;
                (c0 > d) ? iStart = c0 - d : iStart = 0;
                ((c0 + d - 1) < pm.bmCols) ? iEnd = c0 + d - 1 : iEnd = pm.bmCols - 1;
                for (ct = iStart; ct <= iEnd; ct++) {
                    if (pmIsEmpty(idx, ct, rt, sitesC, sitesR)){
                        bEmpty = true;
                        break;
                    }
                }
            }

            //s3: v, from right-upper to (right-lower - 1)
            if (!bEmpty && ((c0 + d) < pm.bmCols)) {
#if _DBG_PlacementMAP
                cout << "v" << endl;
#endif
                ct = c0 + d;
                ((r0 + d) < pm.bmRows) ? iStart = r0 + d : iStart = pm.bmRows - 1;
                (r0 > d - 1) ? iEnd = r0 - d + 1 : iEnd = 0;
                for (rt = iStart; rt >= iEnd; rt--) {
                    if (pmIsEmpty(idx, ct, rt, sitesC, sitesR)){
                        bEmpty = true;
                        break;
                    }
                }
            }

            //s4: <, from right-lower to (left-lower - 1)
            if (!bEmpty && ((r0 - d) >= 0)) {
#if _DBG_PlacementMAP
                cout << "<" << endl;
#endif
                rt = r0 - d;
                ((c0 + d) < pm.bmCols) ? iStart = c0 + d : iStart = pm.bmCols - 1;
                (c0 > d - 1) ? iEnd = c0 - d + 1 : iEnd = 0;
                for (ct = iStart; ct >= iEnd; ct--) {
                    if (pmIsEmpty(idx, ct, rt, sitesC, sitesR)){
                        bEmpty = true;
                        break;
                    }
                }
            }
        }
    }

    // Set Placement Map
    if (bEmpty) {
        // Instance lower-left index should be within PlacementMap
        assert((ct < pm.bmCols) && "ct should be less than bmCols");
        assert((rt < pm.bmRows) && "rt should be less than bmRows");

        cout << "-> " << ffInstance.type_name << " @(" << ct << "," << rt << ")" << endl;
        for (int r=rt; r< rt + sitesR; r++){
            for (int c=ct; c < ct + sitesC; c++){
                // except lower-left index, instance can be outside PlacementMap
                if ((c<pm.bmCols) && (r<pm.bmRows)) {
                    pm.byteMap[r][c] = bits;
                }
            }
        }
        return true;
    }
    else {
        cout << "!!NG Cannot Place FF" << endl;
        return false;
    }
}

void pmUpdatePlacementMap(int startR, int rowCnt) {
    stPlacementMap pm;  // placement Map
    // PlacementRows <startX> <startY> <siteWidth> <siteHeight> <totalNumOfSites>
    pm.bmCols = placementRows[startR].totalNumOfSites;
    pm.bmRows = rowCnt;
    pm.startX = placementRows[startR].startX;
    pm.startY = placementRows[startR].startY;
    pm.siteWidth = placementRows[startR].siteWidth;
    pm.siteHeight = placementRows[startR].siteHeight;
    pm.endX = pm.startX + pm.siteWidth*pm.bmCols;
    pm.endY = pm.startY + pm.siteHeight*pm.bmRows;

    cout << "placementMap: (" << pm.startX << "," << pm.startY << "), (" << pm.endX << "," << pm.endY << ") "
        << pm.siteWidth << ", " << pm.siteHeight << ", "
        << pm.bmCols << "cols x " << pm.bmRows << "rows" << endl;

    // Allocate memory for rows
    pm.byteMap.resize(pm.bmRows);

    // Allocate memory for each column within each row
    for (int i = 0; i < pm.bmRows; ++i) {
        pm.byteMap[i].resize(pm.bmCols, 0); // Initialize with 0 (unoccupied)
    }

    placementMaps.push_back(pm);
}

void pmCreatePlacementMap(void) {
    cout << "CreatePlaceMap" << endl;

    int totalRows = placementRows.size();
    int bmCols = placementRows[0].totalNumOfSites;
    int startR = 0, r;

    // check if all placementRows are in a group
    for (r=1; r < totalRows; r++) {
        if ((placementRows[r-1].startX != placementRows[r].startX)
          ||(placementRows[r-1].totalNumOfSites != placementRows[r].totalNumOfSites)
          ||((placementRows[r-1].startY + placementRows[r-1].siteHeight) != placementRows[r].startY))
        {
            pmUpdatePlacementMap(startR, r-startR);
            startR = r;
            cout << "Create another Placement Map..." << endl;
        }
    }
    pmUpdatePlacementMap(startR, r-startR);

    for (const auto& instance : gateInstances) {
        pmGateSetPlacementMap(instance);
    }
}

void pmShowPlacementMap(void)
{
    for (const auto& pm : placementMaps)
    {
        for (int r=pm.bmRows-1; r>=0; r--){
            cout << "r" << r << " ";
            for (int c=0; c<pm.bmCols; c++) {
                if (pm.byteMap[r][c]) // not 0
                    cout << pm.byteMap[r][c];
                else
                    cout << '.';
            }
            cout << endl;
        }
    }
}

int SetNewInstPoint(Instance& new_instance){
    vector<vector<double>> possible_points;
    for(auto pm : placementMaps){
        // cout << "original x, y = " << new_instance.x << " " << new_instance.y << endl;
        vector<double> possible_point;
        for(int i=0; i<pm.bmCols; i++){
            if(new_instance.x <= pm.startX){
                possible_point.push_back(pm.startX);
                break;
            }else if(new_instance.x >= pm.startX + pm.siteWidth*(pm.bmCols-1)){
                possible_point.push_back(pm.startX + pm.siteWidth*(pm.bmCols-1));
                break;
            }else if(new_instance.x >= pm.startX + pm.siteWidth*(i) && new_instance.x < pm.startX + pm.siteWidth*(i+1)){
                double a = new_instance.x - (pm.startX + pm.siteWidth*(i));
                double b = (pm.startX + pm.siteWidth*(i+1)) - new_instance.x;
                if(a > b){
                    possible_point.push_back(pm.startX + pm.siteWidth*(i+1));
                }else{
                    possible_point.push_back(pm.startX + pm.siteWidth*(i));
                }
                break;
            }
        }
        for(int j=0; j<pm.bmCols; j++){
            if(new_instance.y <= pm.startY){
                possible_point.push_back(pm.startY);
                break;
            }else if(new_instance.y >= pm.startY + pm.siteHeight*(pm.bmRows-1)){
                possible_point.push_back(pm.startY + pm.siteHeight*(pm.bmRows-1));
                break;
            }else if(new_instance.y >= pm.startY + pm.siteHeight*(j) && new_instance.y < pm.startY + pm.siteHeight*(j+1)){
                double a = new_instance.y - (pm.startY + pm.siteHeight*(j));
                double b = (pm.startY + pm.siteHeight*(j+1)) - new_instance.y;
                // cout << "a = " << a << " b = " << b << endl;
                if(a > b){
                    possible_point.push_back(pm.startY + pm.siteHeight*(j+1));
                }else{
                    possible_point.push_back(pm.startY + pm.siteHeight*(j));
                }
                break;
            }
        }
        // cout << "after moving x, y = " << new_instances[k].x << " " << new_instances[k].y << endl;
        possible_points.push_back(possible_point);
    }

    // compute the minimum distance between new_instance and every placementMap
    auto min_point = min_element(possible_points.begin(), possible_points.end(), [&new_instance](const vector<double>& a, const vector<double>& b) {
        return (pow(a[0] - new_instance.x, 2) + pow(a[1] - new_instance.y, 2)) < (pow(b[0] - new_instance.x, 2) + pow(b[1] - new_instance.y, 2));
    });

    // replace new_instance.x and new_instance.y with its closest point on the corresponding placementMaps
    new_instance.x = (*min_point)[0];
    new_instance.y = (*min_point)[1];

    // return the index of the corresponding placementMaps
    int idx = distance(possible_points.begin(), min_point);
    return idx;
}

// 3.1 Format of Input Data
void InputParsing(char *fname)
{
    string line;
    int clk_group_idx = 0;

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
        }
        else if (key == "FlipFlop") { // FlipFlop lib, FlipFlop <bits> <flipFlopName> <flipFlopWidth> <flipFlopHeight> <pinCount>
            // e.g: FlipFlop 1 SVT_FF_1 741 480 3

            FlipFlop flipFlop;
            iss >> flipFlop.bits >> flipFlop.name >> flipFlop.width >> flipFlop.height >> flipFlop.pinCount;

            // memorize the flipflop with the minimum area
            double area = flipFlop.width*flipFlop.height;
            if(bits_minareaff_map.find(flipFlop.bits) == bits_minareaff_map.end() || area < bits_minareaff_map[flipFlop.bits].first){
                bits_minareaff_map[flipFlop.bits].first = area;
                bits_minareaff_map[flipFlop.bits].second = flipFlop.name;
            }

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

            stFFValue values;
            values.bits = flipFlop.bits;
            values.ffWidth = flipFlop.width;
            values.ffHeight = flipFlop.height;
            values.pinCount = flipFlop.pinCount;
            kmFFLib[flipFlop.name] = values;

            if (ffTag == "") {
                pair<string, int>   ffName;
                ffName = splitString(flipFlop.name); // t.first is string
                ffTag = ffName.first;
                cout << "ffTag: " << ffTag << endl;
            }
        }

        else if (key == "Gate") { // Gate lib, Gate <gateName> <gateWidth> <gateHeight> <pinCount>
            // e.g: Gate G1 4590 4200 5
            Gate gate;
            iss >> gate.name >> gate.width >> gate.height >> gate.pinCount;
            cout << key << " " << gate.name << " " << gate.width << " " << gate.height << endl;
            for (int i = 0; i < gate.pinCount; ++i) {
                getline(file, line);
                istringstream pinIss(line);
                Pin pin;
                string type;
                pinIss >> type >> pin.name >> pin.x >> pin.y;
                gate.pins.push_back(pin);
            }
            gates.push_back(gate);

            stGateValue values;
            values.gateWidth = gate.width;
            values.gateHeight = gate.height;
            values.pinCount = gate.pinCount;
            kmGateLib[gate.name] = values;
        }

        else if (key == "NumInstances") { // NumInstances <instanceCount>
            int instanceCount;
            iss >> instanceCount;


            for (int i=0; i<instanceCount; i++) { // Inst <instName> <libCellName> <x> <y>
                getline(file, line);      //e.g: Inst C1 FF1 20.0 0.0
                                          //     Inst reg1 SVT_FF_1 5952 3600
                istringstream iss(line);

                Instance instance;
                string tag;
                iss >> tag >> instance.inst_name >> instance.type_name >> instance.x >> instance.y;

                if (tag != "Inst") {
                    cout << "wrong tag: " << tag << endl;
                    exit(1);
                }

                pair<string, int>   typeName;
                typeName = splitString(instance.type_name);

                if (typeName.first == ffTag) { // ffTag = 'FF'
                    ffInstances.push_back(instance);
                    instName = splitString(instance.inst_name); // instName.first = 'C'
                    name_type_map[instance.inst_name] = instance.type_name;
                }
                else {
                    gateInstances.push_back(instance);
                }
            }
        }

        else if (key == "Net") {
            Net net;
            iss >> net.name >> net.numPins; // ex: Net net36760 5
            for (int i = 0; i < net.numPins; ++i) {
                getline(file, line);
                istringstream pinIss(line);
                string pin;
                string connect;
                pinIss >> pin >> connect;
                if(connect.size() >= 4 && connect.compare(connect.size() - 4, 4, "/CLK") == 0){
                    // get the pin name extracting "/CLK", which is the ffname
                    string ffname = connect.substr(0, connect.find('/'));
                    // map the ffname to its clk group
                    clk_net_map[ffname] = clk_group_idx;
                }
                net.pins.push_back(connect);
            }
            clk_group_idx++;
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

    // for(map<string, int>::iterator it = clk_net_map.begin(); it != clk_net_map.end(); it++){
    //     cout << "id: " << (*it).first << ", name: " << (*it).second << "\n";
    // }

    for (const auto& entry : bits_minareaff_map) {
        std::cout << "Key: " << entry.first << ", Value: (" << entry.second.first << ", " << entry.second.second << ")" << std::endl;
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
#endif
    cout << "\nFlipflop:" << endl;
    for (const auto& flipflop : flipFlops) {
        cout << "bits: " << flipflop.bits << ", name: " << flipflop.name << ", width: " << flipflop.width << ", height: " << flipflop.height << endl;
#if 0
        cout << "Pins:" << endl;
        for (const auto& pin : flipflop.pins) {
            cout << "Name: " << pin.name << ", X: " << pin.x << ", Y: " << pin.y << endl;
        }
#endif
    }

    cout << "\nGate:" << endl;
    for (const auto& gate : gates) {
        cout << ", name: " << gate.name << ", width: " << gate.width << ", height: " << gate.height << endl;

#if 0
        cout << "Pins:" << endl;
        for (const auto& pin : gate.pins) {
            cout << "Name: " << pin.name << ", X: " << pin.x << ", Y: " << pin.y << endl;
        }
#endif
    }

    cout << "\nffInstances:" << endl;
    for (const auto& instance : ffInstances) {
        cout << "Name: " << instance.inst_name << ", FlipFlop Name: " << instance.type_name << ", X: " << instance.x << ", Y: " << instance.y << endl;
    }

    cout << "\ngateInstances:" << endl;
    for (const auto& instance : gateInstances) {
        cout << "Name: " << instance.inst_name << ", FlipFlop Name: " << instance.type_name << ", X: " << instance.x << ", Y: " << instance.y << endl;
    }

#if 0
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
    for(const auto& instance : ffInstances){ // need to rewrite this for-loop
        // Find the corresponding FlipFlop
        for(auto& flipFlop : flipFlops){
            if(instance.type_name == flipFlop.name){
                // Bottom-left corner of the cell push into points
                vector<double> point;
                point.push_back(instance.x);
                point.push_back(instance.y);
                points.push_back(point);

                points_namebits_map[make_pair(instance.x, instance.y)] = make_pair(instance.inst_name, instance.type_name);
            }
        }
    }

    // Cluster
    MeanShift *msp = new MeanShift();
    double kernel_bandwidth = 1000;

    vector<Cluster> clusters = msp->cluster(points, kernel_bandwidth); // generate clustering result
    // assert(0);

    ofstream file("clustering_result.txt");
    ofstream fout(argv[2]);
    vector<string> templines;

    if(!fout || !file){
        perror("Couldn't write output");
        exit(0);
    }


    cout << "\n====================\n";
    cout << "Found " << clusters.size() <<" clusters\n";
    file << "Found " << clusters.size() <<" clusters\n";
    cout << "====================\n\n";

    cout << "flipflops' size : " << ffInstances.size() << "\n";

    map<string, string> reg_map;    //cjdbg,

    for(int cluster = 0; cluster < clusters.size(); cluster++){
        sort(clusters[cluster].original_points.begin(), clusters[cluster].original_points.end(),
            [&points_namebits_map](vector<double>& a, vector<double>& b){ // lambda function
                return CompareByBits(a, b, points_namebits_map);
            });
    }

    // for (const auto& cluster : clusters) {
    //     for (const auto& point : cluster.original_points) {
    //         cout << "(" << point[0] << ", " << point[1] << ") ";
    //     }
    //     cout << endl;
    // }

    int new_idx = ffInstances.size() + gateInstances.size() + 1;   // should include gateInstance too

    vector<Instance> new_instances;

    for(int cluster = 0; cluster < clusters.size(); cluster++){
        cout << "Cluster " << cluster << ":\n";
        file << "Cluster " << cluster << ":\n";

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

            string next_str;
            if(point != clusters[cluster].original_points.size()-1){ // last point doesn't have next_str
                next_str = points_namebits_map[make_pair(clusters[cluster].original_points[point+1][0], clusters[cluster].original_points[point+1][1])].first;
            }

            if(b == max_bit){ // it's already a max-bit flipflop
                reg_map[str] = instName.first + to_string(new_idx);
                new_idx++;

                // output mapping list
                for(auto& flipFlop : flipFlops){
                    if(name_type_map[str] == flipFlop.name){
                        for(auto& pin : flipFlop.pins){
                            templines.push_back(str + "/" + pin.name + " map " +  reg_map[str] + "/" + pin.name);
                        }
                    }
                }

                // Banking : use the max-bit flipflop with the minimum area
                Instance new_instance;
                new_instance.inst_name = reg_map[str];
                // Still need to decide which FF is the best for doing banking
                for(auto& instance : ffInstances){
                    if(str == instance.inst_name){
                        new_instance.type_name = instance.type_name;
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

                    // output mapping list
                    int qpin_count = 0;
                    for(int i=0; i<strvec.size(); i++){
                        reg_map[strvec[i]] = instName.first + to_string(new_idx);
                        if(type_bits_map[name_type_map[strvec[i]]] == 1){
                            templines.push_back(strvec[i] + "/D map " + reg_map[strvec[i]] + "/D" + to_string(qpin_count));
                            templines.push_back(strvec[i] + "/Q map " + reg_map[strvec[i]] + "/Q" + to_string(qpin_count));
                            qpin_count++;
                            templines.push_back(strvec[i] + "/CLK map " +  reg_map[strvec[i]] + "/CLK");
                        }else{
                            for(int j=0; j<type_bits_map[name_type_map[strvec[i]]]; j++){
                                templines.push_back(strvec[i] + "/D" + to_string(j) + " map " +  reg_map[strvec[i]] + "/D" + to_string(qpin_count));
                                templines.push_back(strvec[i] + "/Q" + to_string(j) + " map " +  reg_map[strvec[i]] + "/Q" + to_string(qpin_count));
                                qpin_count++;
                            }
                            templines.push_back(strvec[i] + "/CLK map " +  reg_map[strvec[i]] + "/CLK");
                        }
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

                    // // Still need to determine which FF is the best for doing banking
                    // for(auto& flipFlop : flipFlops){
                    //     if(flipFlop.bits == max_bit){
                    //         new_instance.type_name = flipFlop.name;
                    //     }
                    // }
                    new_instance.type_name = bits_minareaff_map[max_bit].second;
                    new_instance.x = x_avg;
                    new_instance.y = y_avg;

                    // push new_instance into the vector new_instances
                    new_instances.push_back(new_instance);

                    bitcnt = 0; // reset bitcnt
                    vector <string>().swap(strvec); // clear strvec
                    vector <double>().swap(x_vec); // clear x_vec
                    vector <double>().swap(y_vec); // clear y_vec

                }else if((bitcnt + b < max_bit) && (point != clusters[cluster].original_points.size()-1) && (clk_net_map[str] == clk_net_map[next_str])){
                    // cout << "1" << endl;
                    bitcnt += b;
                    strvec.push_back(str);
                    x_vec.push_back(clusters[cluster].original_points[point][0]);
                    y_vec.push_back(clusters[cluster].original_points[point][1]);

                }else if((bitcnt + b < max_bit) && ((point == clusters[cluster].original_points.size()-1) || (clk_net_map[str] != clk_net_map[next_str])) ){ // deal with the remaining elements
                    // cout << "3" << endl;
                    bitcnt += b; // ex: 7 = 2+2+1+1+1, max_bit = 8
                    strvec.push_back(str);
                    x_vec.push_back(clusters[cluster].original_points[point][0]);
                    y_vec.push_back(clusters[cluster].original_points[point][1]);

                    vector<double> x_vec_last;
                    vector<double> y_vec_last;

                    int tmpcnt = 0;
                    int qpin_count = 0;
                    for(int i=0; i<strvec.size(); i++){
                        for(int pb : possible_bits_vec){
                            if(bitcnt >= pb){
                                tmpcnt += type_bits_map[name_type_map[strvec[i]]];

                                if(tmpcnt < pb){
                                    reg_map[strvec[i]] = instName.first + to_string(new_idx);

                                    // output mapping list
                                    if(type_bits_map[name_type_map[strvec[i]]] == 1){
                                        templines.push_back(strvec[i] + "/D map " + reg_map[strvec[i]] + "/D" + to_string(qpin_count));
                                        templines.push_back(strvec[i] + "/Q map " + reg_map[strvec[i]] + "/Q" + to_string(qpin_count));
                                        qpin_count++;
                                        templines.push_back(strvec[i] + "/CLK map " + reg_map[strvec[i]] + "/CLK");
                                    }else{
                                        for(int j=0; j<type_bits_map[name_type_map[strvec[i]]]; j++){
                                            templines.push_back(strvec[i] + "/D" + to_string(j) + " map " + reg_map[strvec[i]] + "/D" + to_string(qpin_count));
                                            templines.push_back(strvec[i] + "/Q" + to_string(j) + " map " + reg_map[strvec[i]] + "/Q" + to_string(qpin_count));
                                            qpin_count++;
                                        }
                                        templines.push_back(strvec[i] + "/CLK map " +  reg_map[strvec[i]] + "/CLK");
                                    }

                                    x_vec_last.push_back(x_vec[i]);
                                    y_vec_last.push_back(y_vec[i]);
                                    // for(auto x : x_vec_last){
                                    //     cout << x << endl;
                                    // }
                                    break;
                                }else if(tmpcnt == pb){
                                    reg_map[strvec[i]] = instName.first + to_string(new_idx);

                                    // output mapping list
                                    if(type_bits_map[name_type_map[strvec[i]]] == 1 && pb == 1){
                                        templines.push_back(strvec[i] + "/D map " + reg_map[strvec[i]] + "/D");
                                        templines.push_back(strvec[i] + "/Q map " + reg_map[strvec[i]] + "/Q");
                                        templines.push_back(strvec[i] + "/CLK map " + reg_map[strvec[i]] + "/CLK");
                                    }else if(type_bits_map[name_type_map[strvec[i]]] == 1){
                                        templines.push_back(strvec[i] + "/D map " + reg_map[strvec[i]] + "/D" + to_string(qpin_count));
                                        templines.push_back(strvec[i] + "/Q map " + reg_map[strvec[i]] + "/Q" + to_string(qpin_count));
                                        qpin_count++;
                                        templines.push_back(strvec[i] + "/CLK map " + reg_map[strvec[i]] + "/CLK");
                                    }else{
                                        for(int j=0; j<type_bits_map[name_type_map[strvec[i]]]; j++){
                                            templines.push_back(strvec[i] + "/D" + to_string(j) + " map " + reg_map[strvec[i]] + "/D" + to_string(qpin_count));
                                            templines.push_back(strvec[i] + "/Q" + to_string(j) + " map " + reg_map[strvec[i]] + "/Q" + to_string(qpin_count));
                                            qpin_count++;
                                        }
                                        templines.push_back(strvec[i] + "/CLK map " + reg_map[strvec[i]] + "/CLK");
                                    }
                                    qpin_count = 0;

                                    x_vec_last.push_back(x_vec[i]);
                                    y_vec_last.push_back(y_vec[i]);
                                    double x_avg = std::accumulate(x_vec_last.begin(), x_vec_last.end(), 0.0) / x_vec_last.size();
                                    double y_avg = std::accumulate(y_vec_last.begin(), y_vec_last.end(), 0.0) / y_vec_last.size();
                                    // for(auto x : x_vec_last){
                                    //     cout << x << endl;
                                    // }
                                    // cout << x_vec[i] << endl;

                                    // build new_instance
                                    Instance new_instance;
                                    new_instance.inst_name = reg_map[strvec[i]];

                                    // // Still need to determine which FF is the best for doing banking
                                    // for(auto& flipFlop : flipFlops){
                                    //     if(flipFlop.bits == pb){
                                    //         new_instance.type_name = flipFlop.name;
                                    //     }
                                    // }
                                    new_instance.type_name = bits_minareaff_map[pb].second;
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
                    bitcnt = 0; // reset bitcnt
                    vector <string>().swap(strvec); // clear strvec
                    vector <double>().swap(x_vec); // clear x_vec
                    vector <double>().swap(y_vec); // clear y_vec

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
            cout << str << " " << b << "bits " << clusters[cluster].original_points[point][0] << " " << clusters[cluster].original_points[point][1]
                << " -> " << reg_map[str] << endl;
            file << str << " " << b << "bits " << clusters[cluster].original_points[point][0] << " " << clusters[cluster].original_points[point][1]
                << " -> " << reg_map[str] << endl;

            // for(int dim = 0; dim < clusters[cluster].shifted_points[point].size(); dim++){
            //     cout << clusters[cluster].shifted_points[point][dim] << " ";
            //     file << clusters[cluster].shifted_points[point][dim] << " ";
            // }
        }
        cout << endl;
    }

    for(int cluster = 0; cluster < clusters.size(); cluster++){
        for(int point = 0; point < clusters[cluster].original_points.size(); point++){
            string str = points_namebits_map[make_pair(clusters[cluster].original_points[point][0], clusters[cluster].original_points[point][1])].first;
            cout << str << " map " << reg_map[str] << endl;
        }
    }

    for(const auto& new_instance : new_instances){
        cout << "before moving : " << new_instance.inst_name << " " << new_instance.type_name << " " << new_instance.x << " " << new_instance.y << endl;
    }

    // Move (x, y) in new_instance to its closest point(the lower left corner of a placement site) of PlacementRows
    // for(int k=0; k<new_instances.size(); k++){
    //     // cout << "original x, y = " << new_instances[k].x << " " << new_instances[k].y << endl;
    //     for(int i=0; i<placementRows[0].totalNumOfSites; i++){
    //         if(new_instances[k].x <= placementRows[0].startX){
    //             new_instances[k].x = placementRows[0].startX;
    //             break;
    //         }else if(new_instances[k].x >= placementRows[0].startX + placementRows[0].siteWidth*(placementRows[0].totalNumOfSites-1)){
    //             new_instances[k].x = placementRows[0].startX + placementRows[0].siteWidth*(placementRows[0].totalNumOfSites-1);
    //             break;
    //         }else if(new_instances[k].x >= placementRows[0].startX + placementRows[0].siteWidth*(i) && new_instances[k].x < placementRows[0].startX + placementRows[0].siteWidth*(i+1)){
    //             double a = new_instances[k].x - (placementRows[0].startX + placementRows[0].siteWidth*(i));
    //             double b = (placementRows[0].startX + placementRows[0].siteWidth*(i+1)) - new_instances[k].x;
    //             if(a > b){
    //                 new_instances[k].x = placementRows[0].startX + placementRows[0].siteWidth*(i+1);
    //             }else{
    //                 new_instances[k].x = placementRows[0].startX + placementRows[0].siteWidth*(i);
    //             }
    //             break;
    //         }
    //     }

    //     for(int j=0; j<placementRows[0].totalNumOfSites; j++){
    //         if(new_instances[k].y <= placementRows[0].startY){
    //             new_instances[k].y = placementRows[0].startY;
    //             break;
    //         }else if(new_instances[k].y >= placementRows[0].startY + placementRows[0].siteHeight*(placementRows.size()-1)){
    //             new_instances[k].y = placementRows[0].startY + placementRows[0].siteHeight*(placementRows.size()-1);
    //             break;
    //         }else if(new_instances[k].y >= placementRows[0].startY + placementRows[0].siteHeight*(j) && new_instances[k].y < placementRows[0].startY + placementRows[0].siteHeight*(j+1)){
    //             double a = new_instances[k].y - (placementRows[0].startY + placementRows[0].siteHeight*(j));
    //             double b = (placementRows[0].startY + placementRows[0].siteHeight*(j+1)) - new_instances[k].y;
    //             // cout << "a = " << a << " b = " << b << endl;
    //             if(a > b){
    //                 new_instances[k].y = placementRows[0].startY + placementRows[0].siteHeight*(j+1);
    //                 cout << new_instances[k].y << endl;
    //             }else{
    //                 new_instances[k].y = placementRows[0].startY + placementRows[0].siteHeight*(j);
    //             }
    //             break;
    //         }
    //     }
    //     // cout << "after moving x, y = " << new_instances[k].x << " " << new_instances[k].y << endl;
    // }

    // cout << "new_instances.size() = " << new_instances.size() << endl;

    file.close();
    // fout.close();

    // assert(0);

    pmCreatePlacementMap();
    //for (const auto& instance : ffInstances) {
    for (auto& instance : new_instances) {
        pmFFSearchPlacementMap(instance);
    }
    pmShowPlacementMap();


    fout << "CellInst " << new_instances.size() << endl;
    for(const auto& new_instance : new_instances){
        fout << "Inst " << new_instance.inst_name << " " << new_instance.type_name << " " << new_instance.x << " " << new_instance.y << endl;
    }

    for (const auto& templine : templines) {
        fout << templine << endl;
    }

    fout.close();

    return 0;

}
