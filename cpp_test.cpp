#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "MeanShift.h"

using namespace std;

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

void print_points(vector<vector<double>> points){
    for(int i=0; i<points.size(); i++){
        for(int dim = 0; dim<points[i].size(); dim++) {
            printf("%f ", points[i][dim]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    MeanShift *msp = new MeanShift();
    double kernel_bandwidth = 1500;

    vector<string> reg_name;
    vector<vector<double>> points = load_points("testcase1.csv", reg_name);

    vector<Cluster> clusters = msp->cluster(points, kernel_bandwidth); // generate clustering result

    FILE *fp = fopen("result.csv", "w");
    if(!fp){
        perror("Couldn't write result.csv");
        exit(0);
    }

    printf("\n====================\n");
    printf("Found %lu clusters\n", clusters.size());
    printf("====================\n\n");

    int reg_cnt = 0;
    cout << "reg_name size : " << reg_name.size() << "\n";

    for(int cluster = 0; cluster < clusters.size(); cluster++) {
      DBGPRN("Cluster %i:\n", cluster);
      fprintf(fp, "Cluster %i:\n", cluster); // need to write clusters into result
      for(int point = 0; point < clusters[cluster].original_points.size(); point++){
        fprintf(fp, "%s ", reg_name[reg_cnt].c_str());
        cout << reg_name[reg_cnt] << " ";
        reg_cnt++;

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
        // fprintf("hello", "\n");
      }
      printf("\n");
    }
    fclose(fp);

    return 0;
}
