#!/bin/sh

if [ ! "$OPENXDK" ]; then
  echo "Set environment variable OPENXDK first"
  exit
fi

CC_COMPILE_FLAGS="-c -g -std=gnu99 -ffreestanding -nostdlib -fno-builtin -fno-exceptions -mno-cygwin -march=i386 -D__XBOX__ -I$OPENXDK/i386-pc-xbox/include -I$OPENXDK/include -I$OPENXDK/include/SDL"
CXX_COMPILE_FLAGS="-c -g -nostdlib -fno-builtin -fno-exceptions -mno-cygwin -march=i386 -D__XBOX__ -I$OPENXDK/i386-pc-xbox/include -I$OPENXDK/include -I$OPENXDK/include/SDL"
LINK_FLAGS="-L$OPENXDK/lib -L$OPENXDK/i386-pc-xbox/lib -mno-cygwin -ljpeg -lz -lSDL -lopenxdk -lhal -lc -lhal -lusb -lm -lc -lxboxkrnl -nostdlib -Wl,--file-alignment,0x20 -Wl,--section-alignment,0x20 -shared -Wl,--entry,_WinMainCRTStartup -Wl,--strip-all"

cd src
echo "generating translate.h..."
/bin/sh ./gentranslate_h.sh <./translate.txt >translate.h

echo "generating translate_table.h..."
/bin/sh ./gentranslatetable.sh <./translate.txt >translate_table.h

cd lib/lpng
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I../.. -I../../arch/sdl/xbox"
for i in png.c pngerror.c pngget.c pngmem.c pngpread.c pngread.c pngrio.c pngrtran.c pngrutil.c pngset.c pngtrans.c \
         pngwio.c pngwrite.c pngwtran.c pngwutil.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libpng.a
$OPENXDK/bin/i386-pc-xbox-ar cru libpng.a png.o pngerror.o pngget.o pngmem.o pngpread.o pngread.o pngrio.o pngrtran.o \
                                 pngrutil.o pngset.o pngtrans.o pngwio.o pngwrite.o pngwtran.o pngwutil.o
$OPENXDK/bin/i386-pc-xbox-ranlib libpng.a

cd ../../sounddrv
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../arch/sdl/xbox"
for i in soundaiff.c sounddummy.c sounddump.c soundfs.c soundiff.c soundspeed.c soundvoc.c soundwav.c soundsdl.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libsounddrv.a
$OPENXDK/bin/i386-pc-xbox-ar cru libsounddrv.a soundaiff.o sounddummy.o sounddump.o soundfs.o soundiff.o soundspeed.o \
                                               soundvoc.o soundwav.o soundsdl.o
$OPENXDK/bin/i386-pc-xbox-ranlib libsounddrv.a

cd ../drive/iec/c64exp
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I. -I../../.. -I../../../arch/sdl -I../../../drive -I../../../drive/iec -I../../../arch/sdl/xbox"
for i in c64exp-cmdline-options.c c64exp-resources.c iec-c64exp.c mc6821.c profdos.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libdriveiecc64exp.a
$OPENXDK/bin/i386-pc-xbox-ar cru libdriveiecc64exp.a c64exp-cmdline-options.o c64exp-resources.o iec-c64exp.o mc6821.o \
                                                     profdos.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libdriveiecc64exp.a

cd ../plus4exp
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I. -I../../.. -I../../../arch/sdl -I../../../drive -I../../../drive/iec -I../../../arch/sdl/xbox"
for i in iec-plus4exp.c plus4exp-cmdline-options.c plus4exp-resources.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libdriveiecplus4exp.a
$OPENXDK/bin/i386-pc-xbox-ar cru libdriveiecplus4exp.a iec-plus4exp.o plus4exp-cmdline-options.o plus4exp-resources.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libdriveiecplus4exp.a

cd ..
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I../.. -I../../arch/sdl -I../../drive -I../../arch/sdl/xbox"
for i in cia1571d.c cia1581d.c glue1571.c iec-cmdline-options.c iec-resources.c iec.c iecrom.c memiec.c via1d1541.c \
         wd1770.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libdriveiec.a
$OPENXDK/bin/i386-pc-xbox-ar cru libdriveiec.a cia1571d.o cia1581d.o glue1571.o iec-cmdline-options.o iec-resources.o \
                                               iec.o iecrom.o memiec.o via1d1541.o wd1770.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libdriveiec.a

cd ../iec128dcr
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I../.. -I../../arch/sdl -I../../drive -I../../arch/sdl/xbox"
for i in iec128dcr-cmdline-options.c iec128dcr-resources.c iec128dcr.c iec128dcrrom.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libdriveiec128dcr.a
$OPENXDK/bin/i386-pc-xbox-ar cru libdriveiec128dcr.a iec128dcr-cmdline-options.o iec128dcr-resources.o iec128dcr.o \
                                                     iec128dcrrom.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libdriveiec128dcr.a

cd ../iecieee
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I../.. -I../../arch/sdl -I../../drive -I../../arch/sdl/xbox"
for i in iecieee.c via2d.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libdriveiecieee.a
$OPENXDK/bin/i386-pc-xbox-ar cru libdriveiecieee.a iecieee.o via2d.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libdriveiecieee.a

cd ../ieee
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I../.. -I../../arch/sdl -I../../drive -I../../arch/sdl/xbox"
for i in fdc.c ieee-cmdline-options.c ieee-resources.c ieee.c ieeerom.c memieee.c riot1d.c riot2d.c via1d2031.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libdriveieee.a
$OPENXDK/bin/i386-pc-xbox-ar cru libdriveieee.a fdc.o ieee-cmdline-options.o ieee-resources.o ieee.o ieeerom.o \
                                                memieee.o riot1d.o riot2d.o via1d2031.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libdriveieee.a

cd ../tcbm
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I../.. -I../../arch/sdl -I../../drive -I../../arch/sdl/xbox"
for i in glue1551.c mem1551.c tcbm-cmdline-options.c tcbm-resources.c tcbm.c tcbmrom.c tpid.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libdrivetcbm.a
$OPENXDK/bin/i386-pc-xbox-ar cru libdrivetcbm.a glue1551.o mem1551.o tcbm-cmdline-options.o tcbm-resources.o tcbm.o \
                                                tcbmrom.o tpid.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libdrivetcbm.a

