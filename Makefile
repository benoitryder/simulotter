CXX = g++

CFLAGS = -g -Wall `ode-config --cflags` -I/usr/local/include/lua5.1 -I/usr/include/lua5.1

ifeq ($(OS),Windows_NT)
LDFLAGS = -mconsole -lfreeglut -mwindows -lSDL -lm -lode -lopengl32 -lglu32 -lwinmm -llua
else
LDFLAGS = `sdl-config --libs` -lm -lode -lGL -llua5.1 -lglut
endif

OBJS = main.o config.o physics.o display.o object.o robot.o match.o maths.o log.o sensor.o
OBJS += eurobot2009.o
# lua_utils must be the last one to register all Lua classes
OBJS += lua_utils.o

TARGET := simulotter


ifeq ($(OS),Windows_NT)
CFLAGS += -I$(dir $(shell $(CXX) -print-file-name=liblua.a))/../include/lua5.1/
CFLAGS += -DWIN32 -DFREEGLUT_STATIC
TARGET := $(TARGET).exe
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp config.h
	$(CXX) $(CFLAGS) -c $< -o $@

clean: distclean
	rm -f $(TARGET)

distclean:
	rm -f $(OBJS)

