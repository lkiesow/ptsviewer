# ptsviewer version
VERSION = 0.7.4

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
MLIB   =-lm

# includes and libs
INCS=-I. -I/usr/X11/include/ ${GLINC} ${GLUINC} ${GLUTINC}
LIBS=-L/usr/lib ${GLLIB} ${GLULIB} ${GLUTLIB} ${MLIB} -lrply

# dirs for source and object files
OBJDIR   = obj
SRCDIR   = src

# compiler and additional flags
COMPILER = gcc
FLAGS    = -Wall -DVERSION=\"${VERSION}\" ${INCS} ${LIBS}
RFLAGS   = ${FLAGS} -O3
DFLAGS   = ${FLAGS} -g