cd ..
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../vdrive -I../arch/sdl/xbox"
for i in drive-check.c drive-cmdline-options.c drive-overflow.c drive-resources.c drive-snapshot.c drive-writeprotect.c \
         drive.c drivecpu.c drivemem.c driveimage.c driverom.c drivesync.c rotation.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libdrive.a
$OPENXDK/bin/i386-pc-xbox-ar cru libdrive.a drive-check.o drive-cmdline-options.o drive-overflow.o drive-resources.o \
                                            drive-snapshot.o drive-writeprotect.o drive.o drivecpu.o drivemem.o \
                                            driveimage.o driverom.o drivesync.o rotation.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libdrive.a

cd ../vdrive
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../arch/sdl/xbox"
for i in vdrive-bam.c vdrive-command.c vdrive-dir.c vdrive-iec.c vdrive-internal.c vdrive-rel.c vdrive-snapshot.c \
         vdrive.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libvdrive.a
$OPENXDK/bin/i386-pc-xbox-ar cru libvdrive.a vdrive-bam.o vdrive-command.o vdrive-dir.o vdrive-iec.o vdrive-internal.o \
                                             vdrive-rel.o vdrive-snapshot.o vdrive.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libvdrive.a

cd ../fsdevice
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../vdrive -I../arch/sdl/xbox"
for i in fsdevice-close.c fsdevice-cmdline-options.c fsdevice-flush.c fsdevice-open.c fsdevice-read.c \
         fsdevice-resources.c fsdevice-write.c fsdevice.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libfsdevice.a
$OPENXDK/bin/i386-pc-xbox-ar cru libfsdevice.a fsdevice-close.o fsdevice-cmdline-options.o fsdevice-flush.o \
                                               fsdevice-open.o fsdevice-read.o fsdevice-resources.o fsdevice-write.o \
                                               fsdevice.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libfsdevice.a

cd ../diskimage
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../arch/sdl/xbox"
for i in diskimage.c fsimage-check.c fsimage-create.c fsimage-gcr.c fsimage-probe.c fsimage.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libdiskimage.a
$OPENXDK/bin/i386-pc-xbox-ar cru libdiskimage.a diskimage.o fsimage-check.o fsimage-create.o fsimage-gcr.o \
                                                fsimage-probe.o fsimage.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libdiskimage.a

cd ../iecbus
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../drive -I../arch/sdl/xbox"
for i in iecbus.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libiecbus.a
$OPENXDK/bin/i386-pc-xbox-ar cru libiecbus.a iecbus.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libiecbus.a

cd ../serial
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../drive -I../arch/sdl/xbox"
for i in fsdrive.c serial-device.c serial-iec-bus.c serial-iec-device.c serial-iec-lib.c serial-iec.c \
         serial-realdevice.c serial-trap.c serial.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libserial.a
$OPENXDK/bin/i386-pc-xbox-ar cru libserial.a fsdrive.o serial-device.o serial-iec-bus.o serial-iec-device.o \
                                             serial-iec-lib.o serial-iec.o serial-realdevice.o serial-trap.o serial.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libserial.a

cd ../parallel
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../drive -I../arch/sdl/xbox"
for i in parallel-trap.c parallel.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libparallel.a
$OPENXDK/bin/i386-pc-xbox-ar cru libparallel.a parallel-trap.o parallel.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libparallel.a

cd ../tape
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../arch/sdl/xbox"
for i in t64.c tap.c tape-internal.c tape-snapshot.c tape.c tapeimage.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libtape.a
$OPENXDK/bin/i386-pc-xbox-ar cru libtape.a t64.o tap.o tape-internal.o tape-snapshot.o tape.o tapeimage.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libtape.a

cd ../imagecontents
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../vdrive -I../arch/sdl/xbox"
for i in diskcontents-block.c diskcontents-iec.c diskcontents.c imagecontents.c tapecontents.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libimagecontents.a
$OPENXDK/bin/i386-pc-xbox-ar cru libimagecontents.a diskcontents-block.o diskcontents-iec.o diskcontents.o \
                                                    imagecontents.o tapecontents.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libimagecontents.a

cd ../fileio
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../arch/sdl/xbox"
for i in cbmfile.c fileio.c p00.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libfileio.a
$OPENXDK/bin/i386-pc-xbox-ar cru libfileio.a cbmfile.o fileio.o p00.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libfileio.a

cd ../video
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../raster -I../arch/sdl/xbox"
for i in render1x1.c render1x1pal.c render1x2.c render2x2.c render2x2pal.c renderscale2x.c renderyuv.c video-canvas.c \
         video-cmdline-options.c video-color.c video-render-1x2.c video-render-2x2.c video-render-pal.c video-render.c \
         video-resources-pal.c video-resources.c video-viewport.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libvideo.a
$OPENXDK/bin/i386-pc-xbox-ar cru libvideo.a render1x1.o render1x1pal.o render1x2.o render2x2.o render2x2pal.o \
                                            renderscale2x.o renderyuv.o video-canvas.o video-cmdline-options.o \
                                            video-color.o video-render-1x2.o video-render-2x2.o video-render-pal.o \
                                            video-render.o video-resources-pal.o video-resources.o video-viewport.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libvideo.a

cd ../raster
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../arch/sdl/xbox"
for i in raster-cache.c raster-canvas.c raster-changes.c raster-cmdline-options.c raster-line-changes-sprite.c \
         raster-line-changes.c raster-line.c raster-modes.c raster-resources.c raster-sprite.c raster-sprite-status.c \
         raster-sprite-cache.c raster.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libraster.a
$OPENXDK/bin/i386-pc-xbox-ar cru libraster.a raster-cache.o raster-canvas.o raster-changes.o raster-cmdline-options.o \
                                             raster-line-changes-sprite.o raster-line-changes.o raster-line.o \
                                             raster-modes.o raster-resources.o raster-sprite.o raster-sprite-status.o \
                                             raster-sprite-cache.o raster.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libraster.a

