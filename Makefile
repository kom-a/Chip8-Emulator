CXX = g++

CFLAGS = -Wall
TARGET = src/*.cpp
LINKING = -lGL -lglut -lGLU

all:
	$(CXX) -o chip8 $(TARGET) $(LINKING)
	./chip8 roms/Pong.rom
