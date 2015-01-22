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
	GstElement *decoder;

	//video
	GstElement *video_scaler;
	GstElement *video_filter;

	//image
	GstElement *image_freezer;

	//audio
	GstElement *audio_volume;
	GstElement *audio_converter;
	GstElement *audio_resampler;
	GstElement *audio_filter;

	//pads
	gchar *video_pad_name;
	gchar *audio_pad_name;

	//properties
	gint x_pos;
	gint y_pos;
	gint z_index;
	gint width;
	gint height;
	gdouble alpha;  /* [0, 1] */
	gdouble volume; /* [0, 1] */

	GMutex mutex;
	gint valid_pads;
} MbMedia; /*!< Representation of media objects to clients of this library*/

typedef enum
{
	/* FILL ME*/

	//Presentation
	MB_END,
	MB_PAUSE,
	MB_BEGIN,

	//Application
	MB_REMOVED

} MbEvent;

typedef struct
{
	MbEvent evt;
	MbMedia *media;
} MbMediaEvent;


/*!
 * \brief	Initializes the library.
 *
 *	@return TRUE if the library was properly initialized or FALSE otherwise.
 */
gboolean
mb_init ();


/*!
 * \brief	Initializes the library.
 * 	@param width Window width
 *	@param height Window height
 *
 *	@return TRUE if the library was properly initialized or FALSE otherwise.
 */
gboolean
mb_init_args (int width, int height);


/*!
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

/*!
 * \brief Frees a MbMedia
 * @param media MbMedia pointer
 */

void
mb_media_free (MbMedia *media);

/*!
 * \brief Change the value of a size property
 * 	@param media Media pointer
 * 	@param width Width value to set
 * 	@param height Height value to set
 *
 *	@return TRUE if the property was properly set or FALSE otherwise.
 */
gboolean
mb_media_set_size (MbMedia *media, int width, int height);

/*!
 * \brief Set the media position
 * 	@param media Media pointer
 * 	@param x New x value
 * 	@param y New y value
 *
 * 	@return TRUE if the position was properly set or FALSE otherwise.
 */
gboolean
mb_media_set_pos (MbMedia *media, int x, int y);

/*!
 * \brief Set the media volume
 * 	@param media Media pointer
 * 	@param volume New volume
 *
 * 	@return TRUE if the position was properly set or FALSE otherwise.
 */
gboolean
mb_media_set_volume (MbMedia *media, gdouble volume);


/*!
 * \brief Set the zIndex property (zorder)
 * 	@param media Media pointer
 * 	@param z New z value
 *
 * 	@return TRUE if the position was properly set or FALSE otherwise.
 */
gboolean
mb_media_set_z (MbMedia *media, int z);

/*!
 * \brief Set the alpha channel value
 * 	@param media Media pointer
 * 	@param alpha New alpha value (allowed values [0, 1])
 *
 * 	@return TRUE if the position was properly set or FALSE otherwise.
 */
gboolean
mb_media_set_alpha (MbMedia *media, double alpha);

/*!
 * \brief Starts a media
 * 	@param media Media pointer
 *
 *	@return TRUE if the media was properly started or FALSE otherwise.
 */
gboolean
mb_media_start (MbMedia *media);


/*!
 * \brief Stops a media
 * 	@param media Media pointer
 *
 *	@return TRUE if the media was properly stopped or FALSE otherwise.
 */
gboolean
mb_media_stop (MbMedia *media);

/*!
 * \brief Returns the window height
 *
 *	@return Window height.
 */
int
mb_get_window_height ();

/*!
 * \brief Returns the window width
 *
 *	@return Window width.
 */
int
mb_get_window_width ();

/*!
 * \brief Register a function that is called when an
 * 					event occurs on medias.
 *
 * 	@param handler Function handler.
 */
void
mb_register_handler (void (*)(MbMediaEvent *));

/*!
 * \brief Unregister the handler
 *
 */
void
mb_unregister_handler ();


GstBus *
mb_get_message_bus ();

/*!
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
