/*
*
*  Copyright (C) 2015 "Rodrigo Costa" <rodrigocosta@telemidia.puc-rio.br>
* 
*  This file is part of Libmicromb.
*
*  Libmicromb is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  Libmicromb is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with Libmicromb.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef UTIL_H_
#define UTIL_H_

#include <gst/gst.h>
#include "mb.h"

typedef enum
{
	APP_EVT_MEDIA_END
	/* FILL ME */
}AppEventType;

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

	//clock
	GstClock *clock_provider;

	//bus
	GstBus *bus;
	GMainLoop *loop; //It is needed to handle messages coming from the bus.
	GThread *loop_thread;

	//handlers
	void (*evt_handler)(MbMediaEvent*);
} MbData;

MbData _global;

gboolean
init (int width, int height);

gboolean
has_image_extension (const char *uri);

void
notify_handler (MbEvent evt, MbMedia *media);

//helpers
gboolean
set_video_bin(GstElement *bin, MbMedia *media, GstPad *decoder_src_pad);

gboolean
set_audio_bin(GstElement *bin, MbMedia *media, GstPad *decoder_src_pad); 

//callbacks
void
pad_added_cb (GstElement *src, GstPad *new_pad, MbMedia *media);

GstPadProbeReturn
stop_pad_cb (GstPad *pad, GstPadProbeInfo *info, gpointer media);

GstPadProbeReturn
eos_event_cb (GstPad *pad, GstPadProbeInfo * info, gpointer media);

#endif /* UTIL_H_ */
