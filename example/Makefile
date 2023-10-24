CC := g++
CFLAGS := -Wall -Werror -g -std=c++17 -pthread
TARGET := a.out

INCLUDE_DIR := ../include
SRC_DIR := ../src
TEST_DIR := .

# $(wildcard *.cpp /xxx/xxx/*.cpp): get all .cpp files from the current directory and dir "/xxx/xxx/"
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
TEST_SRCS := $(wildcard *.cpp)

# $(patsubst %.cpp,%.o,$(SRCS)): substitute all ".cpp" file name strings to ".o" file name strings
OBJS := $(patsubst %.cpp,%.o,$(SRCS))
TEST_OBJS := $(patsubst %.cpp,%.o,$(TEST_SRCS))

src: $(TARGET)
$(TARGET): $(OBJS) $(TEST_OBJS)
	$(CC) -o $@ $^
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -I $(INCLUDE_DIR) -c $< -o $@
$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CC) $(CFLAGS) -I $(INCLUDE_DIR) -c $<

clean:
	rm -rf $(TARGET) *.o
	rm -rf $(SRC_DIR)/*.o
	
.PHONY: all clean