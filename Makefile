CXX = g++
CXXFLAGS = -std=c++11 -O3 -g 

# 定義目標二進制文件
TARGETS = input_process MeanShift

# 編譯所有目標
all: $(TARGETS)

# 編譯input_process目標
input_process: input_process.o
	$(CXX) $(CXXFLAGS) -o $@ $^

# 編譯MeanShift目標
MeanShift: cpp_test.o MeanShift.o
	$(CXX) $(CXXFLAGS) -o $@ $^

# 編譯.cpp文件為.o文件的規則
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(TARGETS) *.o
