#pragma once

#ifdef HAVE_FFMPEG
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#endif

typedef struct {
	int width;           /**< Width of the video in pixels */
	int height;          /**< Height of the video in pixels */
	float aspectRatio;   /**< Aspect ratio of the video */
	int64_t usec;        /**< Time of frame in microseconds */
	unsigned char* data; /**< Contains the decoded image */
	char filename[1024]; /**< Filename of the video we loaded */

	/* The following variables are used internally by video.c */
	int has_new_video_frame;

#ifdef HAVE_FFMPEG
	enum AVPixelFormat pix_fmt;
	AVPacket pkt;
	int video_stream_idx;

	struct SwsContext *sws_ctx;
	AVFrame *frame;
	AVFormatContext *fmt_ctx;
	AVCodecContext *video_dec_ctx;
	AVStream *video_stream;

	int video_frame_count;
#endif
	
} video_state;

video_state* video_get_next_frame(video_state *state, const char *filename);
void video_cleanup(video_state *state);
