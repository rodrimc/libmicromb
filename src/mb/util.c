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

#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <gst/video/video.h>

static char* audio_caps = "audio/x-raw,rate=48000";
static char *image_exts [] = { ".jpg", ".jpeg", ".png", ".gif", ".bmp", NULL };

MbEvent *
handle_navigation_message (GstMessage *message)
{
  MbEvent *mb_event = NULL;
  GstNavigationMessageType nav_msg_type;
  nav_msg_type = gst_navigation_message_get_type (message);
  switch (nav_msg_type)
  {
    case GST_NAVIGATION_MESSAGE_EVENT:
    {
      GstEvent *event;
      GstNavigationEventType nav_evt_type;

      gst_navigation_message_parse_event (message, &event);
      nav_evt_type = gst_navigation_event_get_type (event);

      switch (nav_evt_type) 
      {
        case GST_NAVIGATION_EVENT_MOUSE_BUTTON_PRESS:
        case GST_NAVIGATION_EVENT_MOUSE_BUTTON_RELEASE:
        {
          int button;
          double x, y;
          gst_navigation_event_parse_mouse_button_event (event, &button,
              &x, &y);
          mb_event = create_mouse_button_event (
              nav_evt_type == GST_NAVIGATION_EVENT_MOUSE_BUTTON_PRESS ? 
              MB_MOUSE_BUTTON_PRESS : MB_MOUSE_BUTTON_RELEASE,
              button, (int)x, (int) y);
          break;
        }
        case GST_NAVIGATION_EVENT_MOUSE_MOVE:
        {
          double x, y;
          if (gst_navigation_event_parse_mouse_move_event (event, &x, &y))
            mb_event = create_mouse_move_event (MB_MOUSE_MOVE, (int)x, 
                (int) y);

          break;
        }
        case GST_NAVIGATION_EVENT_KEY_PRESS:
        case GST_NAVIGATION_EVENT_KEY_RELEASE:
        {
          const char *key;
          if (gst_navigation_event_parse_key_event (event, &key))
            mb_event = create_keyboard_event (
              nav_evt_type == GST_NAVIGATION_EVENT_KEY_PRESS ? 
              MB_KEY_PRESS : MB_KEY_RELEASE, key);
        }
        default:
          break;  
      }
      gst_event_unref (event);
      break;
    }
    default:
      break;
  }

  return mb_event;
}

MbEvent *
handle_application_message (GstMessage *message)
{
  MbEvent *mb_event = NULL;
  const GstStructure *msg_struct = NULL;
  int evt_type;

  msg_struct = gst_message_get_structure(message);
  gst_structure_get_int (msg_struct, "event_type", &evt_type);

  switch (evt_type)
  {
    case APP_EVT_MEDIA_END:
    {
      if (gst_structure_has_field(msg_struct, "data"))
      {
        MbMedia *media = NULL;
        GstElement *bin = NULL, *element = NULL;
        GstIterator *bin_it = NULL;
        GValue data = G_VALUE_INIT;
        gboolean done = FALSE;

        gst_structure_get (msg_struct, "data", G_TYPE_POINTER, &media,
            NULL);
        g_assert (media);

        bin = gst_bin_get_by_name(GST_BIN(_mb_global_data.pipeline), 
            media->name);
        g_assert (bin);

        gst_element_set_state(bin, GST_STATE_NULL);
        gst_bin_remove (GST_BIN (_mb_global_data.pipeline), bin);

        bin_it = gst_bin_iterate_elements(GST_BIN(bin));
        while (!done)
        {
          switch (gst_iterator_next (bin_it, &data))
          {
            case GST_ITERATOR_OK:
            {
              element = GST_ELEMENT(g_value_get_object (&data));
              g_assert (element);

              /* we increment the ref_count to destroy this element
               * only when calling the function mb_media_free ()
               */
              g_print ("Removing %s from pipeline.\n", media->name);

              gst_object_ref(element);
              gst_bin_remove(GST_BIN(bin), element);
              gst_element_set_state(element, GST_STATE_NULL);
              break;
            }
            case GST_ITERATOR_RESYNC:
            {  
              gst_iterator_resync (bin_it);
              break;
            }
            case GST_ITERATOR_ERROR:
            case GST_ITERATOR_DONE:
            {
              done = TRUE;
              break;
            }
            default:
              break;
          }
        }
        gst_iterator_free(bin_it);
        gst_object_unref(bin);

        mb_event = create_state_change_event (MB_REMOVED, media->name);
      }
      break;
    }
  }
  return mb_event;
}

