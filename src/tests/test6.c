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

GMainLoop *loop;
MbMedia *media;
int ok = 0;

void handler (MbEvent *evt)
{
	switch (evt->type)
	{
		case MB_APP_INIT_DONE:
		{
			g_print ("MB_APP_INIT_DONE received\n");
      g_main_loop_quit (loop);
      break;
		}
		case MB_BEGIN:
		{
			ok = 1;
      break;
		}
    case MB_MOUSE_MOVE:
    {
      if (ok)
        mb_media_set_pos (media, evt->mouse_move.x, evt->mouse_move.y);
    }
	}
}

int main (int argc, char *argv[])
{
  int width = 800, height = 600;
  int img_width = 300, img_height = 300;
  int i;
  
  loop = g_main_loop_new (NULL, FALSE);
  if (argc < 2)
  {
		g_print ("Usage: %s <media uri>\n", argv[0]);
    g_print ("Exiting...\n");
    return -1;
  }

  mb_register_handler (handler);

  if (!mb_init_args (width, height, TRUE))
  {
    g_print ("Error on intilizing the libmicromb\n");
    mb_clean_up ();
    return -1;
  }

  g_main_loop_run (loop);
  
  media = mb_media_new ("media0", argv[1],
      width / 2 - img_width / 2, height/2 - img_height / 2, 0,
      img_width, img_height);

  mb_media_start (media);

  g_print ("Running...\n");

  sleep (10);

  mb_clean_up();
  g_print ("All done!\n");
  return 0;
}
