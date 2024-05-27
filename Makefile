TARGET = nntile_logger

CC = g++
STARPU_VERSION=1.4

CPPFLAGS = $(shell pkg-config --cflags starpu-$(STARPU_VERSION))
LDLIBS = $(shell pkg-config --libs starpu-$(STARPU_VERSION))

SRC_DIR = ./src/
OBJ_DIR = ./obj/

SRC = $(wildcard $(SRC_DIR)*.cpp)
OBJ = $(patsubst $(SRC_DIR)%.cpp, $(OBJ_DIR)%.o, $(SRC))

$(TARGET) : $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDLIBS)

$(OBJ_DIR)%.o : $(SRC_DIR)%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	rm $(TARGET) $(OBJ_DIR)*.o