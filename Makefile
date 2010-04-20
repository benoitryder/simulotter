CXX = g++

CFLAGS = -g -Wall -Werror -I/usr/local/include/lua5.1 -I/usr/include/lua5.1 -I.

# If Bullet has been built using autotools or Jam
#BULLET_LIBS = -lbulletdynamics -lbulletcollision -lbulletmath
# If Bullet has been built using cmake
BULLET_LIBS = -lBulletDynamics -lBulletCollision -lLinearMath

ifeq ($(OS),Windows_NT)
LDFLAGS = -mconsole -lfreeglut -mwindows -lSDL -lm $(BULLET_LIBS) -lopengl32 -lglu32 -lwinmm -llua -lpng
else
LDFLAGS = -lm $(BULLET_LIBS) -llua5.1 -lGL -lGLU -lglut -lSDL -lpng
endif

OBJS = main.o config.o physics.o display.o object.o sensors.o robot.o galipeur.o log.o
OBJS += modules/eurobot2009.o modules/eurobot2010.o
# lua_utils must be the last one to register all Lua classes
OBJS += lua_utils.o

TARGET := simulotter


ifeq ($(OS),Windows_NT)
CFLAGS += -I$(dir $(shell $(CXX) -print-file-name=liblua.a))../include/lua5.1/
CFLAGS += -DWIN32 -DFREEGLUT_STATIC
RES := $(TARGET).res
TARGET := $(TARGET).exe
endif

all: $(TARGET)

-include $(OBJS:.o=.d)

$(TARGET): $(OBJS) $(RES)
	$(CXX) $(CFLAGS) $(OBJS) $(RES) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CFLAGS) -MMD -c $< -o $@

ifneq ($(RES),)
%.res: %.rc
	windres $< -O coff -o $@
endif

clean: distclean
	rm -f $(TARGET)

distclean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(RES)

