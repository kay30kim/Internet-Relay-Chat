CXX = c++
CXXFLAGS = -Wall -Wextra -MMD -MP -g3 -std=c++11 -I$(INC_DIR)

SRC_DIR = src
CMD_DIR = src/commands
CLASS_DIR = src/class

SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(CMD_DIR)/*.cpp) $(wildcard $(CLASS_DIR)/*.cpp)
INC_DIR = include
OBJ_DIR = obj

OBJS = $(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%, $(SRCS:.cpp=.o))

TARGET = irc_server

all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

# Compiling source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(CMD_DIR)/%.cpp | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(CLASS_DIR)/%.cpp | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR) $(OBJ_DIR)/commands $(OBJ_DIR)/class

clean:
	rm -rf $(OBJ_DIR) $(TARGET) client

re: clean all

.PHONY: all clean re
