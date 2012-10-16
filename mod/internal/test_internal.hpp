/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Use (implemented) basic RDP orders to draw some known test pattern

*/

#if !defined(__TEST_INTERNAL_HPP__)
#define __TEST_INTERNAL_HPP__

#include "FileToGraphic.hpp"
#include "GraphicToFile.hpp"
#include "RDP/RDPGraphicDevice.hpp"

struct test_internal_mod : public internal_mod {

    char movie[1024];

    test_internal_mod( ModContext & context
                     , FrontAPI & front
                     , char * path
                     , char * movie
                     , uint16_t width
                     , uint16_t height):
            internal_mod(front, width, height)
    {
        strcpy(this->movie, path);
        strcat(this->movie, movie);
        LOG(LOG_INFO, "Playing %s", this->movie);
    }

    virtual ~test_internal_mod()
    {
    }

    virtual void rdp_input_invalidate(const Rect & rect)
    {
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
    }

    virtual void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap){
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        return;
    }

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    virtual BackEvent_t draw_event()
    {
        static unsigned i = 0;
        this->event.reset();
        int fd = ::open(this->movie, O_RDONLY);
        if(fd <= 0){
            printf("failed to open replay file %s\n", this->movie);
        }
        
        InFileTransport in_trans(fd);
        timeval now;
        gettimeofday(&now, NULL);
        RDPUnserializer reader(&in_trans, now, &this->front, this->get_screen_rect());
        this->front.send_global_palette();
        this->front.begin_update();
        while (reader.next_order()){
            printf("Reading order %u\n", i++);
            reader.interpret_order();
        }
        this->front.end_update();
        return BACK_EVENT_NONE;
    }
};

#endif
