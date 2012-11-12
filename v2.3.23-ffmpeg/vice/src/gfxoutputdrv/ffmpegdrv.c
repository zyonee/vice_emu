/*
 * ffmpegdrv.c - Movie driver using FFMPEG library and screenshot API.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include "archdep.h"
#include "cmdline.h"
#include "ffmpegdrv.h"
#include "ffmpeglib.h"
#include "gfxoutput.h"
#include "lib.h"
#include "log.h"
#include "palette.h"
#include "resources.h"
#include "screenshot.h"
#include "translate.h"
#include "uiapi.h"
#include "util.h"
#include "vsync.h"
#include "../sounddrv/soundmovie.h"

static gfxoutputdrv_codec_t avi_audio_codeclist[] = { 
    { AV_CODEC_ID_MP2, "MP2" },
    { AV_CODEC_ID_MP3, "MP3" },
    { AV_CODEC_ID_FLAC, "FLAC" },
    { AV_CODEC_ID_PCM_S16LE, "PCM uncompressed" },
    { 0, NULL }
};

static gfxoutputdrv_codec_t mp4_audio_codeclist[] = { 
    { AV_CODEC_ID_MP3, "MP3" },
    { AV_CODEC_ID_AAC, "AAC" },
    { 0, NULL }
};

static gfxoutputdrv_codec_t avi_video_codeclist[] = { 
    { AV_CODEC_ID_MPEG4, "MPEG4 (DivX)" },
    { AV_CODEC_ID_MPEG1VIDEO, "MPEG1" },
    { AV_CODEC_ID_FFV1, "FFV1 (lossless)" },
    { AV_CODEC_ID_H264, "H264" },
    { AV_CODEC_ID_THEORA, "Theora" },
    { 0, NULL }
};

static gfxoutputdrv_codec_t mp4_video_codeclist[] = { 
    { AV_CODEC_ID_H264, "H264" },
    { 0, NULL }
};

static gfxoutputdrv_codec_t ogg_audio_codeclist[] = { 
    { AV_CODEC_ID_FLAC, "FLAC" },
    { 0, NULL }
};

static gfxoutputdrv_codec_t ogg_video_codeclist[] = { 
    { AV_CODEC_ID_THEORA, "Theora" },
    { 0, NULL }
};

gfxoutputdrv_format_t ffmpegdrv_formatlist[] =
{
    { "avi", avi_audio_codeclist, avi_video_codeclist },
    { "mp4", mp4_audio_codeclist, mp4_video_codeclist },
    { "ogg", ogg_audio_codeclist, ogg_video_codeclist },
    { "wav", NULL, NULL },
    { "mp3", NULL, NULL },
    { "mp2", NULL, NULL },
    { NULL, NULL, NULL }
};

/* general */
static ffmpeglib_t ffmpeglib;
static AVFormatContext *ffmpegdrv_oc;
static AVOutputFormat *ffmpegdrv_fmt;
static int file_init_done;

/* audio */
static AVStream *audio_st;
static AVCodec *audio_codec;
static AVFrame *audio_frame;
static soundmovie_buffer_t ffmpegdrv_audio_in;
static int audio_init_done;
static int audio_is_open;
//static unsigned char *audio_outbuf;
//static int audio_outbuf_size;
static double audio_pts;

/* video */
static AVStream *video_st;
static AVCodec *video_codec;
static AVFrame *video_frame;
static int video_init_done;
static int video_is_open;
static AVPicture src_picture, dst_picture;
static unsigned char *video_outbuf;
static int video_outbuf_size;
static int video_width, video_height;
static double video_pts;
static unsigned int framecounter;
#ifdef HAVE_FFMPEG_SWSCALE
static struct SwsContext *sws_ctx;
#endif

/* resources */
static char *ffmpeg_format = NULL;
static int format_index;
static int audio_bitrate;
static int video_bitrate;
static int audio_codec_id;
static int video_codec_id;
static int video_halve_framerate;

static int ffmpegdrv_init_file(void);

