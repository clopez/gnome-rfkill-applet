CC=gcc
INSTALLDIR=/usr/lib/rfkillapplet
BONOBODIR=/usr/lib/bonobo/servers
ICONDIR=/usr/share/icons/hicolor
PIXMAPDIR=/usr/share/pixmaps
CFLAGS=$(shell pkg-config --cflags --libs libpanelapplet-2.0)
ICONSIZES=16x16 22x22 24x24 32x32
ICONS=rfkill-applet-a.png rfkill-applet-b.png

all: build/icons build/rfkillapplet

build/icons:
	$(foreach sizes,$(ICONSIZES),mkdir -p $@/$(sizes)/apps;\
		$(foreach icon,$(ICONS),\
			convert icons/$(icon) -resize $(sizes) $@/$(sizes)/apps/$(icon);\
		)\
	)

build/rfkillapplet:
	$(CC) $(CFLAGS) src/rfkillapplet.c $< -o $@

install:
	mkdir -p $(INSTALLDIR)
	cp build/rfkillapplet $(INSTALLDIR)
	mkdir -p $(BONOBODIR)
	cp src/RFKillApplet.server $(BONOBODIR)
	cp -r build/icons/* $(ICONDIR)
	cp icons/* $(PIXMAPDIR)

clean:
	rm -fr build
