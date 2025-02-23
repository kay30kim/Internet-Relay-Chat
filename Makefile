CXX = c++
CXXFLAGS = -Wall -Wextra -MMD -MP -g3 -std=c++98 -I$(INC_DIR)

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

SRCS = main.cpp $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

TARGET = irc_server

all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

# Compiling source files
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ensure obj directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) client

re: clean all

.PHONY: all clean re
