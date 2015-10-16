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
#include <assert.h>
#include "mb.h"

typedef enum
{
	APP_EVT_MEDIA_END
	/* FILL ME */
}AppEventType;

typedef struct MbData
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

  //internal variables
	GMutex mutex;
  gboolean sync;
  gboolean initialized;

  //handlers
	void (*evt_handler)(MbEvent*);
} MbData;

MbData _mb_global_data;

gboolean
init (int width, int height, gboolean sync);

gboolean
has_image_extension (const char *uri);

void
notify_handler (MbEvent *event);


/*Callers of functions that return MbEvent * should free
  the pointer returned */
MbEvent *
create_state_change_event (MbEventType, const char *); 

MbEvent *
create_mouse_button_event (MbEventType, MbMouseButton, int, int);

MbEvent *
create_mouse_move_event (MbEventType, int, int);

MbEvent *
create_keyboard_event (MbEventType, const char *);

MbEvent *
create_app_event (MbEventType);

/* helpers */
gboolean
set_video_bin(GstElement *, MbMedia *, GstPad *);

gboolean
set_audio_bin(GstElement *, MbMedia *, GstPad *); 

MbEvent *
handle_navigation_message (GstMessage *);

MbEvent *
handle_application_message (GstMessage *);

MbEvent *
handle_state_change_message (GstMessage *);

/* callbacks */
void
pad_added_cb (GstElement *, GstPad *, MbMedia *);

GstPadProbeReturn
stop_pad_cb (GstPad *, GstPadProbeInfo *, gpointer);

GstPadProbeReturn
eos_event_cb (GstPad *, GstPadProbeInfo *, gpointer );

#endif /* UTIL_H_ */
