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

#include <stdio.h>
#include <stdlib.h>
#include <gst/gst.h>
#include <unistd.h>

#include "mb.h"

int n, count = 0;
GMainLoop *loop;
MbMedia *medias[4];


void handler (MbMediaEvent *evt)
{
	switch (evt->evt)
	{
		case MB_BEGIN:
		{
			g_print ("%s has started.\n", evt->media->name);
			break;
		}
		case MB_PAUSE:
		{
			g_print ("%s has paused.\n", evt->media->name);
			break;
		}
		case MB_END:
		{
			g_print ("%s has ended.\n", evt->media->name);

			if (evt->media == medias[3])
			{
				mb_media_stop(medias[0]);
				mb_media_stop(medias[1]);
				mb_media_stop(medias[2]);
			}
			break;
		}
		case MB_REMOVED:
		{
			g_print ("%s has been removed from pipeline.\n", evt->media->name);

			count++;
			if (count == n)
				g_main_loop_quit(loop);

			break;
		}
		default:
			g_printerr ("Unknown event received!\n");
	}
}

int main (int argc, char *argv[])
{
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
  int width, height, x;

  int i;
  if (argc < 2)
  {
		g_print ("Usage: %s <media uri> ...\n", argv[0]);
    g_print ("Exiting...\n");
    return -1;
  }

  if (!mb_init_args (800, 600))
   	return -1;

  loop = g_main_loop_new (NULL, FALSE);

  mb_register_handler(handler);

  width = mb_get_window_width ();
  height = mb_get_window_height ();

  printf ("%dx%d\n", width, height);

  n = argc - 1;

  for (i = 1; i <= n; i++)
  {
  	gchar alias_buff [8];
  	sprintf (alias_buff, "media_%d", i);

  	medias[i-1] = mb_media_new (alias_buff, argv[i],
																0, 0, 0, width, height);
																//width / 2 - 640 / 2, height / 2 - 480 / 2, 0,
																//640, 480);
  }

  for (i = 0; i < n; i++)
  {
  	if (!mb_media_start(medias[i]))
  	{
  		g_printerr ("Unable to start %s.\n", medias[i]->name);
  	}
  }
  mb_media_set_size(medias[0], width, height);
  mb_media_set_z (medias[0], 0);
  mb_media_set_z (medias[1], 1);
  mb_media_set_pos(medias[1], width * 0.05, height * 0.067);
  mb_media_set_size(medias[1], width * 0.45, height * 0.45);
  mb_media_set_alpha (medias[1], 1.0);

  mb_media_set_pos(medias[3], width * 0.15, height * 0.60);
  mb_media_set_size(medias[3], width * 0.25, height * 0.25);
  mb_media_set_z(medias[3], 3);

  mb_media_set_volume(medias[2], 0.09);

  sleep (10);
  mb_media_stop(medias[3]);

  g_print ("Running...\n");
	g_main_loop_run (loop);

  for (i = 0; i < n; i++)
  	mb_media_free(medias[i]);

  mb_clean_up();

  g_print ("All done!\n");

  return 0;
}