MbEvent *
handle_state_change_message (GstMessage *message)
{
  MbEvent *mb_event = NULL;
  GstState old_state, new_state;
  GstElement *source = (GstElement *) message->src;
  gst_message_parse_state_changed (message, &old_state, &new_state, NULL);
  if (new_state == GST_STATE_PLAYING)
  {
    if (source == _mb_global_data.pipeline)
    {
      g_mutex_lock (&(_mb_global_data.mutex));
      if (_mb_global_data.initialized == FALSE)
      {
        if (_mb_global_data.clock_provider == NULL)
          _mb_global_data.clock_provider = 
            gst_element_get_clock(_mb_global_data.pipeline);

        _mb_global_data.initialized = TRUE;

        mb_event = create_app_event (MB_APP_INIT_DONE);
      }
      g_mutex_unlock (&(_mb_global_data.mutex));
    }
    else
    {
      if (strcmp (G_OBJECT_TYPE_NAME(G_OBJECT (source)), "GstBin") == 0)
        mb_event = create_state_change_event (MB_BEGIN,
            GST_ELEMENT_NAME (source));
    }
  }

  return mb_event;
}


static gboolean
bus_cb (GstBus *bus, GstMessage *message, gpointer data)
{
  MbEvent *mb_event = NULL;
  GstElement *source = (GstElement *) message->src;
  const GstStructure *msg_struct;
  switch (GST_MESSAGE_TYPE (message))
  {
    case GST_MESSAGE_STATE_CHANGED:
    {
      mb_event = handle_state_change_message(message);
      break;
    }
    case GST_MESSAGE_ELEMENT:
    {
      if (source == _mb_global_data.video_sink)
      {
        const char* struct_name; 
        msg_struct = gst_message_get_structure(message);
        struct_name = gst_structure_get_name (msg_struct);     
        if (strcmp (struct_name, "GstNavigationMessage") == 0)
          mb_event = handle_navigation_message(message);
      }
      break;
    }
    case GST_MESSAGE_APPLICATION:
    {
      mb_event = handle_application_message (message);
      break;
    }
    case GST_MESSAGE_ERROR:
    {
      GError *err;
      gchar *debug;

      gst_message_parse_error (message, &err, &debug);
      g_print ("Error: %s\n", err->message);
      g_error_free (err);
      g_free (debug);

      break;
    }
    case GST_MESSAGE_EOS:
    {
      /* end-of-stream */
      break;
    }
    default:
      /* unhandled message */
      break;
  }

  if (mb_event != NULL)
  {
    notify_handler(mb_event);
    free (mb_event);
    mb_event = NULL;
  }

  return TRUE;
}

static void
main_loop_thread  ()
{
  _mb_global_data.loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (_mb_global_data.loop);
  g_thread_exit (0);
}

gboolean
has_image_extension (const char *uri)
{
  int i;
  if (uri == NULL)
    return FALSE;

  for (i = 0; image_exts[i] != NULL; i++)
  {
    if (g_str_has_suffix(uri, image_exts[i]))
      return TRUE;
  }

  return FALSE;
}