cd ../vicii
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../raster -I../c64 -I../c64dtv -I../arch/sdl/xbox"
for i in vicii-badline.c vicii-clock-stretch.c vicii-cmdline-options.c vicii-color.c vicii-draw.c vicii-fetch.c \
         vicii-irq.c vicii-mem.c vicii-phi1.c vicii-resources.c vicii-snapshot.c vicii-sprites.c vicii-stubs.c \
         vicii-timing.c vicii.c viciidtv-color.c viciidtv-draw.c viciidtv-snapshot.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libvicii.a
$OPENXDK/bin/i386-pc-xbox-ar cru libvicii.a vicii-badline.o vicii-clock-stretch.o vicii-cmdline-options.o vicii-color.o \
                                            vicii-draw.o vicii-fetch.o vicii-irq.o vicii-mem.o vicii-phi1.o \
                                            vicii-resources.o vicii-snapshot.o vicii-sprites.o vicii-stubs.o \
                                            vicii-timing.o vicii.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libvicii.a
rm -f libviciidtv.a
$OPENXDK/bin/i386-pc-xbox-ar cru libviciidtv.a vicii-badline.o vicii-cmdline-options.o viciidtv-color.o viciidtv-draw.o \
                                               vicii-fetch.o vicii-irq.o vicii-mem.o vicii-phi1.o vicii-resources.o \
                                               viciidtv-snapshot.o vicii-sprites.o vicii-timing.o vicii.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libviciidtv.a

cd ../vdc
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../raster -I../arch/sdl/xbox"
for i in vdc-cmdline-options.c vdc-draw.c vdc-mem.c vdc-resources.c vdc-snapshot.c vdc.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libvdc.a
$OPENXDK/bin/i386-pc-xbox-ar cru libvdc.a vdc-cmdline-options.o vdc-draw.o vdc-mem.o vdc-resources.o vdc-snapshot.o vdc.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libvdc.a

cd ../crtc
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../raster -I../arch/sdl/xbox"
for i in crtc-cmdline-options.c crtc-draw.c crtc-mem.c crtc-resources.c crtc-snapshot.c crtc.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libcrtc.a
$OPENXDK/bin/i386-pc-xbox-ar cru libcrtc.a crtc-cmdline-options.o crtc-draw.o crtc-mem.o crtc-resources.o \
                                           crtc-snapshot.o crtc.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libcrtc.a

cd ../gfxoutputdrv
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../lib/lpng -I../arch/sdl/xbox"
for i in bmpdrv.c gfxoutput.c iffdrv.c pcxdrv.c ppmdrv.c pngdrv.c jpegdrv.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libgfxoutputdrv.a
$OPENXDK/bin/i386-pc-xbox-ar cru libgfxoutputdrv.a bmpdrv.o gfxoutput.o iffdrv.o pcxdrv.o ppmdrv.o pngdrv.o jpegdrv.o
$OPENXDK/bin/i386-pc-xbox-ranlib libgfxoutputdrv.a

cd ../printerdrv
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../arch/sdl/xbox"
for i in driver-select.c drv-ascii.c drv-mps803.c drv-nl10.c interface-serial.c interface-userport.c output-graphics.c \
         output-select.c output-text.c printer-serial.c printer-userport.c printer.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libprinterdrv.a
$OPENXDK/bin/i386-pc-xbox-ar cru libprinterdrv.a driver-select.o drv-ascii.o drv-mps803.o drv-nl10.o interface-serial.o \
                                                 interface-userport.o output-graphics.o output-select.o output-text.o \
                                                 printer-serial.o printer-userport.o printer.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libprinterdrv.a

cd ../rs232drv
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../arch/sdl/xbox"
for i in rs232drv.c rsuser.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f librs232drv.a
$OPENXDK/bin/i386-pc-xbox-ar cru librs232drv.a rs232drv.o rsuser.o 
$OPENXDK/bin/i386-pc-xbox-ranlib librs232drv.a

cd ../sid
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../arch/sdl/xbox"
for i in fastsid.c sid-cmdline-options.c sid-resources.c sid-snapshot.c sid.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libsid.a
$OPENXDK/bin/i386-pc-xbox-ar cru libsid.a fastsid.o sid-cmdline-options.o sid-resources.o sid-snapshot.o sid.o
$OPENXDK/bin/i386-pc-xbox-ranlib libsid.a

cd ../monitor
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../drive -I../vdrive -I../arch/sdl/xbox"
for i in asm6502.c asm6502dtv.c asmz80.c mon_assemble6502.c mon_assemblez80.c mon_breakpoint.c mon_command.c \
         mon_disassemble.c mon_drive.c mon_file.c mon_memory.c mon_register6502.c mon_register6502dtv.c \
         mon_registerz80.c mon_ui.c mon_util.c mon_lex.c mon_parse.c monitor.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libmonitor.a
$OPENXDK/bin/i386-pc-xbox-ar cru libmonitor.a asm6502.o asm6502dtv.o asmz80.o mon_assemble6502.o mon_assemblez80.o \
                                              mon_breakpoint.o mon_command.o mon_disassemble.o mon_drive.o mon_file.o \
                                              mon_memory.o mon_register6502.o mon_register6502dtv.o mon_registerz80.o \
                                              mon_ui.o mon_util.o mon_lex.o mon_parse.o monitor.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libmonitor.a

cd ../core
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../arch/sdl/xbox"
for i in ciacore.c ciatimer.c riotcore.c tpicore.c viacore.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libcore.a
$OPENXDK/bin/i386-pc-xbox-ar cru libcore.a ciacore.o ciatimer.o riotcore.o tpicore.o viacore.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libcore.a

cd ../c64/cart
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I../.. -I../../arch/sdl -I../../c64 -I../../vicii -I../../arch/sdl/xbox"
for i in actionreplay3.c actionreplay4.c actionreplay.c atomicpower.c c64cart.c c64cartmem.c comal80.c crt.c delaep256.c \
         delaep64.c delaep7x8.c epyxfastload.c expert.c final.c generic.c ide64.c kcs.c magicformel.c mikroass.c \
         retroreplay.c rexep256.c ross.c stardos.c stb.c supergames.c supersnapshot.c zaxxon.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libc64cart.a
