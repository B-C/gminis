LD       = g++
CC       = gcc
CXX      = g++
CPPFLAGS =
CFLAGS   = -I. -Wall -Werror -g -std=c99 -O3
CXXFLAGS = -I. -Wall -Werror -g -O3
LDFLAGS  = -L/usr/X11R6/lib
LDLIBS 	 = -lglut -lGLU -lGL -lm
TARGET   = gmini

SRC      = $(wildcard *.cpp)
OBJ      = $(patsubst %.cpp,%.o,$(SRC))


all:: $(TARGET)

$(TARGET): $(OBJ) Makefile
	$(LD) $(LDFLAGS) $(OBJ) $(OUTPUT_OPTION) $(LDLIBS)

clean::
	@rm -f $(TARGET) $(OBJ)

##################################################
# Gestion des dépendances
ifneq "$(MAKECMDGOALS)" "clean"

-include $(patsubst %.cpp,%.d,$(SRC))

endif

%.d : %.cpp Makefile
	$(CC) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -MM -MF $@ -MP -MT $(patsubst %.cpp, %.o,$<) $<