static void
set_background ()
{
  GstElement *bg_src = NULL, *bg_scaler = NULL, *bg_filter = NULL,
             *bg_capsfilter = NULL, *audio_src = NULL, *audio_conv = NULL,
             *audio_resample = NULL, *audio_capsfilter = NULL;
  GstCaps *caps;

  /* black background */
  bg_src = gst_element_factory_make ("videotestsrc", NULL);
  g_assert (bg_src);

  bg_scaler = gst_element_factory_make ("videoscale", NULL);
  g_assert (bg_scaler);

  bg_capsfilter = gst_element_factory_make ("capsfilter", NULL);
  g_assert (bg_capsfilter);

  g_object_set (G_OBJECT (bg_src), "pattern", /*black*/ 2, NULL);

  caps = gst_caps_new_simple (
      "video/x-raw",
      "framerate", GST_TYPE_FRACTION, 25, 1,
      "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
      "width", G_TYPE_INT, _mb_global_data.window_width,
      "height", G_TYPE_INT, _mb_global_data.window_height,
      NULL);
  g_assert (caps);

  g_object_set (G_OBJECT (bg_scaler), "add-borders", 0, NULL);
  g_object_set (G_OBJECT(bg_capsfilter), "caps", caps, NULL);

  gst_caps_unref(caps);

  /* silence audio */
  audio_src = gst_element_factory_make ("audiotestsrc", NULL);
  g_assert (audio_src);

  audio_conv = gst_element_factory_make ("audioconvert", NULL);
  g_assert (audio_src);

  audio_resample = gst_element_factory_make ("audioresample", NULL);
  g_assert (audio_resample);

  audio_capsfilter = gst_element_factory_make ("capsfilter", NULL);
  g_assert (audio_capsfilter);

  g_object_set (G_OBJECT (audio_src), "wave", /*silence*/ 4, NULL);

  caps = gst_caps_from_string (audio_caps);
  g_assert (caps);
  g_object_set (audio_capsfilter, "caps", caps, NULL);

  gst_caps_unref(caps);

  gst_bin_add_many(GST_BIN(_mb_global_data.pipeline), bg_src, bg_scaler,
      bg_capsfilter, audio_src, audio_conv, audio_resample,
      audio_capsfilter, NULL);

  if (!gst_element_link_many(bg_src, bg_scaler, bg_capsfilter, NULL) ||
      !gst_element_link_many (audio_src, audio_conv, audio_resample,
        audio_capsfilter, NULL))
  {
    g_printerr ("Could not link background elements together.\n");
    mb_clean_up ();
  }
  else
  {
    GstPad *src_pad = NULL, *mixer_sink_pad = NULL;
    GstPadLinkReturn ret;
    GstCaps *audio_caps = NULL;
    GstStructure *audio_caps_structure = NULL;
    const gchar *name = NULL;
    int rate;

    /* video */
    src_pad = gst_element_get_static_pad(bg_capsfilter, "src");
    g_assert (src_pad);

    mixer_sink_pad = gst_element_get_request_pad(_mb_global_data.video_mixer,
        "sink_%u");
    g_assert (mixer_sink_pad);

    ret = gst_pad_link (src_pad, mixer_sink_pad);
    if (GST_PAD_LINK_FAILED(ret))
      g_print (" Background video link failed.\n");
    else
    {
      g_print (" Background video link succeeded.\n");
    }

    gst_object_unref (src_pad);
    gst_object_unref (mixer_sink_pad);
    src_pad = NULL;
    mixer_sink_pad = NULL;

    /* audio */
    src_pad = gst_element_get_static_pad(audio_capsfilter, "src");
    g_assert (src_pad);

    mixer_sink_pad = gst_element_get_request_pad(_mb_global_data.audio_mixer,
        "sink_%u");
    g_assert (mixer_sink_pad);

    ret = gst_pad_link (src_pad, mixer_sink_pad);
    if (GST_PAD_LINK_FAILED(ret))
      g_print (" Background audio link failed.\n");
    else
    {
      g_print (" Background audio link succeeded.\n");
    }

    audio_caps = gst_pad_query_caps (src_pad, NULL);
    audio_caps_structure = gst_caps_get_structure (audio_caps, 0);
    name = gst_structure_get_name (audio_caps_structure);
    if (gst_structure_has_field(audio_caps_structure, "rate"))
      gst_structure_get_int(audio_caps_structure, "rate", &rate);

    gst_caps_unref(audio_caps);
    gst_object_unref (src_pad);
    gst_object_unref (mixer_sink_pad);
  }
}

