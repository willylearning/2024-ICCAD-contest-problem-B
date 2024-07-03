#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/point.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

// 定义点和R-tree类型
typedef bg::model::point<double, 2, bg::cs::cartesian> Point;
typedef std::pair<Point, int> Value;
typedef bgi::rtree<Value, bgi::quadratic<16>> RTree;

// 读取CSV文件中的点数据
std::vector<Value> read_csv(const std::string& filename) {
    std::vector<Value> values;
    std::ifstream file(filename);
    std::string line;
    int id = 0;  // 从1开始，而不是0

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        double x, y;
        char comma;
        ss >> x >> comma >> y;
        values.emplace_back(Point(x, y), id++);
    }

    return values;
}

// KNN 查询函数
std::vector<std::vector<std::pair<int, double>>> knn_query(const std::vector<Value>& values, int K) {
    RTree rtree(values.begin(), values.end());
    std::vector<std::vector<std::pair<int, double>>> knn_results;

    for (const auto& query : values) {
        std::vector<Value> result_n;
        rtree.query(bgi::nearest(query.first, K + 1), std::back_inserter(result_n));  // 加1是因为自己也是邻居之一

        std::vector<std::pair<int, double>> neighbors;
        for (const auto& v : result_n) {
            if (v.second != query.second) {  // 排除自己
                double distance = bg::distance(query.first, v.first);
                neighbors.emplace_back(v.second, distance);
                if (neighbors.size() >= K) break;
            }
        }
        knn_results.push_back(neighbors);
    }

    return knn_results;
}

// 写入KNN查询结果到CSV文件
void write_knn_results(const std::string& filename, const std::vector<std::vector<std::pair<int, double>>>& knn_results) {
    std::ofstream file(filename);

    for (const auto& neighbors : knn_results) {
        bool first = true;
        for (const auto& neighbor : neighbors) {
            if (!first) {
                file << ",";
            }
            file << neighbor.first << "," << neighbor.second;
            first = false;
        }
        file << std::endl;
    }

    file.close();
}

int main() {
    std::string input_filename = "test.csv";
    std::vector<Value> values = read_csv(input_filename);

    // 选择K值
    int K;
    std::cout << "Enter number of nearest neighbors (K): ";
    std::cin >> K;

    // 执行KNN查询
    std::vector<std::vector<std::pair<int, double>>> knn_results = knn_query(values, K);

    // 输出结果到文件
    std::string output_filename = "knn_results.csv";
    write_knn_results(output_filename, knn_results);

    std::cout << "KNN results have been written to " << output_filename << std::endl;

    return 0;
}