static int set_format(const char *val, void *param)
{
    int i;

    format_index = -1;
    util_string_set(&ffmpeg_format, val);
    for (i = 0; ffmpegdrv_formatlist[i].name != NULL; i++) {
        if (strcmp(ffmpeg_format, ffmpegdrv_formatlist[i].name) == 0) {
            format_index = i;
        }
    }

    if (format_index < 0) {
        return -1;
    } else {
        return 0;
    }
}

static int set_audio_bitrate(int val, void *param)
{
    audio_bitrate = (CLOCK)val;

    if ((audio_bitrate < VICE_FFMPEG_AUDIO_RATE_MIN)
     || (audio_bitrate > VICE_FFMPEG_AUDIO_RATE_MAX)) {
        audio_bitrate = VICE_FFMPEG_AUDIO_RATE_DEFAULT;
    }
    return 0;
}

static int set_video_bitrate(int val, void *param)
{
    video_bitrate = (CLOCK)val;

    if ((video_bitrate < VICE_FFMPEG_VIDEO_RATE_MIN)
     || (video_bitrate > VICE_FFMPEG_VIDEO_RATE_MAX)) {
        video_bitrate = VICE_FFMPEG_VIDEO_RATE_DEFAULT;
    }
    return 0;
}

static int set_audio_codec(int val, void *param)
{
    audio_codec_id = val;
    return 0;
}

static int set_video_codec(int val, void *param)
{
    video_codec_id = val;
    return 0;
}

static int set_video_halve_framerate(int val, void *param)
{
    if (video_halve_framerate != val && screenshot_is_recording()) {
        ui_error("Can't change framerate while recording. Try again later.");
        return 0;
    }
    video_halve_framerate = val;
    return 0;
}

/*---------- Resources ------------------------------------------------*/

static const resource_string_t resources_string[] = {
    { "FFMPEGFormat", "avi", RES_EVENT_NO, NULL,
      &ffmpeg_format, set_format, NULL },
    { NULL }
};

static const resource_int_t resources_int[] = {
    { "FFMPEGAudioBitrate", VICE_FFMPEG_AUDIO_RATE_DEFAULT,
      RES_EVENT_NO, NULL,
      &audio_bitrate, set_audio_bitrate, NULL },
    { "FFMPEGVideoBitrate", VICE_FFMPEG_VIDEO_RATE_DEFAULT,
      RES_EVENT_NO, NULL,
      &video_bitrate, set_video_bitrate, NULL },
    { "FFMPEGAudioCodec", AV_CODEC_ID_MP3, RES_EVENT_NO, NULL,
      &audio_codec_id, set_audio_codec, NULL },
    { "FFMPEGVideoCodec", AV_CODEC_ID_MPEG4, RES_EVENT_NO, NULL,
      &video_codec_id, set_video_codec, NULL },
    { "FFMPEGVideoHalveFramerate", 0, RES_EVENT_NO, NULL,
      &video_halve_framerate, set_video_halve_framerate, NULL },
    { NULL }
};

static int ffmpegdrv_resources_init(void)
{
    if (resources_register_string(resources_string) < 0)
        return -1;

    return resources_register_int(resources_int);
}
/*---------------------------------------------------------------------*/


/*---------- Commandline options --------------------------------------*/

static const cmdline_option_t cmdline_options[] = {
    { "-ffmpegaudiobitrate", SET_RESOURCE, 1,
      NULL, NULL, "FFMPEGAudioBitrate", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_VALUE, IDCLS_SET_AUDIO_STREAM_BITRATE,
      NULL, NULL },
    { "-ffmpegvideobitrate", SET_RESOURCE, 1,
      NULL, NULL, "FFMPEGVideoBitrate", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_VALUE, IDCLS_SET_VIDEO_STREAM_BITRATE,
      NULL, NULL },
    { NULL }
};

static int ffmpegdrv_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/*---------------------------------------------------------------------*/



