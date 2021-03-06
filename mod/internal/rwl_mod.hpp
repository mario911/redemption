/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen,
 *              Meng Tan
 */

#ifndef REDEMPTION_MOD_INTERNAL_RWL_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_RWL_MOD_HPP

#include "front_api.hpp"
#include "config.hpp"
#include "widget2/rwl_rectangle.hpp"
#include "internal_mod.hpp"


class RwlMod : public InternalMod, public NotifyApi
{
    Inifile & ini;

public:
    RwlMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height)
    : InternalMod(front, width, height)
    , ini(ini)
    {
        ///TODO memory leak

        RwlRectangle::Style style;
        style.color = RED;
        style.borders.top.color = BLUE;
        style.borders.top.size = 2;
        style.borders.top.type = 0;
        style.borders.left = style.borders.right = style.borders.bottom = style.borders.top;
        style.focus_borders = style.inactive_focus_borders = style.inactive_borders = style.borders;

        RwlRectangle * img = new RwlImage(*this, this->screen, 0, SHARE_PATH"/"REDEMPTION_LOGO24, style);
        img->rect.x = width - img->cx();
        img->rect.y = height - img->cy();
        this->screen.add_widget(img);

        RwlRectangle * zone = new RwlRectangle(*this, this->screen, 0, style);
        zone->rect.cx = 100;
        zone->rect.cy = 100;
        this->screen.add_widget(zone);

        zone = new RwlRectangle(*this, this->screen, 0, style);
        zone->rect.x = 100;
        zone->rect.y = 100;
        zone->rect.cx = 100;
        zone->rect.cy = 100;
        this->screen.add_widget(zone);

        style.color = YELLOW;
        zone = new RwlRectangle(*this, this->screen, 0, style);
        zone->rect.y = 100;
        zone->rect.cx = 100;
        zone->rect.cy = 100;
        this->screen.add_widget(zone);

//         this->screen.set_widget_focus(&this->window_dialog);
        this->screen.refresh(this->screen.rect);
    }

    virtual ~RwlMod()
    {
        this->screen.clear();
    }

    virtual void rdp_input_scancode(long int param1, long int param2,
                                    long int param3, long int param4, Keymap2* keymap)
    {
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
                case Keymap2::KEVENT_ESC:
                    keymap->get_kevent();
                    this->event.signal = BACK_EVENT_STOP;
                    this->event.set();
                    break;
                default:
                    InternalMod::rdp_input_scancode(param1, param2, param3, param4, keymap);
            }
        }
    }


    virtual void rdp_input_synchronize(uint32_t /*time*/, uint16_t /*device_flags*/,
                                       int16_t /*param1*/, int16_t /*param2*/)
    {
    }

    virtual void notify(Widget2* sender, notify_event_t event)
    {}

    virtual void draw_event(time_t now)
    {
        this->event.reset();
    }
};

#endif
