TARGET = nntile_logger

CC = g++
STARPU_VERSION=1.4

WEBSOCKETPP_INCLUDE_DIR = /opt/homebrew/include/
WEBSOCKETPP_LIB_DIR = /opt/homebrew/lib/

CPPFLAGS = $(shell pkg-config --cflags starpu-$(STARPU_VERSION)) -I$(WEBSOCKETPP_INCLUDE_DIR)
LDLIBS = $(shell pkg-config --libs starpu-$(STARPU_VERSION)) -L$(WEBSOCKETPP_LIB_DIR)

SRC_DIR = ./src/
OBJ_DIR = ./obj/

SRC = $(wildcard $(SRC_DIR)*.cpp)
OBJ = $(patsubst $(SRC_DIR)%.cpp, $(OBJ_DIR)%.o, $(SRC))

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDLIBS)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -f $(TARGET) $(OBJ_DIR)*.o