int
init (int width, int height, gboolean sync)
{
  GstStateChangeReturn ret;
  GstState current_state;

  if (!gst_init_check(NULL, NULL, NULL))
  {
    g_printerr ("Failed to initialize gstreamer...\n");
    return FALSE;
  }

  _mb_global_data.pipeline = gst_pipeline_new ("pipeline");
  g_assert (_mb_global_data.pipeline);

  g_mutex_init (&(_mb_global_data.mutex));

  _mb_global_data.video_mixer 		= NULL;
  _mb_global_data.audio_mixer 		= NULL;
  _mb_global_data.video_sink  		= NULL;
  _mb_global_data.audio_sink  		= NULL;
  _mb_global_data.clock_provider	= NULL;
  _mb_global_data.loop            = NULL;
  _mb_global_data.bus             = NULL;
  _mb_global_data.window_width 		= width;
  _mb_global_data.window_height 	= height;
  _mb_global_data.initialized     = FALSE;
  _mb_global_data.sync            = sync;

  /* video */
  _mb_global_data.video_mixer = gst_element_factory_make("videomixer", 
      "video_mixer");
  g_assert(_mb_global_data.video_mixer);

  _mb_global_data.video_sink = gst_element_factory_make("xvimagesink",
      "video_sink");
  g_assert (_mb_global_data.video_sink);

  /* audio */
  _mb_global_data.audio_mixer = gst_element_factory_make("adder", 
      "audio_mixer");
  g_assert(_mb_global_data.audio_mixer);

  _mb_global_data.audio_sink = gst_element_factory_make("autoaudiosink", 
      "audio_sink");
  g_assert (_mb_global_data.audio_sink);

  gst_bin_add_many(GST_BIN (_mb_global_data.pipeline),
      _mb_global_data.video_mixer, _mb_global_data.video_sink,
      _mb_global_data.audio_mixer, _mb_global_data.audio_sink,
      NULL);

  if (!gst_element_link(_mb_global_data.video_mixer,
        _mb_global_data.video_sink))
  {
    g_printerr ("Could not link video mixer and sink together.\n");
    mb_clean_up ();
    return FALSE;
  }

  if (!gst_element_link(_mb_global_data.audio_mixer, 
        _mb_global_data.audio_sink))
  {
    g_printerr ("Could not link audio mixer and sink together.\n");
    mb_clean_up ();
    return FALSE;
  }

  gst_bus_add_watch (mb_get_message_bus (), bus_cb, NULL);

  set_background();

  gst_element_set_state (_mb_global_data.pipeline, GST_STATE_PLAYING);

  if (_mb_global_data.sync == TRUE)
  {
    do
    {
      ret = gst_element_get_state (_mb_global_data.pipeline, &current_state,
          NULL, GST_CLOCK_TIME_NONE);
      if (ret == GST_STATE_CHANGE_FAILURE)
      {
        mb_clean_up ();
        return FALSE;
      }
    } while (current_state != GST_STATE_PLAYING);
    _mb_global_data.clock_provider = gst_element_get_clock(
        _mb_global_data.pipeline);
  }

  if ((_mb_global_data.loop_thread = g_thread_new ("bus_thread",
          (GThreadFunc) main_loop_thread,
          NULL)) == NULL)
  {
    g_printerr ("Could not create message handler thread.\n");
    mb_clean_up();
    return FALSE;
  }


  return TRUE;
}

void
pad_added_cb (GstElement *src, GstPad *new_pad, MbMedia *media)
{
  GstPadLinkReturn ret;
  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  GstPad *peer = NULL;
  const gchar *new_pad_type = NULL;
  gboolean success = FALSE;

  g_assert (media);

  g_print ("Received new pad '%s' from '%s'\n", GST_PAD_NAME(new_pad),
      media->name);

  new_pad_caps = gst_pad_query_caps (new_pad, NULL);
  new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
  new_pad_type = gst_structure_get_name (new_pad_struct);

  g_print ("New pad type: %s\n", new_pad_type);

  g_mutex_lock(&(media->mutex));

  media->valid_pads++;

  if (g_str_has_prefix(new_pad_type, "video"))
  {
    success = set_video_bin (media->bin, media, new_pad);

    if (success)
      peer = gst_element_get_static_pad(_mb_global_data.video_mixer,
          media->video_pad_name);
  }
  else if (g_str_has_prefix(new_pad_type, "audio"))
  {
    success = set_audio_bin (media->bin, media, new_pad);

    if (success)
      peer = gst_element_get_static_pad(_mb_global_data.audio_mixer,
          media->audio_pad_name);
  }

  if (success)
  {
    gst_pad_set_offset (new_pad, media->start_offset);

    if (peer != NULL)
    {
      gst_pad_add_probe (peer, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM,
          eos_event_cb, media, NULL);

      gst_object_unref(peer);
    }
  }

  g_mutex_unlock(&(media->mutex));

  if (new_pad_caps != NULL)
    gst_caps_unref (new_pad_caps);
}

