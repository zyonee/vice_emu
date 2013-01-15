#!/bin/sh


LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

#ln -sf libsdl-1.2.so $LOCAL_PATH/../obj/local/armeabi/libSDL.so

if [ \! -f vice/configure ] ; then
	sh -c "cd vice && ./autogen.sh"
fi

if [ \! -f vice/Makefile ] ; then
	./setEnvironment-r5b.sh sh -c "cd vice && ./configure --host=arm-eabi --without-x --enable-sdlui --with-sdlsound --without-resid --disable-textfield --disable-fullscreen --disable-nls --disable-realdevice --disable-ffmpeg --disable-ethernet --disable-ipv6 --disable-lame --disable-rs232 --disable-midi --without-png --without-oss --without-alsa --without-pulse"
fi 
 
make -C vice && mv -f vice/src/x64 libapplication.so
