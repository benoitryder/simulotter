CXX = g++

CFLAGS = -Wall -Werror -I.
# debug info increase compilation significantly, use only when needed
#CFLAGS += -g

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
else
TARGET_EXT = .so
endif


BOOST_PYTHON_LIB = boost_python
PYTHON_LIB = python
PY_CFLAGS = $(CFLAGS)
PY_LDFLAGS = $(LDFLAGS)
PY_LDLIBS = $(LDLIBS) -l$(BOOST_PYTHON_LIB) -l$(PYTHON_LIB)
# precompiled header source
PY_CH_SRC = python/common.h


ifeq ($(OS),Windows_NT)
python_prefix = $(shell python -c 'import sys; print sys.prefix')
PY_CFLAGS += -I$(python_prefix)/include
PY_LDFLAGS += -L$(python_prefix)/libs
BOOST_PYTHON_LIB = boost_python-mgw44-mt-1_43
PYTHON_LIB = python26
PY_TARGET_EXT = .pyd
else
PY_CFLAGS += $(shell python-config --includes)
PYTHON_LIB = python2.6
PY_TARGET_EXT = .so
endif


PROJECT_NAME = simulotter
TARGET = $(PROJECT_NAME)$(TARGET_EXT)

OBJS = physics.o display.o object.o sensors.o robot.o galipeur.o log.o
OBJS += modules/eurobot2009.o modules/eurobot2010.o


PY_TARGET = py$(PROJECT_NAME)$(PY_TARGET_EXT)

PY_SRCS = $(wildcard python/*.cpp)
PY_OBJS = $(PY_SRCS:.cpp=.o)


ALL_OBJS = $(OBJS) $(PY_OBJS)

default: $(TARGET)

python: $(PY_TARGET)

all: default python

-include $(ALL_OBJS:.o=.d)


$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ -shared $(LDFLAGS) $(LDLIBS)

$(PY_TARGET): $(OBJS) $(PY_OBJS)
	$(CXX) $(PY_OBJS) $(OBJS) -o $@ -shared $(PY_LDFLAGS) $(PY_LDLIBS)

$(OBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) -MMD -c $< -o $@


ifeq ($(PY_CH_SRC),)
PY_CH_OBJ = 
else
PY_CH_OBJ = $(PY_CH_SRC).gch

$(PY_CH_OBJ): $(PY_CH_SRC)
	$(CXX) $(PY_CFLAGS) -MMD $< -o $@
endif

$(PY_OBJS): %.o: %.cpp $(PY_CH_OBJ)
	$(CXX) $(PY_CFLAGS) -MMD -c $< -o $@

clean: distclean
	rm -f $(TARGET) $(PY_TARGET)

distclean:
	rm -f $(ALL_OBJS) $(ALL_OBJS:.o=.d) $(PY_CH_OBJ) $(PY_CH_SRC).d

