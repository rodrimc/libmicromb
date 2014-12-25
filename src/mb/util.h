/*
 * util.h
 *
 *  Created on: Nov 17, 2014
 *      Author: Rodrigo Costa
 *			e-mail: rodrigocosta@telemidia.puc-rio.br
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <gst/gst.h>
#include "mb.h"

typedef struct
{
	//pipeline
	GstElement *pipeline;

	//mixers
	GstElement *video_mixer;
	GstElement *audio_mixer;

	//sinks
	GstElement *video_sink;
	GstElement *audio_sink;

	int a_init;

	GMutex a_mutex;

	int window_height;
	int window_width;
} MbData;

MbData _global;

void
pad_added_handler (GstElement *src, GstPad *new_pad, MbMedia *media);

int
init (int width, int height);

int
set_audio_elements ();

#endif /* UTIL_H_ */
