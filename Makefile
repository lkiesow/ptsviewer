COMPILER = gcc
FLAGS    = -Wall -lGL -lglut -lGLU
OBJDIR   = obj
SRCDIR   = src

it: $(OBJDIR)/ptsviewer.o
	$(COMPILER) $(FLAGS) $(OBJDIR)/*.o -o ptsviewer

$(OBJDIR)/ptsviewer.o: $(SRCDIR)/ptsviewer.c $(SRCDIR)/ptsviewer.h
	@mkdir -p $(OBJDIR)
	$(COMPILER) $(FLAGS) -c $(SRCDIR)/ptsviewer.c -o $(OBJDIR)/ptsviewer.o

test: all
	./ptsviewer

clean:
	rm -rf *~ $(OBJDIR) ptsviewer

all: clean it
