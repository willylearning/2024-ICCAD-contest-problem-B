#include <iostream>
#include <vector>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

struct Point {
    double x, y;
};

int main() {
    // 示例数据
    std::vector<Point> points = {
        {1.0, 2.0}, {2.0, 3.0}, {3.0, 4.0}, {4.0, 5.0}, {5.0, 6.0},
        {6.0, 7.0}, {7.0, 8.0}, {8.0, 9.0}, {9.0, 10.0}, {10.0, 11.0},
        {11.0, 12.0}, {12.0, 13.0}, {13.0, 14.0}, {14.0, 15.0}
    };

    // 转换为Boost.Geometry的点类型
    using BoostPoint = bg::model::point<double, 2, bg::cs::cartesian>;
    std::vector<BoostPoint> boostPoints;
    for (const auto& p : points) {
        boostPoints.emplace_back(p.x, p.y);
    }

    // 构建R-tree
    bgi::rtree<BoostPoint, bgi::quadratic<16>> rtree(boostPoints.begin(), boostPoints.end());

    // 查询一个点的12个最近邻居
    BoostPoint queryPoint(5.5, 6.5);
    std::vector<BoostPoint> resultN;
    rtree.query(bgi::nearest(queryPoint, 5), std::back_inserter(resultN));

    // 输出结果
    std::cout << "The 12 nearest neighbors to point (" << bg::get<0>(queryPoint) << ", " << bg::get<1>(queryPoint) << ") are:\n";
    for (const auto& p : resultN) {
        std::cout << "(" << bg::get<0>(p) << ", " << bg::get<1>(p) << ")\n";
    }

    return 0;
}
