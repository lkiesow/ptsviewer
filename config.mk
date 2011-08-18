# ptsviewer version
VERSION = 0.4c

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

GLINC  =$(shell pkg-config --cflags gl)
GLLIB  =$(shell pkg-config --libs   gl)
GLUINC =$(shell pkg-config --cflags glu)
GLULIB =$(shell pkg-config --libs   glu)
GLUTINC=
GLUTLIB=-lglut

# includes and libs
INCS=-I. -I/usr/X11/include/ ${GLINC} ${GLUINC} ${GLUTINC}
LIBS=-L/usr/lib ${GLLIB} ${GLULIB} ${GLUTLIB}

# dirs for source and object files
OBJDIR   = obj
SRCDIR   = src

# compiler and additional flags
COMPILER = gcc
FLAGS    = -Wall -DVERSION=\"${VERSION}\" ${INCS} ${LIBS}
