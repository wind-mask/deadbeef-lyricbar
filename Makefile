CFLAGS+=-std=c99 -Wall -O2 -D_GNU_SOURCE -fPIC -fvisibility=hidden -flto=auto
DEBUGFLAGS=-g 
CXXFLAGS+=-std=c++14 -Wall -O2 -fPIC -fvisibility=hidden -flto=auto
LIBFLAGS=`pkg-config --cflags $(GTKMM) $(GTK)`
LIBS=`pkg-config --libs $(GTKMM) $(GTK)`
LCURL=-lcurl
LDFLAGS+=-flto=auto
GLIBC=glib-compile-resources


CC=gcc
CXX=g++
prefix ?= $(out)
prefix ?= /usr

gtk3: GTKMM=gtkmm-3.0
gtk3: GTK=gtk+-3.0 taglib
gtk3: LYRICBAR=ddb_lyricbar_gtk3.so
gtk3: lyricbar

gtk2: GTKMM=gtkmm-2.4
gtk2: GTK=gtk+-2.0
gtk2: LYRICBAR=ddb_lyricbar_gtk2.so
gtk2: lyricbar

lyricbar: config_dialog.o lrcspotify.o megalobiz.o azlyrics.o ui.o base64.o utils.o resources.o main.o kugou.o music163.o
	$(if $(LYRICBAR),, $(error You should only access this target via "gtk3" or "gtk2"))
	  $(CXX) -rdynamic -shared $(LDFLAGS)  main.o resources.o config_dialog.o lrcspotify.o megalobiz.o azlyrics.o kugou.o  music163.o ui.o base64.o utils.o $(LCURL) -o $(LYRICBAR) $(LIBS) -g

lrcspotify.o: src/lrcspotify.cpp src/lrcspotify.h
	$(CXX) src/lrcspotify.cpp -c $(LIBFLAGS) $(CXXFLAGS) -lcurl

megalobiz.o: src/megalobiz.cpp src/megalobiz.h
	$(CXX) src/megalobiz.cpp -c $(LIBFLAGS) $(CXXFLAGS) -lcurl

azlyrics.o: src/azlyrics.cpp src/azlyrics.h
	$(CXX) src/azlyrics.cpp -c $(LIBFLAGS) $(CXXFLAGS) -lcurl

kugou.o: src/kugou.cpp src/kugou.h
	$(CXX) src/kugou.cpp -c $(LIBFLAGS) $(CXXFLAGS) -lcurl
	
music163.o: src/music163.cpp src/music163.h
	$(CXX) src/music163.cpp -c $(LIBFLAGS) $(CXXFLAGS) -lcurl

ui.o: src/ui.cpp src/ui.h
	$(CXX) src/ui.cpp -c $(LIBFLAGS) $(CXXFLAGS)

base64.o: src/base64.cpp src/base64.h
	$(CXX) src/base64.cpp -c $(LIBFLAGS) $(CXXFLAGS)

utils.o: src/utils.cpp src/utils.h
	$(CXX) src/utils.cpp -c $(LIBFLAGS) $(CXXFLAGS)

config_dialog.o: src/config_dialog.cpp src/resources.h
	$(CXX) src/config_dialog.cpp -c $(LIBFLAGS) $(CXXFLAGS) 
	
resources.o: src/resources.c src/resources.h
	$(CC) $(CFLAGS) src/resources.c -c $(LIBFLAGS)

main.o: src/main.c
	$(CC) $(CFLAGS) src/main.c -c $(LIBFLAGS)

src/resources.h src/resources.c: src/resources.xml
	$(GLIBC) --generate-header --target=src/resources.h src/resources.xml
	$(GLIBC) --generate-source --target=src/resources.c src/resources.xml


install: ddb_lyricbar_gtk3.so
	install -d $(DESTDIR)$(prefix)/lib/deadbeef
	install -d $(DESTDIR)$(prefix)/share/locale/ru/LC_MESSAGES
	install -m 644 ddb_lyricbar_gtk3.so $(DESTDIR)$(prefix)/lib/deadbeef/
	msgfmt gettext/ru/deadbeef-lyricbar.po -o $(DESTDIR)$(prefix)/share/locale/ru/LC_MESSAGES/deadbeef-lyricbar.mo

clean:
	rm -f *.o *.so

debug: CFLAGS+=$(DEBUGFLAGS)
debug: CXXFLAGS+=$(DEBUGFLAGS)
debug: gtk3	

.PHONY: gtk3 gtk2 lyricbar install clean debug