# cad_contest

how to compile :
make

how to run :
./input_process inputs/sampleCase
./input_process inputs/diycase
./input_process inputs/testcase1_0614.txt


how to compile cpp_test :
g++ -std=c++11 -O3 -g -c MeanShift.cpp -o MeanShift.o
g++ -std=c++11 -O3 -g -c cpp_test.cpp -o cpp_test.o  
g++ -std=c++11 -O3 -g MeanShift.o cpp_test.o -o main_program
./main_program


how to compile boost :
<!-- g++ -std=c++11 -I /usr/local/include -L /usr/local/lib boost.cpp -o boost -lboost_system -lboost_filesystem
g++ -std=c++11 boost.cpp -o boost -lboost_system -lboost_filesystem -->

<!-- /opt/homebrew/opt/boost  -->
<!-- g++ -std=c++11 -I /opt/homebrew/opt/boost -L /opt/homebrew/opt/boost boost.cpp -o boost -lboost_system -lboost_filesystem -->
brew --prefix boost(find the path where boost is installed)
g++ -std=c++14 -I /opt/homebrew/opt/boost/include -L /opt/homebrew/opt/boost/lib boost.cpp -o b -lboost_system -lboost_filesystem
./b

g++ -std=c++14 -I (path)/include -L (path)/lib boost.cpp -o b -lboost_system -lboost_filesystem
./b