/*-----------------------*/
/* audio stream encoding */
/*-----------------------*/
static int ffmpegdrv_open_audio(AVFormatContext *oc, AVStream *st)
{
    AVCodecContext *c;
    AVCodec *codec;

    c = st->codec;

    /* find the audio encoder */
    codec = (*ffmpeglib.p_avcodec_find_encoder)(c->codec_id);
    if (!codec) {
        log_debug("ffmpegdrv: audio codec not found");
        return -1;
    }

    /* open it */
    if ((*ffmpeglib.p_avcodec_open2)(c, codec, NULL) < 0) {
        log_debug("ffmpegdrv: could not open audio codec");
        return -1;
    }
    
    if (c->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
        ffmpegdrv_audio_in.size = 10000;
    else
        ffmpegdrv_audio_in.size = c->frame_size;

    ffmpegdrv_audio_in.buffer = lib_malloc(ffmpegdrv_audio_in.size
                                * sizeof(SWORD) * c->channels);

    audio_is_open = 1;

    return 0;
}


static void ffmpegdrv_close_audio(void)
{
    if (audio_st == NULL)
        return;

    if (audio_is_open) {
        (*ffmpeglib.p_avcodec_close)(audio_st->codec);
        lib_free(ffmpegdrv_audio_in.buffer);
        ffmpegdrv_audio_in.buffer = NULL;
        ffmpegdrv_audio_in.size = 0;
    }

    audio_is_open = 0;
}


static int ffmpegmovie_init_audio(int speed, int channels,
                                 soundmovie_buffer_t ** audio_in)
{
    AVCodecContext *c;
    AVStream *st;

    if (ffmpegdrv_oc == NULL || ffmpegdrv_fmt == NULL)
        return -1;

    audio_init_done = 1;

    if (ffmpegdrv_fmt->audio_codec == AV_CODEC_ID_NONE)
        return -1;

    *audio_in = &ffmpegdrv_audio_in;
    
    (*audio_in)->size = 0; /* not allocated yet */
    (*audio_in)->used = 0;

    st = (*ffmpeglib.p_avformat_new_stream)(ffmpegdrv_oc, audio_codec);
    if (!st) {
        log_debug("ffmpegdrv: Could not alloc audio stream\n");
        return -1;
    }

    c = st->codec;
    c->codec_id = ffmpegdrv_fmt->audio_codec;
    c->codec_type = AVMEDIA_TYPE_AUDIO;
    c->sample_fmt = AV_SAMPLE_FMT_S16;

    /* put sample parameters */
    c->bit_rate = audio_bitrate;
    c->sample_rate = speed;
    c->channels = channels;

    /* some formats want stream headers to be separate */
    if (ffmpegdrv_oc->oformat->flags & AVFMT_GLOBALHEADER) {
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    audio_st = st;
    audio_pts = 0;

    if (video_init_done)
        ffmpegdrv_init_file();

    return 0;
}


/* triggered by soundffmpegaudio->write */
static int ffmpegmovie_encode_audio(soundmovie_buffer_t *audio_in)
{
    int got_packet;
    AVPacket pkt = { 0 };
    AVCodecContext *c;
    AVFrame *frame;

    if (!audio_st) {
        return 0;
    }
        
    if (video_st && video_pts < audio_pts) {
        log_error(LOG_DEFAULT, "audio frame dropped");

        /* drop this frame */
        return 0;
    }

    frame = (*ffmpeglib.p_avcodec_alloc_frame)();
    (*ffmpeglib.p_av_init_packet)(&pkt);
    c = audio_st->codec;
    frame->nb_samples = ffmpegdrv_audio_in.size;
    (*ffmpeglib.p_avcodec_fill_audio_frame)(frame, c->channels, c->sample_fmt,
                                            (uint8_t *)audio_in->buffer,
                                            audio_in->used * sizeof(SWORD)
                                            * c->channels, 1);

    if ((*ffmpeglib.p_avcodec_encode_audio2)(c, &pkt, frame, &got_packet) != 0) {
        log_debug("ffmpegdrv_encode_audio: Error while encoding audio frame");
    }
    if (got_packet) {

#if 0
        pkt.size = (*ffmpeglib.p_avcodec_encode_audio2)(c, 
                        audio_outbuf, audio_outbuf_size, audio_in->buffer);
        pkt.pts = c->coded_frame->pts;
        pkt.flags |= AV_PKT_FLAG_KEY;
        pkt.stream_index = audio_st->index;
        pkt.data = audio_outbuf;
#endif
        pkt.stream_index = audio_st->index;

        if ((*ffmpeglib.p_av_interleaved_write_frame)(ffmpegdrv_oc, &pkt) != 0)
        {
            log_debug("ffmpegdrv_encode_audio: Error while writing audio frame");
        }

        audio_pts = (double)audio_st->pts.val * audio_st->time_base.num 
                        / audio_st->time_base.den;
        log_error(LOG_DEFAULT, "audio_pts: %f", audio_pts);
    }

    (*ffmpeglib.p_avcodec_free_frame)(&frame);

    return 0;
}

static void ffmpegmovie_close(void)
{
    /* just stop the whole recording */
    screenshot_stop_recording();
}

static soundmovie_funcs_t ffmpegdrv_soundmovie_funcs = {
    ffmpegmovie_init_audio,
    ffmpegmovie_encode_audio,
    ffmpegmovie_close
};

/*-----------------------*/
/* video stream encoding */
/*-----------------------*/
static int ffmpegdrv_fill_rgb_image(screenshot_t *screenshot, AVFrame *pic)
{ 
    int x, y;
    int colnum;
    int bufferoffset;
    int x_dim = screenshot->width;
    int y_dim = screenshot->height;
    int pix;

    /* center the screenshot in the video */
    bufferoffset = screenshot->x_offset 
                    + screenshot->y_offset * screenshot->draw_buffer_line_size;

    pix = 3 * ((video_width - x_dim) / 2 + (video_height - y_dim) / 2 * video_width);

    for (y = 0; y < y_dim; y++) {
        for (x=0; x < x_dim; x++) {
            colnum = screenshot->draw_buffer[bufferoffset + x];
            pic->data[0][pix++] = screenshot->palette->entries[colnum].red;
            pic->data[0][pix++] = screenshot->palette->entries[colnum].green;
            pic->data[0][pix++] = screenshot->palette->entries[colnum].blue;
        }
        pix += (3 * (video_width - x_dim));

        bufferoffset += screenshot->draw_buffer_line_size;
    }

    return 0;
}


static AVFrame* ffmpegdrv_alloc_picture(int pix_fmt, int width, int height)
{
    AVFrame *picture;
    unsigned char *picture_buf;
    int size;
    
    picture = lib_malloc(sizeof(AVFrame));
    memset(picture, 0, sizeof(AVFrame));

    picture->pts = AV_NOPTS_VALUE;

    size = (*ffmpeglib.p_avpicture_get_size)(pix_fmt, width, height);
    picture_buf = lib_malloc(size);
    memset(picture_buf, 0, size);
    if (!picture_buf) {
        lib_free(picture);
        return NULL;
    }
    (*ffmpeglib.p_avpicture_fill)((AVPicture *)picture, picture_buf, 
                   pix_fmt, width, height);

    return picture;
}


static int ffmpegdrv_open_video(AVFormatContext *oc, AVStream *st)
{
    AVCodecContext *c;

    c = st->codec;

#if 0
    /* find the video encoder */
    codec = (*ffmpeglib.p_avcodec_find_encoder)(c->codec_id);
    if (!codec) {
        log_debug("ffmpegdrv: video codec not found");
        return -1;
    }
#endif
    /* open the codec */
    if ((*ffmpeglib.p_avcodec_open2)(c, video_codec, NULL) < 0) {
        log_debug("ffmpegdrv: could not open video codec");
        return -1;
    }

    video_is_open = 1;
    video_outbuf = NULL;
    if (!(oc->oformat->flags & AVFMT_RAWPICTURE)) {
        /* allocate output buffer */
        /* XXX: API change will be done */
        video_outbuf_size = 200000;
        video_outbuf = lib_malloc(video_outbuf_size);
    }
    video_frame = (*ffmpeglib.p_avcodec_alloc_frame)();
    if (!video_frame) {
        log_debug("ffmpegdrv: could not allocate video frame");
        return -1;
    }

    /* allocate the encoded raw picture */
    if ((*ffmpeglib.p_avpicture_alloc)(&dst_picture, c->pix_fmt,
                                        c->width, c->height) < 0)
    {
        log_debug("ffmpegdrv: could not allocate picture");
        return -1;
    }
#if 0
    picture = ffmpegdrv_alloc_picture(c->pix_fmt, c->width, c->height);
    if (!picture) {
        log_debug("ffmpegdrv: could not allocate picture");
        return -1;
    }
#endif
    /* if the output format is not RGB24, then a temporary YUV420P
       picture is needed too. It is then converted to the required
       output format */
    if (c->pix_fmt != PIX_FMT_RGB24) {
        if ((*ffmpeglib.p_avpicture_alloc)(&src_picture, PIX_FMT_RGB24,
                                        c->width, c->height) < 0)
        {
                log_debug("ffmpegdrv: could not allocate temporary picture");
            return -1;
        }

#if 0
        tmp_picture = ffmpegdrv_alloc_picture(PIX_FMT_RGB24, 
                                                c->width, c->height);
        if (!tmp_picture) {
            log_debug("ffmpegdrv: could not allocate temporary picture");
            return -1;
        }
#endif
    }

    /* copy data and linesize picture pointers to frame */
    *((AVPicture *)video_frame) = dst_picture;

    return 0;
}


static void ffmpegdrv_close_video(void)
{
    if (video_st == NULL)
        return;

    if (video_is_open) {
        (*ffmpeglib.p_avcodec_close)(video_st->codec);
        (*ffmpeglib.p_av_free)(src_picture.data[0]);
        (*ffmpeglib.p_av_free)(dst_picture.data[0]);
        (*ffmpeglib.p_av_free)(video_frame);
        lib_free(video_outbuf);
        video_outbuf = NULL;
    }

    video_is_open = 0;

#ifdef HAVE_FFMPEG_SWSCALE
    if (sws_ctx != NULL) {
        (*ffmpeglib.p_sws_freeContext)(sws_ctx);
    }
#endif
}


static void ffmpegdrv_init_video(screenshot_t *screenshot)
{
    AVCodecContext *c;
    AVStream *st;

    if (ffmpegdrv_oc == NULL || ffmpegdrv_fmt == NULL)
        return;

     video_init_done = 1;

     if (ffmpegdrv_fmt->video_codec == AV_CODEC_ID_NONE)
        return;

    st = (*ffmpeglib.p_avformat_new_stream)(ffmpegdrv_oc, video_codec);
    if (!st) {
        log_debug("ffmpegdrv: Could not alloc video stream\n");
        return;
    }

    c = st->codec;
    (*ffmpeglib.p_avcodec_get_context_defaults3)(c, video_codec);
    c->codec_id = ffmpegdrv_fmt->video_codec;

    /* put sample parameters */
    c->bit_rate = video_bitrate;
    /* resolution should be a multiple of 16 */
    video_width = c->width = (screenshot->width + 15) & ~0xf; 
    video_height = c->height = (screenshot->height + 15) & ~0xf;
    /* frames per second */
    c->time_base.den = (int)(vsync_get_refresh_frequency() + 0.5);
    if (video_halve_framerate) {
        c->time_base.den /= 2;
    }
    c->time_base.num = 1;
    c->gop_size = 12; /* emit one intra frame every twelve frames at most */
    c->pix_fmt = PIX_FMT_YUV420P;

    /* Avoid format conversion which would lead to loss of quality */
    if (c->codec_id == AV_CODEC_ID_FFV1) {
        c->pix_fmt = PIX_FMT_RGB32;
    }

    /* Avoid format conversion which would lead to loss of quality */
    if (c->codec_id == AV_CODEC_ID_H264) {
        (*ffmpeglib.p_av_opt_set)(c->priv_data, "preset", "slow", 0);
    }

    /* Use XVID instead of FMP4 FOURCC for better compatibility */
    if (c->codec_id == AV_CODEC_ID_MPEG4) {
        c->codec_tag = MKTAG('X','V','I','D');
    }

#ifdef HAVE_FFMPEG_SWSCALE
    /* setup scaler */
    if (c->pix_fmt != PIX_FMT_RGB24) {
        sws_ctx = (*ffmpeglib.p_sws_getContext)
            (video_width, video_height, PIX_FMT_RGB24, 
             video_width, video_height, c->pix_fmt, 
             SWS_BICUBIC, 
             NULL, NULL, NULL);
        if (sws_ctx == NULL) {
            log_debug("ffmpegdrv: Can't create Scaler!\n");
        }
    }
#endif

    video_st = st;
    video_pts = 0;
    framecounter = 0;

    if (audio_init_done)
        ffmpegdrv_init_file();
}


static int ffmpegdrv_init_file(void)
{
    if (!video_init_done || !audio_init_done)
        return 0;

#if 0
    if ((*ffmpeglib.p_av_set_parameters)(ffmpegdrv_oc, NULL) < 0) {
        log_debug("ffmpegdrv: Invalid output format parameters");
            return -1;
    }
#else
#endif

    (*ffmpeglib.p_av_dump_format)(ffmpegdrv_oc, 0, ffmpegdrv_oc->filename, 1);

    if (video_st && (ffmpegdrv_open_video(ffmpegdrv_oc, video_st) < 0)) {
        ui_error(translate_text(IDGS_FFMPEG_CANNOT_OPEN_VSTREAM));
        screenshot_stop_recording();
        return -1;
    }
    if (audio_st && (ffmpegdrv_open_audio(ffmpegdrv_oc, audio_st) < 0)) {
        ui_error(translate_text(IDGS_FFMPEG_CANNOT_OPEN_ASTREAM));
        screenshot_stop_recording();
        return -1;
    }

    if (!(ffmpegdrv_fmt->flags & AVFMT_NOFILE)) {
        if ((*ffmpeglib.p_avio_open)(&ffmpegdrv_oc->pb, ffmpegdrv_oc->filename,
                            AVIO_FLAG_WRITE) < 0) 
        {
            ui_error(translate_text(IDGS_FFMPEG_CANNOT_OPEN_S), ffmpegdrv_oc->filename);
            screenshot_stop_recording();
            return -1;
        }

    }

    (*ffmpeglib.p_avformat_write_header)(ffmpegdrv_oc, NULL);
    video_frame->pts = 0;
    log_debug("ffmpegdrv: Initialized file successfully");

    file_init_done = 1;

    return 0;
}


static int ffmpegdrv_save(screenshot_t *screenshot, const char *filename)
{
    video_st = NULL;
    audio_st = NULL;

    audio_init_done = 0;
    video_init_done = 0;
    file_init_done = 0;

#if 0
    ffmpegdrv_fmt = (*ffmpeglib.p_av_guess_format)(ffmpeg_format, NULL, NULL);

    if (!ffmpegdrv_fmt)
        ffmpegdrv_fmt = (*ffmpeglib.p_av_guess_format)("mpeg", NULL, NULL);

    if (!ffmpegdrv_fmt) {
        log_debug("ffmpegdrv: Cannot find suitable output format");
        return -1;
    }
#else
    (*ffmpeglib.p_avformat_alloc_output_context2)(&ffmpegdrv_oc, NULL, NULL, filename);
    if (!ffmpegdrv_oc) {
        (*ffmpeglib.p_avformat_alloc_output_context2)(&ffmpegdrv_oc, NULL, "mpeg", filename);
    }
    if (!ffmpegdrv_oc) {
        log_debug("ffmpegdrv: Cannot find suitable output format");
        return -1;
    }
    ffmpegdrv_fmt = ffmpegdrv_oc->oformat;

#endif
    if (format_index >= 0) {

        gfxoutputdrv_format_t *format = &ffmpegdrv_formatlist[format_index];

        if (format->audio_codecs !=NULL
            && (audio_codec = (*ffmpeglib.p_avcodec_find_encoder)(audio_codec_id)) != NULL)
        {
            ffmpegdrv_fmt->audio_codec = audio_codec_id;
        }

        if (format->video_codecs !=NULL
            &&  (video_codec = (*ffmpeglib.p_avcodec_find_encoder)(video_codec_id)) != NULL)
        {
            ffmpegdrv_fmt->video_codec = video_codec_id;
        }
    }
#if 0
    ffmpegdrv_oc = lib_malloc(sizeof(AVFormatContext));
    memset(ffmpegdrv_oc, 0, sizeof(AVFormatContext));

    if (!ffmpegdrv_oc) {
        log_debug("ffmpegdrv: Cannot allocate format context");
        return -1;
    }

    ffmpegdrv_oc->oformat = ffmpegdrv_fmt;
#endif
    snprintf(ffmpegdrv_oc->filename, sizeof(ffmpegdrv_oc->filename), 
             "%s", filename);

    ffmpegdrv_init_video(screenshot);

    soundmovie_start(&ffmpegdrv_soundmovie_funcs);

    return 0;
}

#if FFMPEG_ALIGNMENT_HACK
__declspec(naked) static int ffmpeg_avcodec_encode_video2(AVCodecContext *c, AVPacket *pkt,
                                                        const AVFrame *frame, int *got_output)
{
    _asm {
        /*
         * Create a standard stack frame.
         * This way, we can be sure that we
         * can restore ESP afterwards.
         */
        push ebp
        mov ebp,esp
        sub esp, __LOCAL_SIZE /* not needed, but safer against errors when changing this function */

        /* adjust stack to 16 byte boundary */
        and esp,~0x0f
    }

    /* execute the command */

    (*ffmpeglib.p_avcodec_encode_video2)(c, pkt, video_frame, got_output);

    _asm {
        /* undo the stack frame, restoring ESP and EBP */
        mov esp,ebp
        pop ebp
        ret
    }
}
#endif

/* triggered by screenshot_record */
static int ffmpegdrv_record(screenshot_t *screenshot)
{
    AVCodecContext *c;
    AVFrame *frame = NULL;
    int ret;
    int got_output = 0;
    AVPacket pkt = { 0 };

    if (audio_init_done && video_init_done && !file_init_done)
        ffmpegdrv_init_file();

    if (video_st == NULL || !file_init_done)
        return 0;

    if (audio_st && video_pts > audio_pts) {
        log_error(LOG_DEFAULT, "video frame dropped", video_pts);

        /* drop this frame */
        return 0;
    }
    
    framecounter++;
    if (screenshot && video_halve_framerate && (framecounter & 1)) {
        /* drop every second frame */
        return 0;
    }

    (*ffmpeglib.p_av_init_packet)(&pkt);

    c = video_st->codec;

    if (screenshot) {
        frame = video_frame;
        if (c->pix_fmt != PIX_FMT_RGB24) {
            ffmpegdrv_fill_rgb_image(screenshot, (AVFrame *)&src_picture);
#ifdef HAVE_FFMPEG_SWSCALE
            if (sws_ctx != NULL) {
                (*ffmpeglib.p_sws_scale)(sws_ctx, 
                    (uint8_t **)src_picture.data, src_picture.linesize, 0, c->height,
                    dst_picture.data, dst_picture.linesize);
            }
#else
            (*ffmpeglib.p_img_convert)((AVPicture *)picture, c->pix_fmt,
                        (AVPicture *)tmp_picture, PIX_FMT_RGB24,
                        c->width, c->height);
#endif
        } else {
            ffmpegdrv_fill_rgb_image(screenshot, (AVFrame *)&dst_picture);
        }
    }

    if (screenshot && ffmpegdrv_oc->oformat->flags & AVFMT_RAWPICTURE) {
        pkt.flags |= AV_PKT_FLAG_KEY;
        pkt.stream_index = video_st->index;
        pkt.data = dst_picture.data[0];
        pkt.size = sizeof(AVPicture);

        ret = (*ffmpeglib.p_av_interleaved_write_frame)(ffmpegdrv_oc, &pkt);
    } else {
        /* encode the image */
        pkt.data = NULL;
        pkt.size = 0;

#if FFMPEG_ALIGNMENT_HACK
        ret = ffmpeg_avcodec_encode_video2(c, &pkt, frame, &got_output);
#else
        ret = (*ffmpeglib.p_avcodec_encode_video2)(c, &pkt, frame, &got_output);
#endif
        if (ret < 0 ) {
            log_debug("ffmpegdrv: Cannot encode video");
            return -1;
        }

        if (got_output) {
            /* write the compressed frame in the media file */
#if 1
            if (c->coded_frame->pts != AV_NOPTS_VALUE) {
                pkt.pts = (*ffmpeglib.p_av_rescale_q)(video_frame->pts,
                                       c->time_base, video_st->time_base);
            }
#else
            if (pkt.pts != AV_NOPTS_VALUE)
                pkt.pts = (*ffmpeglib.p_av_rescale_q)(pkt.pts, c->time_base, video_st->time_base);
            if (pkt.dts != AV_NOPTS_VALUE)
                pkt.dts = (*ffmpeglib.p_av_rescale_q)(pkt.dts, c->time_base, video_st->time_base);
#endif

            if (c->coded_frame->key_frame) {
                pkt.flags |= AV_PKT_FLAG_KEY;
            }
            pkt.stream_index = video_st->index;

            /* Write the compressed frame to the media file. */
            ret = (*ffmpeglib.p_av_interleaved_write_frame)(ffmpegdrv_oc, &pkt);
        } else {
            ret = 0;
        }
    }
    if (ret != 0) {
        log_debug("ffmpegdrv: Cannot write video frame");
        return -1;
    }

    video_pts = (double)video_st->pts.val * video_st->time_base.num 
                    / video_st->time_base.den;
    log_error(LOG_DEFAULT, "video_pts: %f", video_pts);

    video_frame->pts++;

    if (!screenshot && got_output) {
        /* we are flushing the encoder and still data to encode */
        return 1;
    } else {
        return 0;
    }
}

static int ffmpegdrv_close(screenshot_t *screenshot)
{
    unsigned int i;

    soundmovie_stop();
    /* flush the video encoder */
    while (ffmpegdrv_record(NULL) > 0);

    /* write the trailer, if any */
    if (file_init_done) {
        (*ffmpeglib.p_av_write_trailer)(ffmpegdrv_oc);

        if (video_st) {
            ffmpegdrv_close_video();
        }
        if (audio_st) {
            ffmpegdrv_close_audio();
        }

        /* free the streams */
        for (i = 0; i < ffmpegdrv_oc->nb_streams; i++) {
            (*ffmpeglib.p_av_free)((void *)ffmpegdrv_oc->streams[i]->codec);
            (*ffmpeglib.p_av_free)((void *)ffmpegdrv_oc->streams[i]);
            ffmpegdrv_oc->streams[i] = NULL;
        }

        if (!(ffmpegdrv_fmt->flags & AVFMT_NOFILE)) {
            /* close the output file */
            (*ffmpeglib.p_avio_close)(ffmpegdrv_oc->pb);
        }
    }
    
    /* free the stream */
    (*ffmpeglib.p_av_free)(ffmpegdrv_oc);
    log_debug("ffmpegdrv: Closed successfully");

    file_init_done = 0;

    return 0;
}


static int ffmpegdrv_write(screenshot_t *screenshot)
{
    return 0;
}

static void ffmpegdrv_shutdown(void)
{
    ffmpeglib_close(&ffmpeglib);
    lib_free(ffmpeg_format);
}

static gfxoutputdrv_t ffmpeg_drv = {
    "FFMPEG",
    "FFMPEG",
    NULL,
    ffmpegdrv_formatlist,
    NULL, /* open */
    ffmpegdrv_close,
    ffmpegdrv_write,
    ffmpegdrv_save,
    NULL,
    ffmpegdrv_record,
    ffmpegdrv_shutdown,
    ffmpegdrv_resources_init,
    ffmpegdrv_cmdline_options_init
#ifdef FEATURE_CPUMEMHISTORY
    ,NULL
#endif
};

void gfxoutput_init_ffmpeg(void)
{
    if (ffmpeglib_open(&ffmpeglib) < 0)
        return;

    gfxoutput_register(&ffmpeg_drv);

    (*ffmpeglib.p_av_register_all)();
}

