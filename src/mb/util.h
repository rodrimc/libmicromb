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

	//properties
	int window_height;
	int window_width;

	//handlers
	void (*evt_handler)(MbMediaEvent*);
} MbData;

MbData _global;

gboolean
init (int width, int height);

gboolean
has_image_extension (const char *uri);

//callbacks
void
pad_added_cb (GstElement *src, GstPad *new_pad, MbMedia *media);

GstPadProbeReturn
stop_pad_cb (GstPad *pad, GstPadProbeInfo *info, gpointer media);

GstPadProbeReturn
eos_event_cb (GstPad *pad, GstPadProbeInfo * info, gpointer media);

#endif /* UTIL_H_ */
