LD       = g++
CC       = gcc
CXX      = g++
CPPFLAGS =
CFLAGS   = -I. -Wall -Werror -Wno-unused-function -g -std=c99 -O3
CXXFLAGS = -I. -Wall -Werror -Wno-unused-function -Wno-unused-result -g -O3
LDFLAGS  = -L/usr/X11R6/lib
LDLIBS 	 = -lglut -lGLU -lGL -lm
TARGET   = gmini

SRC      = $(wildcard *.cpp)
OBJ      = $(patsubst %.cpp,%.o,$(SRC))


all:: $(TARGET)

$(TARGET): $(OBJ) Makefile
	$(LD) $(LDFLAGS) $(OBJ) $(OUTPUT_OPTION) $(LDLIBS)

clean::
	@rm -f $(TARGET) $(OBJ) *.o *.d

##################################################
# Gestion des dépendances
ifneq "$(MAKECMDGOALS)" "clean"
-include $(patsubst %.cpp,%.d,$(SRC))
endif

%.d : %.cpp Makefile
	@$(CC) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -MM -MF $@ -MP -MT $(patsubst %.cpp, %.o,$<) $<