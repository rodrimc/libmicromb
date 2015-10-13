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
#include <unistd.h>

#include "mb.h"

int n, count = 0;

GMainLoop *loop = NULL;

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
			break;
		}
		case MB_REMOVED:
		{
			g_print ("%s has been removed from pipeline.\n", evt->media->name);
			count++;
			if (count == 3)
				g_main_loop_quit(loop);

			break;
		}
		default:
			g_printerr ("Unknown event received!\n");
	}
}
int main (int argc, char *argv[])
{
	MbMedia *medias[3];
  int i;

  if (!mb_init_args (400, 80))
   	return -1;

  mb_register_handler(handler);

  loop = g_main_loop_new (NULL, FALSE);

 	medias[0] = mb_media_new ("media1", "file:///home/rodrigocosta/workspace/"
														"Libmb/resources/video1.mp4", 0, 0, 0, 128, 78);

 	medias[1] = mb_media_new ("media2", "file:///home/rodrigocosta/workspace/"
														"Libmb/resources/video2.mkv", 130, 0, 0, 128, 78);

 	medias[2] = mb_media_new ("media3", "file:///home/rodrigocosta/workspace/"
														"Libmb/resources/video3.avi", 260, 0, 0, 128, 78);


  mb_media_start(medias[0]);
  mb_media_start(medias[1]);
  mb_media_start(medias[2]);

  sleep (20);

  mb_media_stop(medias[0]);
	mb_media_stop(medias[1]);
	mb_media_stop(medias[2]);

	g_print ("Running...\n");
	g_main_loop_run (loop);

  for (i = 0; i < 3; i++)
  	mb_media_free(medias[i]);

  mb_clean_up();

  g_print ("All done!\n");

  return 0;
}
