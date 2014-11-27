/*
 * util.h
 *
 *  Created on: Nov 17, 2014
 *      Author: Rodrigo Costa
 *			e-mail: rodrigocosta@telemidia.puc-rio.br
 */

#ifndef MB_H_
#define MB_H_

#include <gst/gst.h>

typedef struct
{
	gchar *name;
	gchar *decoder;

	//video
	gchar *video_scaler;
	gchar *video_filter;

	//audio
	gchar *audio_converter;
} MbMedia;

int
mb_init ();

MbMedia *
mb_media_new (const char *uri, const char *media_name);

int
mb_media_start (MbMedia *media);

GstBus *
mb_get_message_bus ();

void
mb_media_free (MbMedia *media);

void
mb_clean_up ();

#endif /* MB_H_ */
