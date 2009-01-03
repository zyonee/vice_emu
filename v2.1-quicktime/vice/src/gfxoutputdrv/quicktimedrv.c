/*
 * quicktimedrv.c - Movie driver using Apple QuickTime.
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <stdio.h>
#include <string.h>

#include <QuickTime/Movies.h>
#include <QuickTime/QuickTimeComponents.h>

#include "gfxoutput.h"
#include "screenshot.h"
#include "palette.h"
#include "log.h"

static  Movie           movie=NULL;
static  DataHandler     dataHandler=NULL;
static  Track           track=NULL;
static  Media           media=NULL;
static  ICMCompressionSessionRef    compressionSession = NULL;
static  TimeValue64     timestamp=0;
static  TimeValue64     divider=0;
static  CVPixelBufferRef pixelBuffer=NULL;
static  int video_width, video_height, video_xoff, video_yoff;

static OSStatus FrameOutputCallback(void* encodedFrameOutputRefCon, 
    ICMCompressionSessionRef session, OSStatus error, ICMEncodedFrameRef frame,
    void* reserved)
{
    if(error)
        log_debug("quicktime: error encoding frame!");
    else {
        if(ICMEncodedFrameGetDecodeDuration(frame) > 0) {
             //  Adds sample data and description from an encoded frame to a media.
             OSErr theError = AddMediaSampleFromEncodedFrame(media, frame, NULL);
             if(theError)
                 log_debug("quicktime: error adding media sample!");
         }
    }
    return error;
}

// ----- gfxoutputdrv interface ---------------------------------------------

static int quicktimedrv_open(screenshot_t *screenshot, const char *filename)
{
    /* UNUSED */
    return 0;
}

static int quicktimedrv_save(screenshot_t *screenshot, const char *filename)
{
    // align and center video
    video_width  = screenshot->width;
    video_height = screenshot->height;
    video_width  = (video_width + 15)  & ~15;
    video_height = (video_height + 15) & ~15;
    video_xoff   = (video_width - screenshot->width) >> 1;
    video_yoff   = (video_height - screenshot->height) >> 1;
    
    // create cfstring from filename    
    CFStringRef path = CFStringCreateWithCString(NULL,filename, kCFStringEncodingUTF8);
    if(path==NULL) {
        log_debug("quicktime: error creating CFString!");
        return -1;
    }
    
    // create data reference
    Handle dataRef;
    OSType dataRefType;
    OSErr theError = QTNewDataReferenceFromFullPathCFString(
        path, kQTNativeDefaultPathStyle, 0, &dataRef, &dataRefType);
    if(theError) {
        log_debug("quicktime: error creating data reference for '%s'",filename);
        return -1;
    }
    
    // Create a movie for this file (data ref)
    theError = CreateMovieStorage(
        dataRef, dataRefType, 'TVOD', smCurrentScript, createMovieFileDeleteCurFile, 
        &dataHandler, &movie);
    if(theError) {
        log_debug("quicktime: error creating movie storage for '%s'",filename);
        return -1;
    }

    // dispose of the data reference handle - we no longer need it
    DisposeHandle(dataRef);
    
    //Add track
    track = NewMovieTrack(movie, video_width << 16, video_height << 16, 0);
    theError = GetMoviesError();
    if(theError) {
        log_debug("quicktime: error creating movie track");
        return -1;
    }

    // define time scale and host clock divider
    TimeScale timeScale = 1000000;
    divider = (TimeScale)CVGetHostClockFrequency() / timeScale;

    //Create track media
    media = NewTrackMedia(track, VideoMediaType, timeScale, 0, 0);
    theError = GetMoviesError();
    if(theError) {
        log_debug("quicktime: error creating track media!");
        return -1;
    }

    //Prepare media for editing
    theError = BeginMediaEdits(media);
    if(theError) {
        log_debug("quicktime: error beginning media edits!");
        return -1;
    }

    // ----- Setup Codec -----

#if 0
    // Component setup
    ComponentInstance component = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubType);
    if(component == NULL) {
        log_debug("QT: no comonent");
        return -1;
    }
    
    // set preferences
    long flags = scAllowEncodingWithCompressionSession;
    SCSetInfo(component, scPreferenceFlagsType, &flags);
	
#if 0
    // open dialog to adjust settings
    theError = SCRequestSequenceSettings(component);
    if(theError) {
        log_debug("QT: request sequence settings");
        return -1;
    }
#endif

    // Copy settings from compression dialog
    ICMCompressionSessionOptionsRef options;
    theError = SCCopyCompressionSessionOptions(component, &options);
    if(theError) {
        log_debug("QT: copy compression session");
        return -1;
    }

    // Get codec
    SCSpatialSettings spatialSettings;
    theError = SCGetInfo(component, scSpatialSettingsType, &spatialSettings);
    CodecType codec = spatialSettings.codecType;
    
    log_debug("codec %s\n",(const char *)&codec);
    
    // close component
    CloseComponent(component);
#else
    CodecType codec = kPNGCodecType;
