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




int main (int argc, char *argv[])
{
	MbMedia *medias[3];
	//GMainLoop *loop;
  int i;

  if (!mb_init_args (400, 80))
   	return -1;

//  loop = g_main_loop_new (NULL, FALSE);

 	medias[0] = mb_media_new ("media1", "file:///home/rodrigocosta/workspace/"
														"Libmb/resources/img1.jpg", 0, 0, 0, 128, 78);

 	medias[1] = mb_media_new ("media2", "file:///home/rodrigocosta/workspace/"
 														"Libmb/resources/img2.jpg", 130, 0, 0, 128, 78);

 	medias[2] = mb_media_new ("media3", "file:///home/rodrigocosta/workspace/"
 	 													"Libmb/resources/img3.jpg", 260, 0, 0, 128, 78);

  mb_media_start(medias[0]);
  mb_media_start(medias[1]);
  mb_media_start(medias[2]);

  sleep (2);

  mb_media_stop(medias[0]);
	mb_media_stop(medias[1]);
	mb_media_stop(medias[2]);

  sleep (2);

  for (i = 0; i < 3; i++)
  	mb_media_free(medias[i]);

  mb_clean_up();

  g_print ("All done!\n");

  return 0;
}
