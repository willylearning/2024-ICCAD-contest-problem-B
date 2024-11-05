CXX = g++
CXXFLAGS = -std=c++14 -O3 -g
# CXXFLAGS = -std=c++11 -O3 -g

# Boost库的路径
# BOOST_INCLUDES = -I/home/willy/anaconda3/include/boost # 你們把這邊改成 -I/opt/homebrew/opt/boost/include
# BOOST_LIBS = -L/home/willy/anaconda3/lib -lboost_system -lboost_filesystem # 你們把這邊改成 -L/opt/homebrew/opt/boost/lib -lboost_system -lboost_filesystem

# 定义目标二进制文件
TARGET = input_process

# 源文件
SRCS = input_process.cpp MeanShift.cpp
OBJS = $(SRCS:.cpp=.o)

# 编译所有目标
all: $(TARGET)

# 编译目标二进制文件
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(BOOST_LIBS) -o $@ $^

# 编译 .cpp 文件为 .o 文件的规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(BOOST_INCLUDES) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(TARGET) $(OBJS)


############################################ v3

# CXX = g++
# CXXFLAGS = -std=c++14 -O3 -g -I /opt/homebrew/opt/boost/include
# LDFLAGS = -L /opt/homebrew/opt/boost/lib -lboost_system -lboost_filesystem

# # 定义目标二进制文件
# TARGET = input_process

# # 源文件
# SRCS = input_process.cpp MeanShift.cpp
# OBJS = $(SRCS:.cpp=.o)

# # 编译所有目标
# all: $(TARGET)

# # 编译目标二进制文件
# $(TARGET): $(OBJS)
# 	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# # 编译 .cpp 文件为 .o 文件的规则
# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# # 清理生成的文件
# clean:
# 	rm -f $(TARGET) $(OBJS)
