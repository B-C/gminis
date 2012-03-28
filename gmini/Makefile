# Makefile pour un unique exécutable

# liste des variables à renseigner
#   CIBLE : nom du programme ( $(CIBLE).c doit contenir main() )
#   SRCS : ensemble des fichiers sources 
#   LIBS : liste des bibliothèques utiles à l'édition des liens 
#          (format : -lnom1 -lnom2 ...) 
#   PREFIX : chemin de la hiérarchie 
#
# NE PAS OUBLIER D'AJOUTER LA LISTE DES DEPENDANCES A LA FIN DU FICHIER

CIBLE = gmini
SRCS =  Camera.cpp gmini.cpp Trackball.cpp 
LIBS =  -lglut -lGLU -lGL -lm

#########################################################"

INCDIR = .
LIBDIR = .
BINDIR = .

# nom du compilateur
CC = g++
CPP = g++

# options du compilateur          
CFLAGS = -Wall -O3 
CXXFLAGS = -Wall -O3 

# option du preprocesseur
CPPFLAGS =  -I$(INCDIR) 

# options du linker et liste des bibliothèques à charger
LDFLAGS = -L/usr/X11R6/lib              
LDLIBS = -L$(LIBDIR) $(LIBS)  

# construire la liste des fichiers objets une nouvelle chaine à partir
# de SRCS en substituant les occurences de ".c" par ".o" 
OBJS = $(SRCS:.cpp=.o)   

# cible par défaut
$(CIBLE): $(OBJS)

install:  $(CIBLE)
	cp $(CIBLE) $(BINDIR)/

installdirs:
	test -d $(INCDIR) || mkdir $(INCDIR)
	test -d $(LIBDIR) || mkdir $(LIBDIR)
	test -d $(BINDIR) || mkdir $(BINDIR)

clean:
	rm -f  *~  $(CIBLE) $(OBJS)

veryclean: clean
	rm -f $(BINDIR)/$(CIBLE)

dep:
	gcc $(CPPFLAGS) -MM $(SRCS)

# liste des dépendances générée par 'make dep'
Camera.o: Camera.cpp Camera.h Vec3D.h Trackball.h
gmini.o: gmini.cpp Vec3D.h Camera.h Trackball.h 
Trackball.o: Trackball.cpp Trackball.h


