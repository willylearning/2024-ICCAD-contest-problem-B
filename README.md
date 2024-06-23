# cad_contest

how to compile :
make

how to run :
./input_process inputs/sampleCase
./input_process inputs/testcase1_0614.txt


how to compile cpp_test :
g++ -std=c++11 -O3 -g -c MeanShift.cpp -o MeanShift.o
g++ -std=c++11 -O3 -g -c cpp_test.cpp -o cpp_test.o  
g++ -std=c++11 -O3 -g MeanShift.o cpp_test.o -o main_program
./main_program