gboolean
set_video_bin(GstElement *bin, MbMedia *media, GstPad *decoder_src_pad)
{
  GstElement *sink_element = NULL;
  GstCaps *caps = NULL;
  GstPad *sink_pad = NULL, *ghost_pad = NULL, *output_sink_pad = NULL;
  GstPadLinkReturn ret;
  gchar *uri = NULL;
  gboolean is_image = FALSE;
  int return_code = TRUE;

  g_assert (media->video_scaler);
  g_assert (media->video_filter);

  gst_element_set_state(media->video_scaler, GST_STATE_PAUSED);
  gst_element_set_state(media->video_filter, GST_STATE_PAUSED);

  caps = gst_caps_new_simple ("video/x-raw",
      "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
      "width", G_TYPE_INT, media->width,
      "height", G_TYPE_INT, media->height,
      NULL);

  g_object_set (G_OBJECT (media->video_scaler), "add-borders", 0, NULL);
  g_object_set (G_OBJECT (media->video_filter), "caps", caps, NULL);

  gst_bin_add_many(GST_BIN (bin), media->video_scaler, media->video_filter,
      NULL);
  if (!gst_element_link (media->video_scaler, media->video_filter))
  {
    g_printerr ("Could not link elements together.\n");
    gst_object_unref (media->video_scaler);
    gst_object_unref (media->video_filter);
    return FALSE;
  }

  sink_element = media->video_scaler;

  g_object_get (G_OBJECT(media->decoder), "uri", &uri, NULL);
  is_image = has_image_extension(uri);

  g_free (uri);

  if (is_image)
  {
    media->image_freezer = gst_element_factory_make("imagefreeze", NULL);
    g_assert (media->image_freezer);

    gst_bin_add (GST_BIN(bin), media->image_freezer);

    if (!gst_element_link (media->image_freezer, media->video_scaler))
    {
      g_printerr("Could not link image element.\n");
      gst_object_unref(media->image_freezer);
      return FALSE;
    }

    gst_element_set_state(media->image_freezer, GST_STATE_PAUSED);
    sink_element = media->image_freezer;
  }

  sink_pad = gst_element_get_static_pad (sink_element, "sink");
  g_assert(sink_pad);
  ret = gst_pad_link (decoder_src_pad, sink_pad);

  if (GST_PAD_LINK_FAILED(ret))
    g_print (" Link failed.\n");
  else
    g_print (" Link succeeded.\n");

  ghost_pad = gst_ghost_pad_new (
      "v_src", gst_element_get_static_pad (media->video_filter, "src"));

  gst_pad_set_active (ghost_pad, TRUE);
  gst_element_add_pad (bin, ghost_pad);

  output_sink_pad = gst_element_get_request_pad(_mb_global_data.video_mixer,
      "sink_%u");
  g_assert (output_sink_pad);

  media->video_pad_name = gst_pad_get_name (output_sink_pad);
  g_print ("videomixer: new pad requested (%s)\n", media->video_pad_name);

  ret = gst_pad_link (ghost_pad, output_sink_pad);
  if (GST_PAD_LINK_FAILED(ret))
  {
    return_code = FALSE;
    g_print (" Could not link %s and videomixer together\n", media->name);
  }
  else
  {
    g_object_set (output_sink_pad, "xpos", media->x_pos, NULL);
    g_object_set (output_sink_pad, "ypos", media->y_pos, NULL);
    g_object_set (output_sink_pad, "zorder", media->z_index, NULL);
    g_object_set (output_sink_pad, "alpha", media->alpha, NULL);

    g_debug (" Link succeeded between %s and videomixer.\n", media->name);
  }

  if (is_image)
    gst_element_set_state(media->image_freezer, GST_STATE_PLAYING);

  gst_element_set_state (media->video_scaler, GST_STATE_PLAYING);
  gst_element_set_state (media->video_filter, GST_STATE_PLAYING);

  gst_caps_unref(caps);
  gst_object_unref (output_sink_pad);
  gst_object_unref(sink_pad);

  return return_code;
}

