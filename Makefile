# CXX = g++
# CXXFLAGS = -std=c++14 -O3 -g

# # 定義目標二進制文件
# TARGETS = input_process_original MeanShift

# # 編譯所有目標
# all: $(TARGETS)

# # 編譯input_process目標
# input_process: input_process_original.o
# 	$(CXX) $(CXXFLAGS) -o $@ $^

# # 編譯MeanShift目標
# MeanShift: cpp_test_original.o MeanShift.o
# 	$(CXX) $(CXXFLAGS) -o $@ $^

# # 編譯.cpp文件為.o文件的規則
# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# # 清理生成的文件
# clean:
# 	rm -f $(TARGETS) *.o

CXX = g++
CXXFLAGS = -std=c++14 -O3 -g -I /home/willy/anaconda3/include/boost -L /home/willy/anaconda3/lib/

# 定义目标二进制文件
TARGET = input_process

# 源文件
SRCS = input_process.cpp MeanShift.cpp
OBJS = $(SRCS:.cpp=.o)

# 编译所有目标
all: $(TARGET)

# 编译目标二进制文件
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 编译 .cpp 文件为 .o 文件的规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(TARGET) $(OBJS)

