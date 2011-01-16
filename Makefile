CC=gcc
INSTALLDIR=$(DESTDIR)/usr/lib/rfkillapplet
BONOBODIR=$(DESTDIR)/usr/lib/bonobo/servers
ICONDIR=$(DESTDIR)/usr/share/icons/hicolor
XMLDIR=$(DESTDIR)/usr/share/gnome-2.0/ui/
PIXMAPDIR=$(DESTDIR)/usr/share/pixmaps
CFLAGS=$(shell pkg-config --cflags --libs libpanelapplet-2.0)
ICONSIZES=16x16 22x22 24x24 32x32
ICONS=rfkill-applet-a.png rfkill-applet-b.png rfkill-applet-c.png

all: build/icons build/rfkillapplet

build/icons:
	$(foreach sizes,$(ICONSIZES),mkdir -p $@/$(sizes)/apps;\
		$(foreach icon,$(ICONS),\
			convert icons/$(icon) -resize $(sizes) $@/$(sizes)/apps/$(icon);\
		)\
	)

build/rfkillapplet:
	$(CC) $(CFLAGS) src/rfkillapplet.c -o $@

install: build/icons build/rfkillapplet
	mkdir -p $(INSTALLDIR)
	mkdir -p $(BONOBODIR)
	mkdir -p $(ICONDIR)
	mkdir -p $(XMLDIR)
	mkdir -p $(PIXMAPDIR)
	cp build/rfkillapplet $(INSTALLDIR)
	mkdir -p $(BONOBODIR)
	cp src/RFKillApplet.server $(BONOBODIR)
	cp src/RFKillApplet.xml $(XMLDIR)
	cp -r build/icons/* $(ICONDIR)
	cp icons/* $(PIXMAPDIR)

clean:
	rm -fr build
