SUBDIRS = archives p64 liblame

ffmpeglibs: $(FFMPEG_DIR)

.PHONY: libp64 libx264 liblame libffmpeg

libp64:
	(cd p64; $(MAKE))

libx264:
	(cd libx264; ./configure --enable-shared --enable-static --prefix=$(PWD)/libffmpeg; $(MAKE) install)

liblame:
	(cd liblame; ./configure --enable-shared --disable-frontend --prefix=$(PWD)/libffmpeg; $(MAKE) install)

libffmpeg: libx264 liblame
	(cd libffmpeg; ./configure --enable-libmp3lame --enable-libx264 --enable-shared --disable-static --disable-programs --enable-gpl --extra-cflags="-Iinclude" --extra-ldflags="-Llib" --prefix=$(VICEDIR); $(MAKE))

ffmpeglibs-install:
	(cd libffmpeg; $(MAKE) install-libs)