$OPENXDK/bin/i386-pc-xbox-ar cru libc64cart.a actionreplay3.o actionreplay4.o actionreplay.o atomicpower.o c64cart.o \
                                              c64cartmem.o comal80.o crt.o delaep256.o delaep64.o delaep7x8.o \
                                              epyxfastload.o expert.o final.o generic.o ide64.o kcs.o magicformel.o \
                                              mikroass.o retroreplay.o rexep256.o ross.o stardos.o stb.o supergames.o \
                                              supersnapshot.o zaxxon.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libc64cart.a

cd ..
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../drive -I../sid -I../tape -I../vicii -I../arch/sdl/xbox"
for i in c64-cmdline-options.c c64-midi.c c64-resources.c c64-snapshot.c c64.c c64_256k.c c64acia1.c c64bus.c c64cia1.c \
         c64cia2.c c64datasette.c c64drive.c c64export.c c64fastiec.c c64iec.c c64io.c c64keyboard.c c64mem.c \
         c64meminit.c c64memlimit.c c64memrom.c c64memsnapshot.c c64parallel.c c64pla.c c64printer.c c64rom.c \
         c64romset.c c64rsuser.c c64sound.c c64tpi.c c64video.c digimax.c dqbb.c georam.c isepic.c mmc64.c patchrom.c \
         plus256k.c plus60k.c psid.c ramcart.c reloc65.c reu.c tfe.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libc64.a
$OPENXDK/bin/i386-pc-xbox-ar cru libc64.a c64-cmdline-options.o c64-midi.o c64-resources.o c64-snapshot.o c64.o \
                                          c64_256k.o c64acia1.o c64bus.o c64cia1.o c64cia2.o c64datasette.o c64drive.o \
                                          c64export.o c64fastiec.o c64iec.o c64io.o c64keyboard.o c64mem.o c64meminit.o \
                                          c64memlimit.o c64memrom.o c64memsnapshot.o c64parallel.o c64pla.o c64printer.o \
                                          c64rom.o c64romset.o c64rsuser.o c64sound.o c64tpi.o c64video.o digimax.o \
                                          dqbb.o georam.o isepic.o mmc64.o patchrom.o plus256k.o plus60k.o psid.o \
                                          ramcart.o reloc65.o reu.o tfe.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libc64.a
rm -f libc64c128.a
$OPENXDK/bin/i386-pc-xbox-ar cru libc64c128.a c64-midi.o c64acia1.o c64bus.o c64cia2.o c64datasette.o c64export.o \
                                              c64iec.o c64io.o c64keyboard.o c64meminit.o c64memrom.o c64printer.o \
                                              c64pla.o c64parallel.o c64rsuser.o c64sound.o c64tpi.o digimax.o dqbb.o \
                                              georam.o isepic.o mmc64.o patchrom.o ramcart.o reu.o tfe.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libc64c128.a
rm -f libc64c64dtv.a
$OPENXDK/bin/i386-pc-xbox-ar cru libc64c64dtv.a c64bus.o c64drive.o c64fastiec.o c64keyboard.o c64parallel.o \
                                                c64printer.o c64rom.o c64romset.o c64rsuser.o c64video.o patchrom.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libc64c64dtv.a

cd ../c64dtv
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../drive -I../sid -I../tape -I../vicii -I../c64 -I../arch/sdl/xbox"
for i in c64dtvmemsnapshot.c c64dtvmem.c c64dtvmemrom.c c64dtvcpu.c c64dtvblitter.c c64dtvdma.c c64dtvflash.c \
         c64dtv-cmdline-options.c c64dtv-resources.c c64dtv-snapshot.c c64dtv.c c64dtvcia1.c c64dtvcia2.c c64dtviec.c \
         c64dtvmeminit.c c64dtvpla.c c64dtvsound.c flash-trap.c hummeradc.c c64dtvcart.c c64dtvtfe.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libc64dtv.a
$OPENXDK/bin/i386-pc-xbox-ar cru libc64dtv.a c64dtvmemsnapshot.o c64dtvmem.o c64dtvmemrom.o c64dtvcpu.o c64dtvblitter.o \
                                             c64dtvdma.o c64dtvflash.o c64dtv-cmdline-options.o c64dtv-resources.o \
                                             c64dtv-snapshot.o c64dtv.o c64dtvcia1.o c64dtvcia2.o c64dtviec.o \
                                             c64dtvmeminit.o c64dtvpla.o c64dtvsound.o flash-trap.o hummeradc.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libc64dtv.a
rm -f libc64dtvstubs.a
$OPENXDK/bin/i386-pc-xbox-ar cru libc64dtvstubs.a c64dtvcart.o c64dtvtfe.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libc64dtvstubs.a

cd ../c128
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../drive -I../c64 -I../sid -I../tape -I../vicii -I../vdc -I../arch/sdl/xbox"
for i in c128-cmdline-options.c c128-resources.c c128-snapshot.c c128.c c128cia1.c c128cpu.c c128drive.c c128fastiec.c \
         c128mem.c c128meminit.c c128memlimit.c c128memrom.c c128memsnapshot.c c128mmu.c c128rom.c c128romset.c \
         c128video.c daa.c functionrom.c z80.c z80mem.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libc128.a
$OPENXDK/bin/i386-pc-xbox-ar cru libc128.a c128-cmdline-options.o c128-resources.o c128-snapshot.o c128.o c128cia1.o \
                                           c128cpu.o c128drive.o c128fastiec.o c128mem.o c128meminit.o c128memlimit.o \
                                           c128memrom.o c128memsnapshot.o c128mmu.o c128rom.o c128romset.o c128video.o \
                                           daa.o functionrom.o z80.o z80mem.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libc128.a

cd ../vic20
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../drive -I../sid -I../tape -I../raster -I../arch/sdl/xbox"
for i in vic-cmdline-options.c vic-color.c vic-draw.c vic-mem.c vic-resources.c vic-snapshot.c vic.c \
         vic20-cmdline-options.c vic20-midi.c vic20-resources.c vic20-snapshot.c vic20.c vic20bus.c vic20cartridge.c \
         vic20datasette.c vic20drive.c vic20iec.c vic20ieeevia1.c vic20ieeevia2.c vic20mem.c vic20memrom.c \
         vic20memsnapshot.c vic20printer.c vic20rom.c vic20romset.c vic20rsuser.c vic20sound.c vic20via1.c vic20via2.c \
         vic20video.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libvic20.a
