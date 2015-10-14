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

#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>

typedef enum
{
	/* FILL ME*/
  //Application internal event
  MB_APP_INIT_DONE = 0x100, 

	//Presentation
	MB_BEGIN        = 0x200,
	MB_END, 
  MB_PAUSE,
	MB_REMOVED, //media has been removed from the pipeline 
              //this means that from now on is safe to free it (mb_media_free)

  //Attribution ...
  
  //Input ...
} MbEventType;

typedef struct MbStateChangeEvent
{
  uint32_t type;
 
  const char *media_name;

} MbStateChangeEvent;

typedef union MbEvent
{
	uint32_t type; //this field should be present in all event structures

  MbStateChangeEvent state_change;

} MbEvent;

#endif //EVENTS_H
