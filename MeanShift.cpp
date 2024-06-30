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
bool cmp(pair<double, int> a, pair<double, int> b){
    return a.first < b.first;
}

void MeanShift::shift_point(const Point &point, const std::vector<Point> &points, double kernel_bandwidth, Point &shifted_point) {
    // point : current center point, points : all data points
    double hmax = 100000; // need to be tested how big it should be
    int K = 2; // need to be tested how big it should be
    vector<double> var_h = variable_bandwidth(points, kernel_bandwidth);
    shifted_point.resize(point.size());
    for(int dim = 0; dim<shifted_point.size(); dim++){
        shifted_point[dim] = 0;
    }
    double total_weight = 0;

    // Identifying Effective Neighbors by KNN
    vector<pair<double, int>> dist_from_point;
    for(int i = 0; i < points.size(); ++i){
        if(points[i] != point){
            double distance = euclidean_distance(points[i], point);
            dist_from_point.push_back(make_pair(distance, i));
            // cout << points[i][0] << endl;
        }  
    }

    sort(dist_from_point.begin(), dist_from_point.end(), cmp); // sort dist_from_point by distance
    // cout << points[0][0] << endl;

    vector<double> effective_var_h;
    vector<Point> effective_neighbors;
    for(int i=0; i<K; i++){ 
        // cout << "t1 " << dist_from_point[i].first << ", " << hmax << endl;
        if(dist_from_point[i].first <= hmax){ // if the neighbor's distance > hmax => excluded neighbor
            effective_neighbors.push_back(points[dist_from_point[i].second]);
            effective_var_h.push_back(var_h[dist_from_point[i].second]);
            // cout << "t2 " << effective_neighbors[i][0] << endl;
        }
    }

    for(int i=0; i<effective_neighbors.size(); i++){
        const Point& temp_point = effective_neighbors[i];
        double distance = euclidean_distance(point, temp_point);
        // 核函数（如高斯核）計算了每個點的權重，这些權重用於計算當前點的移動方向和距離。這個過程實際上就是在計算梯度
        double weight = kernel_func(distance, effective_var_h[i]); // change to variable bandwidth

        for(int j=0; j<shifted_point.size(); j++){
            shifted_point[j] += temp_point[j] * weight;
            // cout << shifted_point[j] << endl;
        }
        total_weight += weight;
    }
    
    // Original version
    // for(int i=0; i<points.size(); i++){
    //     const Point& temp_point = points[i];
    //     double distance = euclidean_distance(point, temp_point);
    //     // 核函数（如高斯核）計算了每個點的權重，这些權重用於計算當前點的移動方向和距離。这个過程實際上就是在計算梯度
    //     double weight = kernel_func(distance, var_h[i]); // change to variable bandwidth
    //     for(int j=0; j<shifted_point.size(); j++){
    //         shifted_point[j] += temp_point[j] * weight;
    //     }
    //     total_weight += weight;
    // }

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
    }while (max_shift_distance > EPSILON_SQR);
    return shifted_points;
}

vector<Cluster> MeanShift::cluster(const std::vector<Point> &points, const std::vector<Point> &shifted_points){
    vector<Cluster> clusters;

    for (int i = 0; i < shifted_points.size(); i++){
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
    vector<Point> a(points.size()); // distances between every point and points[i] 
    vector<double> var_h;
    double hmax = 100000; // need to be tested how big it should be
    int alpha = 19; // need to be determined
    int M = 1;

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
    double tmp = 0;
    for(int i=0; i<points.size(); i++){ 
        tmp = alpha*a[i][M-1]; // M-th nearest points
        var_h.push_back(min(hmax, tmp));
    }

    return var_h;
}