$OPENXDK/bin/i386-pc-xbox-ar cru libvic20.a vic-cmdline-options.o vic-color.o vic-draw.o vic-mem.o vic-resources.o \
                                            vic-snapshot.o vic.o vic20-cmdline-options.o vic20-midi.o vic20-resources.o \
                                            vic20-snapshot.o vic20.o vic20bus.o vic20cartridge.o vic20datasette.o \
                                            vic20drive.o vic20iec.o vic20ieeevia1.o vic20ieeevia2.o vic20mem.o \
                                            vic20memrom.o vic20memsnapshot.o vic20printer.o vic20rom.o vic20romset.o \
                                            vic20rsuser.o vic20sound.o vic20via1.o vic20via2.o vic20video.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libvic20.a

cd ../pet
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../crtc -I../drive -I../sid -I../tape -I../arch/sdl/xbox"
for i in pet-cmdline-options.c pet-resources.c pet-snapshot.c pet.c petacia1.c petbus.c petdatasette.c petdrive.c \
         petiec.c petmem.c petmemsnapshot.c petpia1.c petpia2.c petprinter.c petreu.c petrom.c petromset.c petsound.c \
         petvia.c petvideo.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libpet.a
$OPENXDK/bin/i386-pc-xbox-ar cru libpet.a pet-cmdline-options.o pet-resources.o pet-snapshot.o pet.o petacia1.o petbus.o \
                                          petdatasette.o petdrive.o petiec.o petmem.o petmemsnapshot.o petpia1.o \
                                          petpia2.o petprinter.o petreu.o petrom.o petromset.o petsound.o petvia.o \
                                          petvideo.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libpet.a

cd ../plus4
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../drive -I../sid -I../tape -I../raster -I../arch/sdl/xbox"
for i in plus4-cmdline-options.c plus4-resources.c plus4-snapshot.c plus4.c plus4acia.c plus4bus.c plus4cart.c \
         plus4cpu.c plus4datasette.c plus4drive.c plus4iec.c plus4mem.c plus4memcsory256k.c plus4memhannes256k.c \
         plus4memlimit.c plus4memrom.c plus4memsnapshot.c plus4parallel.c plus4pio1.c plus4pio2.c plus4printer.c \
         plus4rom.c plus4romset.c plus4tcbm.c plus4video.c ted-badline.c ted-cmdline-options.c ted-color.c ted-draw.c \
         ted-fetch.c ted-irq.c ted-mem.c ted-resources.c ted-snapshot.c ted-sound.c ted-timer.c ted-timing.c ted.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libplus4.a
$OPENXDK/bin/i386-pc-xbox-ar cru libplus4.a plus4-cmdline-options.o plus4-resources.o plus4-snapshot.o plus4.o \
                                            plus4acia.o plus4bus.o plus4cart.o plus4cpu.o plus4datasette.o plus4drive.o \
                                            plus4iec.o plus4mem.o plus4memcsory256k.o plus4memhannes256k.o \
                                            plus4memlimit.o plus4memrom.o plus4memsnapshot.o plus4parallel.o plus4pio1.o \
                                            plus4pio2.o plus4printer.o plus4rom.o plus4romset.o plus4tcbm.o plus4video.o \
                                            ted-badline.o ted-cmdline-options.o ted-color.o ted-draw.o ted-fetch.o \
                                            ted-irq.o ted-mem.o ted-resources.o ted-snapshot.o ted-sound.o ted-timer.o \
                                            ted-timing.o ted.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libplus4.a

cd ../cbm2
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I.. -I../arch/sdl -I../sid -I../crtc -I../vicii -I../drive -I../tape -I../arch/sdl/xbox"
for i in cbm2-cmdline-options.c cbm2-resources.c cbm2-snapshot.c cbm2.c cbm2acia1.c cbm2bus.c cbm2cia1.c cbm2cpu.c \
         cbm2datasette.c cbm2drive.c cbm2iec.c cbm2mem.c cbm2memsnapshot.c cbm2printer.c cbm2rom.c cbm2romset.c \
         cbm2sound.c cbm2tpi1.c cbm2tpi2.c cbm2video.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libcbm2.a
$OPENXDK/bin/i386-pc-xbox-ar cru libcbm2.a cbm2-cmdline-options.o cbm2-resources.o cbm2-snapshot.o cbm2.o cbm2acia1.o \
                                           cbm2bus.o cbm2cia1.o cbm2cpu.o cbm2datasette.o cbm2drive.o cbm2iec.o \
                                           cbm2mem.o cbm2memsnapshot.o cbm2printer.o cbm2rom.o cbm2romset.o cbm2sound.o \
                                           cbm2tpi1.o cbm2tpi2.o cbm2video.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libcbm2.a

cd ../arch/sdl
VICE_FLAGS="-DHAVE_CONFIG_H -I. -I../.. -I../../drive -I../../c64 -I../../c64dtv -I../../c128 -I../../vic20 -I../../pet -I../../cbm2 -I../../plus4 -I../../raster -I../../sid -Ixbox"
for i in archdep.c blockdev.c console.c fullscreen.c joy.c kbd.c menu_c64cart.c menu_c64dtvhw.c menu_c64hw.c \
         menu_common.c menu_drive.c menu_help.c menu_joystick.c menu_mouse.c menu_plus4cart.c menu_reset.c \
         menu_screenshot.c menu_settings.c menu_sid.c menu_snapshot.c menu_sound.c menu_speed.c menu_tape.c \
         menu_vic20cart.c menu_video.c mousedrv.c sdlmain.c signals.c ui.c uicmdline.c uifilereq.c uihotkey.c uimenu.c \
         uimon.c uimsgbox.c uipoll.c video.c vkbd.c vsidui.c vsyncarch.c x128_ui.c x64_ui.c x64dtv_ui.c xcbm2_ui.c \
         xpet_ui.c xplus4_ui.c xvic_ui.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done
