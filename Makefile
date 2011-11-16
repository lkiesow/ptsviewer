# ptsviewer - simple pointcloudviewer

include config.mk

it: $(OBJDIR)/ptsviewer.o
	$(COMPILER) $(OBJDIR)/*.o -o ptsviewer  $(FLAGS)

$(OBJDIR)/ptsviewer.o: $(SRCDIR)/ptsviewer.c $(SRCDIR)/ptsviewer.h
	@mkdir -p $(OBJDIR)
	$(COMPILER) $(FLAGS) -c $(SRCDIR)/ptsviewer.c -o $(OBJDIR)/ptsviewer.o


# DEBUG
debug: $(OBJDIR)/ptsviewer.dbg.o
	$(COMPILER) $(OBJDIR)/*.o -o ptsviewer  $(DFLAGS)

$(OBJDIR)/ptsviewer.dbg.o: $(SRCDIR)/ptsviewer.c $(SRCDIR)/ptsviewer.h
	@mkdir -p $(OBJDIR)
	$(COMPILER) $(DFLAGS) -c $(SRCDIR)/ptsviewer.c -o $(OBJDIR)/ptsviewer.dbg.o


# RELEASE
release: $(OBJDIR)/ptsviewer.rel.o
	$(COMPILER) $(OBJDIR)/*.o -o ptsviewer  $(RFLAGS)

$(OBJDIR)/ptsviewer.rel.o: $(SRCDIR)/ptsviewer.c $(SRCDIR)/ptsviewer.h
	@mkdir -p $(OBJDIR)
	$(COMPILER) $(RFLAGS) -c $(SRCDIR)/ptsviewer.c -o $(OBJDIR)/ptsviewer.rel.o


test: it
	./ptsviewer

install: release
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f ptsviewer ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/ptsviewer
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < ptsviewer.1 > ${DESTDIR}${MANPREFIX}/man1/ptsviewer.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/ptsviewer.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/ptsviewer
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/ptsviewer.1

clean:
	rm -rf *~ $(OBJDIR) ptsviewer

all: clean it
