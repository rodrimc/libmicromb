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

int
mb_init ();

GstElement *
mb_add_media (const gchar *uri, const gchar *media_alias);

int
mb_start (GstElement* element);

GstBus *
mb_get_message_bus ();

#endif /* MB_H_ */