rm -f libarch.a
$OPENXDK/bin/i386-pc-xbox-ar cru libarch.a archdep.o blockdev.o console.o fullscreen.o joy.o kbd.o menu_c64cart.o \
                                           menu_c64dtvhw.o menu_c64hw.o menu_common.o menu_drive.o menu_help.o \
                                           menu_joystick.o menu_mouse.o menu_plus4cart.o menu_reset.o menu_screenshot.o \
                                           menu_settings.o menu_sid.o menu_snapshot.o menu_sound.o menu_speed.o \
                                           menu_tape.o menu_vic20cart.o menu_video.o mousedrv.o sdlmain.o signals.o ui.o \
                                           uicmdline.o uifilereq.o uihotkey.o uimenu.o uimon.o uimsgbox.o uipoll.o \
                                           video.o vkbd.o vsidui.o vsyncarch.o x128_ui.o x64_ui.o x64dtv_ui.o \
                                           xcbm2_ui.o xpet_ui.o xplus4_ui.o xvic_ui.o 
$OPENXDK/bin/i386-pc-xbox-ranlib libarch.a

cd ../..
VICE_FLAGS="-DHAVE_CONFIG_H -I. -Iarch/sdl -Idrive -Ivdrive -Iarch/sdl/xbox"
for i in alarm.c attach.c autostart.c charset.c clkguard.c clipboard.c cmdline.c cbmdos.c cbmimage.c color.c crc32.c \
         datasette.c debug.c dma.c emuid.c event.c findpath.c fliplist.c gcr.c info.c init.c initcmdline.c interrupt.c \
         ioutil.c joystick.c kbdbuf.c keyboard.c lib.c libm_math.c log.c machine-bus.c machine.c main.c network.c \
         palette.c ram.c rawfile.c resources.c romset.c screenshot.c snapshot.c sound.c sysfile.c translate.c traps.c \
         util.c vsync.c zfile.c zipcode.c maincpu.c mouse.c midi.c ps2mouse.c
do
    echo Compiling $i.
    $OPENXDK/bin/i386-pc-xbox-gcc $VICE_FLAGS $CC_COMPILE_FLAGS $i
done

$OPENXDK/bin/i386-pc-xbox-gcc -o x64.exe alarm.o attach.o autostart.o charset.o clkguard.o clipboard.o cmdline.o cbmdos.o \
                                         cbmimage.o color.o crc32.o datasette.o debug.o dma.o emuid.o event.o findpath.o \
                                         fliplist.o gcr.o info.o init.o initcmdline.o interrupt.o ioutil.o joystick.o \
                                         kbdbuf.o keyboard.o lib.o libm_math.o log.o machine-bus.o machine.o main.o \
                                         network.o palette.o ram.o rawfile.o resources.o romset.o screenshot.o snapshot.o \
                                         sound.o sysfile.o translate.o traps.o util.o vsync.o zfile.o zipcode.o maincpu.o \
                                         mouse.o midi.o c64/libc64.a c64/cart/libc64cart.a drive/iec/libdriveiec.a \
                                         drive/iecieee/libdriveiecieee.a drive/iec/c64exp/libdriveiecc64exp.a \
                                         drive/ieee/libdriveieee.a drive/libdrive.a iecbus/libiecbus.a \
                                         parallel/libparallel.a vdrive/libvdrive.a sid/libsid.a monitor/libmonitor.a \
                                         sounddrv/libsounddrv.a gfxoutputdrv/libgfxoutputdrv.a printerdrv/libprinterdrv.a \
                                         rs232drv/librs232drv.a diskimage/libdiskimage.a fsdevice/libfsdevice.a \
                                         tape/libtape.a imagecontents/libimagecontents.a fileio/libfileio.a \
                                         serial/libserial.a core/libcore.a vicii/libvicii.a raster/libraster.a \
                                         video/libvideo.a arch/sdl/libarch.a lib/lpng/libpng.a $LINK_FLAGS

$OPENXDK/bin/cxbe -TITLE:'x64' -DUMPINFO:"x64.cxbe" -OUT:"x64.xbe" x64.exe > /dev/null

$OPENXDK/bin/i386-pc-xbox-gcc -o x128.exe alarm.o attach.o autostart.o charset.o clkguard.o clipboard.o cmdline.o \
                                          cbmdos.o cbmimage.o color.o crc32.o datasette.o debug.o dma.o emuid.o event.o \
                                          findpath.o fliplist.o gcr.o info.o init.o initcmdline.o interrupt.o ioutil.o \
                                          joystick.o kbdbuf.o keyboard.o lib.o libm_math.o log.o machine-bus.o machine.o \
                                          main.o network.o palette.o ram.o rawfile.o resources.o romset.o screenshot.o \
                                          snapshot.o sound.o sysfile.o translate.o traps.o util.o vsync.o zfile.o \
                                          zipcode.o mouse.o midi.o c128/libc128.a c64/libc64c128.a c64/cart/libc64cart.a \
                                          drive/iec/libdriveiec.a drive/iec128dcr/libdriveiec128dcr.a \
                                          drive/iecieee/libdriveiecieee.a drive/iec/c64exp/libdriveiecc64exp.a \
                                          drive/ieee/libdriveieee.a drive/libdrive.a iecbus/libiecbus.a \
                                          parallel/libparallel.a vdrive/libvdrive.a sid/libsid.a monitor/libmonitor.a \
                                          sounddrv/libsounddrv.a gfxoutputdrv/libgfxoutputdrv.a \
                                          printerdrv/libprinterdrv.a rs232drv/librs232drv.a diskimage/libdiskimage.a \
                                          fsdevice/libfsdevice.a tape/libtape.a imagecontents/libimagecontents.a \
                                          fileio/libfileio.a serial/libserial.a core/libcore.a vicii/libvicii.a \
                                          vdc/libvdc.a raster/libraster.a video/libvideo.a arch/sdl/libarch.a \
                                          lib/lpng/libpng.a $LINK_FLAGS

$OPENXDK/bin/cxbe -TITLE:'x128' -DUMPINFO:"x128.cxbe" -OUT:"x128.xbe" x128.exe > /dev/null

