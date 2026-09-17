UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
	FUNC := clang++
	OMP_PREFIX := $(shell brew --prefix libomp)
	FLAGS := -O3 -g -Werror \
		-Xpreprocessor -fopenmp \
		-I$(OMP_PREFIX)/include \
		-L$(OMP_PREFIX)/lib \
		-Wl,-rpath,$(OMP_PREFIX)/lib \
		-lomp \
		-lm
else
	FUNC := g++
	FLAGS := -O3 -g -Werror -fopenmp -lm
endif

copt := -c
OBJ_DIR := ./bin/

CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix $(OBJ_DIR),$(notdir $(CPP_FILES:.cpp=.obj)))

TEXTURE_CPP_FILES := $(wildcard src/Textures/*.cpp)
TEXTURE_OBJ_FILES := $(addprefix $(OBJ_DIR)Textures/,$(notdir $(TEXTURE_CPP_FILES:.cpp=.obj)))

all:
	cd ./src && make
	$(FUNC) ./main.cpp -o ./main.exe ./src/*.obj ./src/Textures/*.obj $(FLAGS)

clean:
	cd ./src && make clean
	rm -f ./*.exe
	rm -f ./*.obj