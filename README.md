# 2024 ICCAD contest problem B: Power and Timing Optimization Using Multibit Flip-Flop

how to compile :
make

how to run :  
./input_process inputs/sampleCase outputs/sampleCase_output.txt  
./input_process inputs/testcase1_0812.txt outputs/testcase1_0812_output.txt | tee log.txt  
./input_process inputs/testcase2_0812.txt outputs/testcase2_0812_output.txt  

how to check connection :  
./sanity_20240801 inputs/testcase1_0812.txt outputs/testcase1_0812_output.txt  
./sanity_20240801 inputs/testcase2_0812.txt outputs/testcase2_0812_output.txt  

how to compile cpp_test :  
g++ -std=c++11 -O3 -g -c MeanShift.cpp -o MeanShift.o  
g++ -std=c++11 -O3 -g -c cpp_test.cpp -o cpp_test.o  
g++ -std=c++11 -O3 -g MeanShift.o cpp_test.o -o main_program  
./main_program