$OPENXDK/bin/i386-pc-xbox-gcc -o x64dtv.exe alarm.o attach.o autostart.o charset.o clkguard.o clipboard.o cmdline.o \
                                            cbmdos.o cbmimage.o color.o crc32.o datasette.o debug.o dma.o emuid.o \
                                            event.o findpath.o fliplist.o gcr.o info.o init.o initcmdline.o interrupt.o \
                                            ioutil.o joystick.o kbdbuf.o keyboard.o lib.o libm_math.o log.o machine-bus.o \
                                            machine.o main.o network.o palette.o ram.o rawfile.o resources.o romset.o \
                                            screenshot.o snapshot.o sound.o sysfile.o translate.o traps.o util.o vsync.o \
                                            zfile.o zipcode.o ps2mouse.o c64dtv/libc64dtv.a c64/libc64c64dtv.a \
                                            drive/iec/libdriveiec.a drive/iecieee/libdriveiecieee.a \
                                            drive/iec/c64exp/libdriveiecc64exp.a drive/ieee/libdriveieee.a \
                                            drive/libdrive.a iecbus/libiecbus.a parallel/libparallel.a vdrive/libvdrive.a \
                                            sid/libsid.a monitor/libmonitor.a sounddrv/libsounddrv.a \
                                            gfxoutputdrv/libgfxoutputdrv.a printerdrv/libprinterdrv.a \
                                            rs232drv/librs232drv.a diskimage/libdiskimage.a fsdevice/libfsdevice.a \
                                            tape/libtape.a imagecontents/libimagecontents.a fileio/libfileio.a \
                                            serial/libserial.a core/libcore.a vicii/libviciidtv.a raster/libraster.a \
                                            video/libvideo.a arch/sdl/libarch.a lib/lpng/libpng.a c64dtv/libc64dtvstubs.a \
                                            $LINK_FLAGS

$OPENXDK/bin/cxbe -TITLE:'x64dtv' -DUMPINFO:"x64dtv.cxbe" -OUT:"x64dtv.xbe" x64dtv.exe > /dev/null

$OPENXDK/bin/i386-pc-xbox-gcc -o xvic.exe alarm.o attach.o autostart.o charset.o clkguard.o clipboard.o cmdline.o \
                                          cbmdos.o cbmimage.o color.o crc32.o datasette.o debug.o dma.o emuid.o event.o \
                                          findpath.o fliplist.o gcr.o info.o init.o initcmdline.o interrupt.o ioutil.o \
                                          joystick.o kbdbuf.o keyboard.o lib.o libm_math.o log.o machine-bus.o machine.o \
                                          main.o network.o palette.o ram.o rawfile.o resources.o romset.o screenshot.o \
                                          snapshot.o sound.o sysfile.o translate.o traps.o util.o vsync.o zfile.o \
                                          zipcode.o maincpu.o mouse.o midi.o vic20/libvic20.a drive/iec/libdriveiec.a \
                                          drive/iecieee/libdriveiecieee.a drive/ieee/libdriveieee.a drive/libdrive.a \
                                          iecbus/libiecbus.a parallel/libparallel.a vdrive/libvdrive.a \
                                          monitor/libmonitor.a sounddrv/libsounddrv.a gfxoutputdrv/libgfxoutputdrv.a \
                                          printerdrv/libprinterdrv.a rs232drv/librs232drv.a diskimage/libdiskimage.a \
                                          fsdevice/libfsdevice.a tape/libtape.a imagecontents/libimagecontents.a \
                                          fileio/libfileio.a serial/libserial.a core/libcore.a raster/libraster.a \
                                          sid/libsid.a video/libvideo.a arch/sdl/libarch.a lib/lpng/libpng.a $LINK_FLAGS

$OPENXDK/bin/cxbe -TITLE:'xvic' -DUMPINFO:"xvic.cxbe" -OUT:"xvic.xbe" xvic.exe > /dev/null

$OPENXDK/bin/i386-pc-xbox-gcc -o xpet.exe alarm.o attach.o autostart.o charset.o clkguard.o clipboard.o cmdline.o \
                                          cbmdos.o cbmimage.o color.o crc32.o datasette.o debug.o dma.o emuid.o event.o \
                                          findpath.o fliplist.o gcr.o info.o init.o initcmdline.o interrupt.o ioutil.o \
                                          joystick.o kbdbuf.o keyboard.o lib.o libm_math.o log.o machine-bus.o machine.o \
                                          main.o network.o palette.o ram.o rawfile.o resources.o romset.o screenshot.o \
                                          snapshot.o sound.o sysfile.o translate.o traps.o util.o vsync.o zfile.o \
                                          zipcode.o maincpu.o mouse.o pet/libpet.a drive/iecieee/libdriveiecieee.a \
                                          drive/ieee/libdriveieee.a drive/libdrive.a parallel/libparallel.a \
                                          vdrive/libvdrive.a monitor/libmonitor.a sounddrv/libsounddrv.a \
                                          gfxoutputdrv/libgfxoutputdrv.a printerdrv/libprinterdrv.a \
                                          rs232drv/librs232drv.a diskimage/libdiskimage.a fsdevice/libfsdevice.a \
                                          tape/libtape.a imagecontents/libimagecontents.a fileio/libfileio.a \
                                          serial/libserial.a core/libcore.a crtc/libcrtc.a raster/libraster.a \
                                          video/libvideo.a sid/libsid.a arch/sdl/libarch.a lib/lpng/libpng.a $LINK_FLAGS

$OPENXDK/bin/cxbe -TITLE:'xpet' -DUMPINFO:"xpet.cxbe" -OUT:"xpet.xbe" xpet.exe > /dev/null

