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

/*
 * TODO:
 * 	-> add 'videomixer' to the pipeline before 'video_sink'
 */

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

	int v_init;
	int a_init;

	GMutex v_mutex;
	GMutex a_mutex;
} MbData;

MbData _global;

void
pad_added_handler (GstElement *src, GstPad *new_pad, MbMedia *media);

int
set_audio_elements ();

int
set_video_elements ();

#endif /* UTIL_H_ */
