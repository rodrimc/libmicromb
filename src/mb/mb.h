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

#define SUCCESS 0
#define FAILURE -1

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

/*
 * \brief	Initializes the library.
 *
 */
int
mb_init ();


/*
 * \brief	Initializes the library.
 * 	@param width Window width
 *	@param height Window height
 */
int
mb_init_args (int width, int height);

MbMedia *
mb_media_new (const char *uri, const char *media_name);

void
mb_media_free (MbMedia *media);

/*
 * \brief Change the value of a size property
 * 	@param media Media pointer to change the property value
 * 	@param property Property name. Allowed values: width, height
 * 	@param value The new value
 *
 */
int
mb_media_set_size_property (MbMedia *media, const char *property, int value);

/*
 * \brief Starts a media
 * 	@param media Media pointer to be started
 *
 */
int
mb_media_start (MbMedia *media);

GstBus *
mb_get_message_bus ();

/*
 * \brief Frees internal data structures. Should be called on the end
 * 				of the application. Once this function is called, call other
 * 				functions of the library won't work until the mb_init () is
 * 				called again.
 *
 */
void
mb_clean_up ();

#endif /* MB_H_ */
