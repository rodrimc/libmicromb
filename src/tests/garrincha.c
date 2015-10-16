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
GMainLoop *loop;
MbMedia *background;
MbMedia *anim;
MbMedia *choro;
MbMedia *drible;
MbMedia *shoes;


void handler (MbEvent *evt)
{
	switch (evt->type)
	{
    case MB_APP_INIT_DONE:
    {
      g_print ("INIT_DONE received\n");
      g_main_loop_quit(loop);
      break;
    }
		case MB_BEGIN:
		{
			g_print ("%s has started.\n", evt->state_change.media_name);
			break;
		}
		case MB_PAUSE:
		{
			g_print ("%s has paused.\n", evt->state_change.media_name);
			break;
		}
		case MB_END:
		{
			g_print ("%s has ended.\n", evt->state_change.media_name);
			if (evt->state_change.media_name == shoes->name)
			{
				mb_media_set_pos(anim, 0, 0);
				mb_media_set_size(anim, mb_get_window_width(),
													mb_get_window_height());
			}

			if (evt->state_change.media_name == anim->name)
				mb_media_stop(background);

			break;
		}
		case MB_REMOVED:
		{
			gboolean quit = FALSE;
			g_print ("%s has been removed from pipeline.\n", 
          evt->state_change.media_name);

      if (evt->state_change.media_name == background->name)
      {
        mb_media_free (background);
        quit = TRUE;
      }
      else if (evt->state_change.media_name == anim->name)
        mb_media_free (anim);
      else if (evt->state_change.media_name == choro->name)
        mb_media_free (choro);
      else if (evt->state_change.media_name == drible->name)
        mb_media_free (drible);
      else if (evt->state_change.media_name == shoes->name)
        mb_media_free (shoes);

			if (quit)
				g_main_loop_quit(loop);

			break;
		}
    case MB_MOUSE_BUTTON_PRESS:
    {
      g_print ("Mouse button press: %d (button), (%d, %d) (x, y)\n",
          evt->mouse_button.button, evt->mouse_button.x, 
          evt->mouse_button.y);
      break;
    }
    case MB_MOUSE_BUTTON_RELEASE:
    {
      g_print ("Mouse button release: %d (button), (%d, %d) (x, y)\n",
          evt->mouse_button.button, evt->mouse_button.x, 
          evt->mouse_button.y);
      break;
    }
    case MB_MOUSE_MOVE:
    {
      g_print ("Mouse move event: (%d, %d) (x, y)\n", evt->mouse_move.x,
          evt->mouse_move.y);
      break;
    }
    case MB_KEY_PRESS:
    {
      g_print ("Key pressed: %s\n", evt->keyboard.key);
      break;
    }
    case MB_KEY_RELEASE:
    {
      g_print ("Key released: %s\n", evt->keyboard.key);
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
  int width = 800, height = 600;

  mb_register_handler(handler);
  
  if (!mb_init_args (width, height, FALSE))
   	return -1;
  
  loop = g_main_loop_new (NULL, FALSE);
	g_main_loop_run (loop);
  
  n = 5;

  n = argc - 1;

  background = mb_media_new ("background", "file:////media/rodrigocosta/Dados/"
														 "primeirojoao/media/background.png", 0, 0, 0,
														 width, height);

  anim = mb_media_new ("animation", "file:////media/rodrigocosta/Dados/"
											 "primeirojoao/media/animGar.mp4", 0, 0, 1,
											 width, height);

  choro = mb_media_new ("choro", "file:////media/rodrigocosta/Dados/"
												 "primeirojoao/media/choro.mp4", 0, 0, 0, 0, 0);

  drible = mb_media_new ("drible", "file:////media/rodrigocosta/Dados/"
												 "primeirojoao/media/drible.mp4",
												 width * 0.05 /* left=5% */,
												 height * 0.067 /* top=6.7% */,
												 3 /* zIndex=3 */,
												 width * 0.185 /* width=18.5% */,
												 height * 0.185 /* height=18.5% */);

  shoes = mb_media_new ("shoes", "file:////media/rodrigocosta/Dados/"
												 "primeirojoao/media/shoes.mp4",
												 width * 0.15 /* left=15% */,
												 height * 0.60 /* top=60% */,
												 3 /* zIndex=3 */,
												 width * 0.25 /* width=25% */,
												 height * 0.25 /* height=25% */);

  mb_media_start(background);
  mb_media_start(anim);
  sleep (5);

  mb_media_start(choro);
  sleep (7);

  mb_media_start(drible);
  sleep (33);

  mb_media_set_pos(anim, width * 0.05, height * 0.067);
  mb_media_set_size(anim, width * 0.45, height * 0.45);

  mb_media_start(shoes);

  g_print ("Running...\n");
	g_main_loop_run (loop);

  mb_clean_up();

  g_print ("All done!\n");

  return 0;
}
