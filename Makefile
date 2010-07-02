CXX = g++

CFLAGS = -g -Wall -Werror -I.

# If Bullet has been built using autotools or Jam
#BULLET_LIBS = -lbulletdynamics -lbulletcollision -lbulletmath
# If Bullet has been built using cmake
BULLET_LIBS = -lBulletDynamics -lBulletCollision -lLinearMath

LDLIBS = 
ifeq ($(OS),Windows_NT)
GL_LIBS = -lfreeglut -lopengl32 -lglu32 -lwinmm
LDLIBS += -mconsole -mwindows
else
GL_LIBS = -lGL -lGLU -lglut
endif
LDLIBS += $(GL_LIBS) -lSDL -lpng $(BULLET_LIBS) -lm

ifeq ($(OS),Windows_NT)
CFLAGS += -DWIN32 -DFREEGLUT_STATIC
TARGET_EXT = .dll
endif


PROJECT_NAME = simulotter
TARGET = $(PROJECT_NAME)$(TARGET_EXT)
TARGET_EXT = .so

OBJS = physics.o display.o object.o sensors.o robot.o galipeur.o log.o
OBJS += modules/eurobot2009.o modules/eurobot2010.o


all: $(TARGET)

-include $(ALL_OBJS:.o=.d)


$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ -shared $(LDFLAGS) $(LDLIBS)

$(OBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) -MMD -c $< -o $@

clean: distclean
	rm -f $(TARGET)

distclean:
	rm -f $(OBJS) $(OBJS:.o=.d)

