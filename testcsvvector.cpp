#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <string>

using namespace std;

// Function to parse the CSV and create the vector structure
vector<vector<pair<double, int> > > parseCSV(const string& filename) {
    vector<vector<pair<double, int> > > data;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Could not open the file!" << endl;
        return data;
    }

    string line;
    while (getline(file, line)) {
        vector<pair<double, int> > row;
        stringstream ss(line);
        string value;
        bool isDouble = true;
        double doubleVal;
        int intVal;

        while (getline(ss, value, ',')) {
            if (isDouble) {
                doubleVal = stod(value);
            } else {
                intVal = stoi(value);
                row.push_back(make_pair(doubleVal, intVal));
            }
            isDouble = !isDouble;
        }

        data.push_back(row);
    }

    file.close();
    return data;
}

// Function to print the vector structure
void printData(const vector<vector<pair<double, int> > >& data) {
    for (const auto& row : data) {
        for (const auto& p : row) {
            cout << "(" << p.first << ", " << p.second << ") ";
        }
        cout << endl;
    }
}

int main() {
    string filename = "knn_results.csv";
    vector<vector<pair<double, int> > > data = parseCSV(filename);
    cout << data[0][2].first << data[0][2].second;
    return 0;
}
