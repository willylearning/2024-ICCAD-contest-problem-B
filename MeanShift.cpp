#include <stdio.h>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <float.h>
#include "MeanShift.h"

using namespace std;

#define CLUSTER_EPSILON 0.5

double euclidean_distance(const vector<double> &point_a, const vector<double> &point_b){
    double total = 0;
    for(int i=0; i<point_a.size(); i++){
        const double temp = (point_a[i] - point_b[i]);
        total += temp*temp;
    }
    return sqrt(total);
}

double euclidean_distance_sqr(const vector<double> &point_a, const vector<double> &point_b){
    double total = 0;
    for(int i=0; i<point_a.size(); i++){
        const double temp = (point_a[i] - point_b[i]);
        total += temp*temp;
    }
    return (total);
}

double gaussian_kernel(double distance, double kernel_bandwidth){
    double temp =  exp(-1.0/2.0 * (distance*distance) / (kernel_bandwidth*kernel_bandwidth)); // gaussian function
    return temp;
}

void MeanShift::set_kernel( double (*_kernel_func)(double,double)){
    if(!_kernel_func){
        kernel_func = gaussian_kernel;
    }else{
        kernel_func = _kernel_func;    
    }
}

void MeanShift::shift_point(const Point &point, const std::vector<Point> &points, double kernel_bandwidth, Point &shifted_point) {
    double hmax = 1500; // need to be tested how big it should be
    // point : current center point, points : all data points
    vector<double> var_h = variable_bandwidth(points, kernel_bandwidth);
    shifted_point.resize(point.size());
    for(int dim = 0; dim<shifted_point.size(); dim++){
        shifted_point[dim] = 0;
    }
    double total_weight = 0;
    for(int i=0; i<points.size(); i++){
        const Point& temp_point = points[i];
        double distance = euclidean_distance(point, temp_point);
        double weight;
        // 核函数（如高斯核）計算了每個點的權重，这些權重用於計算當前點的移動方向和距離。這個過程實際上就是在計算梯度
        // knn(Identifying Effective Neighbors)
        if(distance <= hmax){
            weight = kernel_func(distance, var_h[i]); // change to variable bandwidth
        } else{
            weight = 0;
        }
        for(int j=0; j<shifted_point.size(); j++){
            shifted_point[j] += temp_point[j] * weight;
        }
        total_weight += weight;
    }

    const double total_weight_inv = 1.0/total_weight;
    // knn function
    for(int i=0; i<shifted_point.size(); i++){ // add KNN identify effective neighbors
        shifted_point[i] *= total_weight_inv; // 計算加權平均值
    }
}

std::vector<MeanShift::Point> MeanShift::meanshift(const std::vector<Point> &points, double kernel_bandwidth, double EPSILON){
    const double EPSILON_SQR = EPSILON*EPSILON; // determine when to converge
    vector<bool> stop_moving(points.size(), false);
    vector<Point> shifted_points = points;
    double max_shift_distance;
    Point point_new;
    do {
        max_shift_distance = 0;
        for(int i=0; i<points.size(); i++){
            if (!stop_moving[i]) {
                shift_point(shifted_points[i], points, kernel_bandwidth, point_new);
                double shift_distance_sqr = euclidean_distance_sqr(point_new, shifted_points[i]);
                if(shift_distance_sqr > max_shift_distance){
                    max_shift_distance = shift_distance_sqr;
                }
                if(shift_distance_sqr <= EPSILON_SQR) {
                    stop_moving[i] = true;
                }
                shifted_points[i] = point_new;
            }
        }
        printf("max_shift_distance: %f\n", sqrt(max_shift_distance));
    } while (max_shift_distance > EPSILON_SQR);
    return shifted_points;
}

vector<Cluster> MeanShift::cluster(const std::vector<Point> &points, const std::vector<Point> &shifted_points){
    vector<Cluster> clusters;

    for (int i = 0; i < shifted_points.size(); i++) {

        int c = 0;
        for (; c < clusters.size(); c++) {
            if (euclidean_distance(shifted_points[i], clusters[c].mode) <= CLUSTER_EPSILON) {
                break;
            }
        }

        if (c == clusters.size()) {
            Cluster clus;
            clus.mode = shifted_points[i];
            clusters.push_back(clus);
        }

        clusters[c].original_points.push_back(points[i]);
        clusters[c].shifted_points.push_back(shifted_points[i]);
    }

    return clusters;
}

vector<Cluster> MeanShift::cluster(const std::vector<Point> &points, double kernel_bandwidth){
    vector<Point> shifted_points = meanshift(points, kernel_bandwidth);
    return cluster(points, shifted_points);
}

vector<double> MeanShift::variable_bandwidth(const std::vector<Point> &points, double kernel_bandwidth){ // Point is vector<double>
    vector<Point> a(points.size());
    vector<double> var_h;
    double hmax = 1500; // need to be tested how big it should be
    int alpha = 5; // need to be determined
    int M = 2;
    double m = DBL_MAX;
    // find the M-th nearest neighbor of every point in points
    for(int i=0; i<points.size(); i++){
        for(int j=0; j<points.size(); j++){
            if(points[i] != points[j]){
                // the Euclidean distance between register i and its M-th nearest neighbor
                a[i].push_back(euclidean_distance(points[i], points[j])); 
            }
        }
        sort(a[i].begin(), a[i].end());
    }

    // for(int i=0; i< a.size(); i++){
    //     for(int j=0; j< a[i].size(); j++){
    //         cout << a[i][j] << "\n";
    //     }
    //     cout << "stop" << "\n";
    //     sort(a[i].begin(), a[i].end());
    // }

    double tmp = 0;
    for(int i=0; i<points.size(); i++){ 
        // knn(Identifying Effective Neighbors)
        // if (a[i][M-1] <= hmax){
        tmp = alpha*a[i][M-1]; // M-th nearest points
        var_h.push_back(min(hmax, tmp));
        // }
    }
    return var_h;
}