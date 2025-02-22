CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

SRCS = $(wildcard *.cpp) $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:.cpp=.o)

TARGET = irc_server

all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compiling source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

re: clean all

.PHONY: all clean re