#endif

    // Create compression session
    ICMEncodedFrameOutputRecord record = { FrameOutputCallback, NULL, NULL };
    theError = ICMCompressionSessionCreate(kCFAllocatorDefault, 
        video_width, video_height, codec, timeScale, NULL /*options*/, NULL, 
        &record, &compressionSession);
    if(theError) {
        log_debug("quicktime: error creating compression session!");
        return -1;
    }

    // ----- PixelBuffer -----
    theError = CVPixelBufferCreate(NULL,video_width,video_height,
        kCVPixelFormatType_24RGB, NULL, &pixelBuffer);
    if(theError) {
        log_debug("quicktime: error creating pixel buffer!");
        return -1;
    }
    CVPixelBufferRetain(pixelBuffer);

    // set initial time stamp
    timestamp = CVGetCurrentHostTime() / divider;
    return 0;
}

static int quicktimedrv_record(screenshot_t *screenshot)
{
    OSErr theError;

    // lock buffer
    theError = CVPixelBufferLockBaseAddress(pixelBuffer,0);
    if(theError) {
        log_debug("quicktime: error locking pixel buffer!");
        return -1;
    }
    
    // fill frame
    unsigned char *buffer = (unsigned char *)CVPixelBufferGetBaseAddress(pixelBuffer);
    size_t bytesPerRow    = CVPixelBufferGetBytesPerRow(pixelBuffer);

    unsigned int line_size = screenshot->draw_buffer_line_size;
    int h = screenshot->height;
    int w = screenshot->width;
    int xoff = screenshot->x_offset;
    int yoff = screenshot->y_offset;
    BYTE *srcBuffer = screenshot->draw_buffer;

    // move to last line in tgt buffer and to first in source
    buffer += (video_yoff) * bytesPerRow + video_xoff * 3;
    srcBuffer += yoff * line_size + xoff;
    
    int x,y;
    for(y = 0; y < h; y++) {
        int pix = 0;
        for(x=0; x < w; x++) {
            BYTE val = srcBuffer[x];
            buffer[pix++] = screenshot->palette->entries[val].red;
            buffer[pix++] = screenshot->palette->entries[val].green;
            buffer[pix++] = screenshot->palette->entries[val].blue;
        }
        buffer += bytesPerRow;
        srcBuffer += line_size;
    }

    // unlock buffer
    theError = CVPixelBufferUnlockBaseAddress(pixelBuffer,0);
    if(theError) {
        log_debug("quicktime: error unlocking pixel buffer!");
        return -1;
    }    
    
    TimeValue64 next = CVGetCurrentHostTime() / divider;
    TimeValue64 duration = next - timestamp;
    timestamp = next;
    
    // encode frame
    theError = ICMCompressionSessionEncodeFrame(compressionSession,
        pixelBuffer,
        timestamp, duration,  
        kICMValidTime_DisplayTimeStampIsValid |
        kICMValidTime_DisplayDurationIsValid, 
        NULL, NULL, (void *)NULL);
    if(theError) {
         log_debug("quicktime: error encoding frame!");
         return -1;
    }
    
    return 0;
}

static int quicktimedrv_close(screenshot_t *screenshot)
{
    // ----- PixelBuffer -----
    CVPixelBufferRelease(pixelBuffer);

    // ----- Codec -----
    
    OSErr theError = ICMCompressionSessionCompleteFrames(compressionSession, true, 0, 0);
    if(theError)
        log_debug("quicktime: error completing frames!");
        
    ICMCompressionSessionRelease(compressionSession);
	
    // ----- Movie -----

    //End media editing
    theError = EndMediaEdits(media);
    if(theError)
        log_debug("quicktime: error ending media edits");

    theError = ExtendMediaDecodeDurationToDisplayEndTime(media, NULL);
    if(theError)
        log_debug("quicktime: error setting decode duration!");

    //Add media to track
    theError = InsertMediaIntoTrack(track, 0, 0, GetMediaDisplayDuration(media), fixed1);
    if(theError)
        log_debug("quicktime: error inserting media into track!");

    //Write movie
    theError = AddMovieToStorage(movie, dataHandler);
    if(theError)
        log_debug("quicktime: error adding movie to storage!");

    //Close movie file
    if(dataHandler) {
        CloseMovieStorage(dataHandler);
    }
    if(movie) {
        DisposeMovie(movie);
    }

    movie=NULL;
    dataHandler=NULL;
    track=NULL;
    media=NULL;

    return 0;
}

static int quicktimedrv_write(screenshot_t *screenshot)
{
    /* UNUSED */
    return 0;
}


static gfxoutputdrv_t quicktime_drv = {
    "QuickTime",
    "QuickTime",
    "mov",
    quicktimedrv_open,
    quicktimedrv_close,
    quicktimedrv_write,
    quicktimedrv_save,
    quicktimedrv_record
#ifdef FEATURE_CPUMEMHISTORY
    ,NULL
#endif
};

void gfxoutput_init_quicktime(void)
{
    /* init quicktime */
    OSErr error = EnterMoviesOnThread(0);
    if(error != noErr) {
        log_debug("quicktime: error initializing!");
        return;
    }
  
    gfxoutput_register(&quicktime_drv);
}

