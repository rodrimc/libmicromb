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

/*
 * TODO:
 * 	-> add 'videomixer' to the pipeline before 'video_sink'
 * 	-> add 'adder' (or another audio mixer) before audio sink
 *
 */
typedef struct
{
	GstElement *pipeline;
	GstElement *video_sink;
} GlobalElements;

typedef struct
{
	GstElement *element;
	GlobalElements *p_global;
} Context;

void
pad_added_handler (GstElement *src, GstPad *new_pad, Context *context);

int
set_video_bin (Context *context, GstPad *new_pad);

//TODO
int
set_audio_bin (Context *context, GstPad *new_pad);

#endif /* UTIL_H_ */
