INCLUDE = $(shell pkg-config hidapi --cflags)
LIBS = $(shell pkg-config hidapi --libs)

all:
	clang++ ${INCLUDE} ${LIBS} -std=c++17 -o pdc-control src/main.cpp
