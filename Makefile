CXX      := g++
CXXFLAGS := -std=c++17 -O3 -Wall

FILES := fast_list.cpp

TARGET := program
$(TARGET): benchmark.cpp
	$(CXX) $(CXXFLAGS) benchmark.cpp -o $(TARGET) $(FILES)