gboolean
set_audio_bin(GstElement *bin, MbMedia *media, GstPad *decoder_src_pad)
{
  GstPad *sink_pad = NULL, *ghost_pad = NULL, *output_sink_pad = NULL;
  GstCaps *caps = NULL;
  GstPadLinkReturn ret;
  int return_code = TRUE;

  media->audio_volume = gst_element_factory_make ("volume", NULL);
  g_assert (media->audio_volume);

  media->audio_converter = gst_element_factory_make ("audioconvert", NULL);
  g_assert (media->audio_converter);

  media->audio_resampler = gst_element_factory_make ("audioresample", NULL);
  g_assert (media->audio_resampler);

  media->audio_filter = gst_element_factory_make ("capsfilter", NULL);
  g_assert (media->audio_filter);

  gst_element_set_state (media->audio_volume, GST_STATE_PAUSED);
  gst_element_set_state (media->audio_converter, GST_STATE_PAUSED);
  gst_element_set_state (media->audio_resampler, GST_STATE_PAUSED);
  gst_element_set_state (media->audio_filter, GST_STATE_PAUSED);

  caps = gst_caps_from_string (audio_caps);
  g_assert (caps);

  g_object_set (media->audio_filter, "caps", caps, NULL);
  gst_caps_unref(caps);

  gst_bin_add_many (GST_BIN(bin), media->audio_volume, media->audio_converter,
      media->audio_resampler, media->audio_filter, NULL);

  if (!gst_element_link_many (media->audio_volume, media->audio_converter,
        media->audio_resampler, media->audio_filter, NULL))
  {
    g_print ("Could not link audio_converter and audio_volume together\n.");
    return_code = FALSE;
  }
  else
  {
    sink_pad = gst_element_get_static_pad (media->audio_volume, "sink");
    g_assert (sink_pad);

    ret = gst_pad_link (decoder_src_pad, sink_pad);
    if (GST_PAD_LINK_FAILED(ret))
    {
      return_code = FALSE;
      g_print (" Link failed.\n");
    }
    else
    {
      g_print (" Link succeeded.\n");

      g_object_set (G_OBJECT(media->audio_volume), "volume", media->volume,
          NULL);

      gst_element_set_state (media->audio_volume, GST_STATE_PLAYING);
      gst_element_set_state (media->audio_converter, GST_STATE_PLAYING);
      gst_element_set_state (media->audio_resampler, GST_STATE_PLAYING);
      gst_element_set_state (media->audio_filter, GST_STATE_PLAYING);

      ghost_pad = gst_ghost_pad_new (
          "a_src", gst_element_get_static_pad (media->audio_filter,
            "src"));
      gst_pad_set_active (ghost_pad, TRUE);
      gst_element_add_pad (bin, ghost_pad);

      output_sink_pad = gst_element_get_request_pad (_mb_global_data.audio_mixer,
          "sink_%u");
      g_assert(output_sink_pad);

      media->audio_pad_name = gst_pad_get_name(output_sink_pad);
      g_print ("audiomixer: new pad requested (%s)\n", media->audio_pad_name);

      ret = gst_pad_link (ghost_pad, output_sink_pad);
      if (GST_PAD_LINK_FAILED(ret))
      {
        return_code = FALSE;
        g_print (" Could not link %s and audiomixer together.\n", media->name);
      }

      gst_object_unref (output_sink_pad);
      gst_object_unref (sink_pad);
    }
  }
  return return_code;
}

