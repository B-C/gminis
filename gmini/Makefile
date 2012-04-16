LD       = g++
CC       = gcc
CXX      = g++
CPPFLAGS =
CFLAGS   = -I. -Wall -Werror -Wno-unused-function -g -std=c99 -O3
CXXFLAGS = -I. -Wall -Werror -Wno-unused-function -Wno-unused-result -std=c++0x -g -O3
LDFLAGS  = -L/usr/X11R6/lib
LDLIBS 	 = -lglut -lGLU -lGL -lm
TARGET   = gmini gmini_2D

SRC 	 = $(filter-out gmini_2D.cpp, $(filter-out gmini.cpp,$(wildcard *.cpp)))
OBJ      = $(patsubst %.cpp,%.o,$(SRC))


all:: $(TARGET)

gmini: $(OBJ) gmini.o Makefile
	$(LD) $(LDFLAGS) $(filter-out Makefile,$^) $(OUTPUT_OPTION) $(LDLIBS)

gmini_2D: $(OBJ) gmini_2D.o Makefile
	$(LD) $(LDFLAGS) $(filter-out Makefile,$^) $(OUTPUT_OPTION) $(LDLIBS)

clean::
	@rm -f $(TARGET) $(OBJ) *.o *.d

##################################################
# Gestion des dépendances
ifneq "$(MAKECMDGOALS)" "clean"
-include $(patsubst %.cpp,%.d,$(SRC))
-include gmini.d
-include gmini_2D.d
endif

%.d : %.cpp Makefile
	@$(CC) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -MM -MF $@ -MP -MT $(patsubst %.cpp, %.o,$<) $<