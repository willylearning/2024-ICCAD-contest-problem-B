#pragma once 

#include <vector>

#define _DEBUG  0
#if _DEBUG
#define DBGPRN(fmt,args...)          printf(fmt, ##args)
#else
#define DBGPRN(fmt,args...)          ()
#endif

struct Cluster {
    std::vector<double> mode;
    std::vector<std::vector<double>> original_points;
    std::vector<std::vector<double>> shifted_points;
    std::vector<int> original_reg_idx;
    std::vector<int> shifted_reg_idx;
};

class MeanShift {
public:
    typedef std::vector<double> Point;

    MeanShift() { set_kernel(NULL); }
    MeanShift(double (*_kernel_func)(double,double)) { set_kernel(kernel_func); }
    std::vector<Point> meanshift(const std::vector<Point> & points, double kernel_bandwidth, std::vector<double> var_h = std::vector<double>(), double EPSILON = 100); // EPSILON small => more clusters
    std::vector<Cluster> cluster(const std::vector<Point> &, double);
    std::vector<double> variable_bandwidth(const std::vector<Point> &points, double kernel_bandwidth); // define variable bandwidth
    void legalization(const std::vector<Point> &, const std::vector<Point> &);

private:
    double (*kernel_func)(double,double);
    void set_kernel(double (*_kernel_func)(double,double));
    void shift_point(const Point&, const std::vector<Point> &, double, Point&, std::vector<double>);
    std::vector<Cluster> cluster(const std::vector<Point> &, const std::vector<Point> &);
    
};