$OPENXDK/bin/i386-pc-xbox-gcc -o xplus4.exe alarm.o attach.o autostart.o charset.o clkguard.o clipboard.o cmdline.o \
                                            cbmdos.o cbmimage.o color.o crc32.o datasette.o debug.o dma.o emuid.o event.o \
                                            findpath.o fliplist.o gcr.o info.o init.o initcmdline.o interrupt.o ioutil.o \
                                            joystick.o kbdbuf.o keyboard.o lib.o libm_math.o log.o machine-bus.o \
                                            machine.o main.o network.o palette.o ram.o rawfile.o resources.o romset.o \
                                            screenshot.o snapshot.o sound.o sysfile.o translate.o traps.o util.o vsync.o \
                                            zfile.o zipcode.o mouse.o plus4/libplus4.a drive/iec/libdriveiec.a \
                                            drive/iecieee/libdriveiecieee.a drive/iec/plus4exp/libdriveiecplus4exp.a \
                                            drive/tcbm/libdrivetcbm.a drive/libdrive.a iecbus/libiecbus.a \
                                            vdrive/libvdrive.a monitor/libmonitor.a sounddrv/libsounddrv.a \
                                            gfxoutputdrv/libgfxoutputdrv.a printerdrv/libprinterdrv.a \
                                            rs232drv/librs232drv.a diskimage/libdiskimage.a fsdevice/libfsdevice.a \
                                            tape/libtape.a imagecontents/libimagecontents.a fileio/libfileio.a \
                                            serial/libserial.a core/libcore.a sid/libsid.a raster/libraster.a \
                                            video/libvideo.a arch/sdl/libarch.a lib/lpng/libpng.a $LINK_FLAGS

$OPENXDK/bin/cxbe -TITLE:'xplus4' -DUMPINFO:"xplus4.cxbe" -OUT:"xplus4.xbe" xplus4.exe > /dev/null

$OPENXDK/bin/i386-pc-xbox-gcc -o xcbm2.exe alarm.o attach.o autostart.o charset.o clkguard.o clipboard.o cmdline.o \
                                           cbmdos.o cbmimage.o color.o crc32.o datasette.o debug.o dma.o emuid.o event.o \
                                           findpath.o fliplist.o gcr.o info.o init.o initcmdline.o interrupt.o ioutil.o \
                                           joystick.o kbdbuf.o keyboard.o lib.o libm_math.o log.o machine-bus.o machine.o \
                                           main.o network.o palette.o ram.o rawfile.o resources.o romset.o screenshot.o \
                                           snapshot.o sound.o sysfile.o translate.o traps.o util.o vsync.o zfile.o \
                                           zipcode.o mouse.o cbm2/libcbm2.a drive/iecieee/libdriveiecieee.a \
                                           drive/ieee/libdriveieee.a drive/libdrive.a parallel/libparallel.a \
                                           vdrive/libvdrive.a monitor/libmonitor.a sounddrv/libsounddrv.a \
                                           gfxoutputdrv/libgfxoutputdrv.a printerdrv/libprinterdrv.a \
                                           rs232drv/librs232drv.a diskimage/libdiskimage.a fsdevice/libfsdevice.a \
                                           tape/libtape.a imagecontents/libimagecontents.a fileio/libfileio.a \
                                           serial/libserial.a core/libcore.a sid/libsid.a crtc/libcrtc.a vicii/libvicii.a \
                                           raster/libraster.a video/libvideo.a arch/sdl/libarch.a lib/lpng/libpng.a \
                                           $LINK_FLAGS

$OPENXDK/bin/cxbe -TITLE:'xcbm2' -DUMPINFO:"xcbm2.cxbe" -OUT:"xcbm2.xbe" xcbm2.exe > /dev/null

cd ..

VICEVERSION="2.1"

if [ ! -e src/x64.exe -o ! -e src/x64dtv.exe -o ! -e src/x128.exe -o ! -e src/xvic.exe -o ! -e src/xpet.exe -o ! -e src/xplus4.exe -o ! -e src/xcbm2.exe ]
then
  echo Error: executable file\(s\) not found, something went wrong
  exit 1
fi

echo Generating xbox SDL port binary distribution.
rm -f -r SDLVICE-$VICEVERSION-xbox
mkdir SDLVICE-$VICEVERSION-xbox
cp src/x64.xbe src/x64dtv.xbe src/x128.xbe SDLVICE-$VICEVERSION-xbox
cp src/xvic.xbe src/xpet.xbe src/xplus4.xbe SDLVICE-$VICEVERSION-xbox
cp src/xcbm2.xbe SDLVICE-$VICEVERSION-xbox
cp -a data/C128 data/C64 SDLVICE-$VICEVERSION-xbox
cp -a data/C64DTV data/CBM-II SDLVICE-$VICEVERSION-xbox
cp -a data/DRIVES data/PET SDLVICE-$VICEVERSION-xbox
cp -a data/PLUS4 data/PRINTER SDLVICE-$VICEVERSION-xbox
cp -a data/VIC20 SDLVICE-$VICEVERSION-xbox
cp -a data/fonts SDLVICE-$VICEVERSION-xbox
cp -a doc/html SDLVICE-$VICEVERSION-xbox
cp FEEDBACK README SDLVICE-$VICEVERSION-xbox
rm `find SDLVICE-$VICEVERSION-xbox -name "Makefile*"`
rm `find SDLVICE-$VICEVERSION-xbox -name "amiga_*.vkm"`
rm `find SDLVICE-$VICEVERSION-xbox -name "dos_*.vkm"`
rm `find SDLVICE-$VICEVERSION-xbox -name "os2*.vkm"`
rm `find SDLVICE-$VICEVERSION-xbox -name "osx*.vkm"`
rm `find SDLVICE-$VICEVERSION-xbox -name "beos_*.vkm"`
rm `find SDLVICE-$VICEVERSION-xbox -name "x11_*.vkm"`
rm `find SDLVICE-$VICEVERSION-xbox -name "RO*.vkm"`
rm `find SDLVICE-$VICEVERSION-xbox -name "win*.vkm"`
rm SDLVICE-$VICEVERSION-xbox/html/texi2html
if test x"$ZIP" = "x"; then
  zip -r -9 -q SDLVICE-$VICEVERSION-xbox.zip SDLVICE-$VICEVERSION-xbox
else
  $ZIP SDLVICE-$VICEVERSION-xbox.zip SDLVICE-$VICEVERSION-xbox
fi
echo xbox SDL port binary distribution archive generated as SDLVICE-$VICEVERSION-xbox.zip
echo and
echo xbox SDL port binary distribution directory generated as SDLVICE-$VICEVERSION-xbox
