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

	//pads
	gchar *video_pad_name;
	gchar *audio_pad_name;

	//properties
	gint x_pos;
	gint y_pos;
	gint z_index;
	gint width;
	gint height;
	gdouble alpha;
} MbMedia; /*!< Representation of media objects to clients of this library*/

/**
 * \brief	Initializes the library.
 *
 *	@return SUCESS if the library was properly initialized or FAILURE otherwise.
 */
int
mb_init ();


/**
 * \brief	Initializes the library.
 * 	@param width Window width
 *	@param height Window height
 *
 *	@return SUCESS if the library was properly initialized or FAILURE otherwise.
 */
int
mb_init_args (int width, int height);


/**
 * \brief	Creates a new MbMedia.
 * 	@param media_name Name of the media (identifier)
 *	@param uri Media URI
 *	@param x X coord
 *	@param y Y coord
 *	@param width Width of the media
 *	@param height Height of the media
 */
MbMedia *
mb_media_new (const char *media_name, const char *uri,
							int x, int y, int z, int width, int height);

/**
 * \brief Frees a MbMedia
 * @param media MbMedia pointer
 */

void
mb_media_free (MbMedia *media);

/**
 * \brief Change the value of a size property
 * 	@param media Media pointer to change the property value
 * 	@param property Property name. Allowed values: width, height
 * 	@param value The new value
 *
 *	@return SUCESS if the property was properly set or FAILURE otherwise.
 */
int
mb_media_set_size_property (MbMedia *media, const char *property, int value);

/**
 * \brief Set the media position
 * 	@param media Media pointer to change the property value
 * 	@param x New x value
 * 	@param y New y value
 *
 * 	@return SUCESS if the position was properly set or FAILURE otherwise.
 */
int
mb_media_set_pos (MbMedia *media, int x, int y);

/**
 * \brief Set the zIndex property (zorder)
 * 	@param media Media pointer to change the property value
 * 	@param z New z value
 *
 * 	@return SUCESS if the position was properly set or FAILURE otherwise.
 */
int
mb_media_set_z (MbMedia *media, int z);

/**
 * \brief Set the alpha channel value
 * 	@param media Media pointer to change the property value
 * 	@param alpha New alpha value (allowed values [0, 1])
 *
 * 	@return SUCESS if the position was properly set or FAILURE otherwise.
 */
int
mb_media_set_alpha (MbMedia *media, double alpha);

/**
 * \brief Starts a media
 * 	@param media Media pointer to be started
 *
 *	@return SUCESS if the media was properly started or FAILURE otherwise.
 */
int
mb_media_start (MbMedia *media);

/**
 * \brief Returns the window height
 *
 *	@return Window height.
 */
int
mb_get_window_height ();

/**
 * \brief Returns the window width
 *
 *	@return Window width.
 */
int
mb_get_window_width ();


GstBus *
mb_get_message_bus ();

/**
 * \brief Frees internal data structures.
 *
 * \detail Should be called on the end of the application.
 * 				 Once this function is called, call other functions of the library
 * 				 won't work until the mb_init () is called again.
 *
 */
void
mb_clean_up ();

#endif /* MB_H_ */