GstPadProbeReturn
eos_event_cb (GstPad *pad, GstPadProbeInfo *info, gpointer data)
{
  if (GST_EVENT_TYPE (GST_PAD_PROBE_INFO_DATA (info)) == GST_EVENT_EOS)
  {
    GstElement *bin = NULL;
    GstMessage *message = NULL;
    GstStructure *msg_struct = NULL;
    MbMedia *media;
    gchar *uri = NULL;
    int pads = 0;

    media = (MbMedia *) data;

    bin = gst_bin_get_by_name (GST_BIN(_mb_global_data.pipeline), media->name);
    g_assert(bin);

    g_mutex_lock (&(media->mutex));
    pads = --media->valid_pads;
    g_mutex_unlock (&(media->mutex));

    g_object_get (media->decoder, "uri", &uri, NULL);
    g_print ("EOS received (%s): %s.\n", media->name, uri);
    g_print ("%s still has %d valid pad(s).\n", media->name, pads);

    if (pads == 0)
    {
      MbEvent *event = create_state_change_event (MB_END, media->name);
      notify_handler (event);
      free (event);

      msg_struct = gst_structure_new ("end-media", "event_type", G_TYPE_INT,
          APP_EVT_MEDIA_END, "data", G_TYPE_POINTER,
          media,
          /* FILL ME IF NECESSARY */
          NULL);

      message = gst_message_new_application (GST_OBJECT(bin), msg_struct);
      g_print ("Posting event\n");
      gst_bus_post (_mb_global_data.bus, message);
    }
    g_free (uri);
  }
  return GST_PAD_PROBE_OK;
}

GstPadProbeReturn
stop_pad_cb (GstPad *pad, GstPadProbeInfo *info, gpointer data)
{
  GstElement *bin = NULL;
  MbMedia *media = NULL;
  GstPad *peer;

  GST_DEBUG_OBJECT(pad, "pad is blocked now");
  /* remove the probe first */
  gst_pad_remove_probe (pad, GST_PAD_PROBE_INFO_ID(info));

  media = (MbMedia *) data;
  g_assert (media);

  peer = gst_pad_get_peer (pad);
  g_assert(peer);

  gst_pad_send_event (peer, gst_event_new_eos ());
  gst_object_unref(peer);

  return GST_PAD_PROBE_OK;
}

MbEvent *
create_app_event (MbEventType type)
{
  MbEvent *e = (MbEvent *) malloc (sizeof (MbEvent));
  assert (e);
  e->type = type;
  return e;
}

MbEvent *
create_state_change_event (MbEventType type, const char *media_name)
{
  MbStateChangeEvent state_change;
  MbEvent *e = (MbEvent *) malloc (sizeof (MbEvent));
  assert (e);

  state_change.type = type;
  state_change.media_name = media_name;

  e->state_change = state_change;

  return e;
}

MbEvent *
create_mouse_button_event (MbEventType type, MbMouseButton button, 
    int x, int y)
{
  MbMouseButtonEvent mouse_button;

  MbEvent *e = (MbEvent *) malloc (sizeof (MbEvent));
  assert (e);

  mouse_button.type = type;
  mouse_button.button = button;
  mouse_button.x = x;
  mouse_button.y = y;

  e->mouse_button = mouse_button;

  return e;
}  

MbEvent *
create_mouse_move_event (MbEventType type, int x, int y)
{
  MbMouseMoveEvent mouse_move;

  MbEvent *e = (MbEvent *) malloc (sizeof (MbEvent));
  assert (e);

  mouse_move.type = type;
  mouse_move.x = x;
  mouse_move.y = y;

  e->mouse_move = mouse_move;

  return e;
}

MbEvent *
create_keyboard_event (MbEventType type, const char *key)
{
  MbKeyboardEvent keyboard;

  MbEvent *e = (MbEvent *) malloc (sizeof (MbEvent));
  assert (e);

  keyboard.type = type;
  keyboard.key = key;

  e->keyboard = keyboard;

  return e;
}

void
notify_handler (MbEvent* evt)
{
  if (_mb_global_data.evt_handler != NULL)
  {
    _mb_global_data.evt_handler(evt);
  }
}
