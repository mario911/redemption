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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetButton
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/button.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/composite.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
# define FIXTURES_PATH
#endif

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetButton)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;
    int xtext = 4;
    int ytext = 1;

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test1", auto_resize, id, fg_color, bg_color, xtext, ytext);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0, 0, wbutton.cx(), wbutton.cy()));


    //drawable.save_to_png(OUTPUT_FILE_PATH "button.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x7d\xfe\xb4\x41\x31\x06\x68\xe8\xbb\x75"
        "\x8c\x35\x11\x19\x97\x2a\x16\x0f\x65\x28")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton2)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 100;

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test2", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x2f\xea\x44\x73\x16\x6d\xea\xcf\xa0\xf6"
        "\xc2\x89\x1f\xec\xf3\xb4\xb7\xba\x92\x1b")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton3)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -10;
    int16_t y = 500;

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test3", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xe2\x04\x73\x0e\x1b\xa8\xb9\x28\x5b\x31"
        "\x50\x60\x43\x67\x2c\x71\xbe\xc3\x7d\x4a")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton4)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 770;
    int16_t y = 500;

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test4", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x0e\x13\x6c\x6a\x14\x0b\x5b\x1a\x51\x01"
        "\x17\xff\x5a\xf3\xd5\x09\x2c\xc1\x78\x75")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton5)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -20;
    int16_t y = -7;

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test5", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x5b\xb1\x08\xa2\x09\xd1\x50\x87\x24\xc1"
        "\x5e\xfb\x62\x51\x47\x93\xaa\xc7\x92\xb2")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton6)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xed\x10\xc4\xa5\x1a\x55\x26\x9d\xca\x2e"
        "\x78\x21\x2a\x38\x83\x16\x44\x9d\x0d\xad")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButtonClip)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x42\xe2\xf6\xef\xb6\x1d\xbf\x59\x8a\x39"
        "\x55\x39\x23\x66\x1b\xee\x85\xe6\x0f\xe9")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButtonClip2)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.dx(),
                                      5 + wbutton.dy(),
                                      30,
                                      10));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x11\x66\x8f\x68\x02\x7d\xb5\xa7\xec\xcd"
        "\xdf\x0d\x32\xd0\x68\x9f\x25\x35\x07\xc9")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButtonDownAndUp)
{
    TestDraw drawable(800, 600);

    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 10;
    int xtext = 4;
    int ytext = 1;

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color, xtext, ytext);

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "button9.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x96\x86\xc7\x47\xc4\x9d\x08\xbd\xf1\x6e"
        "\x81\xaf\xc3\xcb\xeb\xfa\x31\x4d\x02\x71")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, NULL);
    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button10.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x0e\x0d\x15\x27\x90\x5a\x23\x3a\xa3\x6d\x56\x31\x3a\xfe\x6d\x72\x39\x1a\x7e\x5a"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 15, 15, NULL);
    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button11.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xa1\xf6\x94\xe3\x14\x79\xf7\x1f\xbf\x58\x88\x5f\x24\x4e\x7a\x2e\xd2\x33\x66\xee"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButtonEvent)
{
    TestDraw drawable(800, 600);

    struct WidgetReceiveEvent : public Widget2 {
        Widget2* sender;
        NotifyApi::notify_event_t event;

        WidgetReceiveEvent(TestDraw& drawable)
        : Widget2(drawable, Rect(), *this, NULL)
        , sender(0)
        , event(0)
        {}

        virtual void draw(const Rect&)
        {}

        virtual void notify(Widget2* sender, NotifyApi::notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } widget_for_receive_event(drawable);

    struct Notify : public NotifyApi {
        Widget2* sender;
        notify_event_t event;

        Notify()
        : sender(0)
        , event(0)
        {
        }
        virtual void notify(Widget2* sender, notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Widget2& parent = widget_for_receive_event;
    bool auto_resize = false;
    int16_t x = 0;
    int16_t y = 0;

    WidgetButton wbutton(drawable, x, y, parent, &notifier, "", auto_resize);

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == &wbutton);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_KEY);
    keymap.push_char('a');
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_KEY);
    keymap.push_char(' ');
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == &wbutton);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == &wbutton);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;
}

BOOST_AUTO_TEST_CASE(TraceWidgetButtonAndComposite)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    WidgetComposite wcomposite(drawable, Rect(0,0,800,600), parent, notifier);

    WidgetButton wbutton1(drawable, 0,0, wcomposite, notifier,
                        "abababab", true, 0, YELLOW, BLACK);
    WidgetButton wbutton2(drawable, 0,100, wcomposite, notifier,
                        "ggghdgh", true, 0, WHITE, RED);
    WidgetButton wbutton3(drawable, 100,100, wcomposite, notifier,
                        "lldlslql", true, 0, BLUE, RED);
    WidgetButton wbutton4(drawable, 300,300, wcomposite, notifier,
                        "LLLLMLLM", true, 0, PINK, DARK_GREEN);
    WidgetButton wbutton5(drawable, 700,-10, wcomposite, notifier,
                        "dsdsdjdjs", true, 0, LIGHT_GREEN, DARK_BLUE);
    WidgetButton wbutton6(drawable, -10,550, wcomposite, notifier,
                        "xxwwp", true, 0, DARK_GREY, PALE_GREEN);

    wcomposite.add_widget(&wbutton1);
    wcomposite.add_widget(&wbutton2);
    wcomposite.add_widget(&wbutton3);
    wcomposite.add_widget(&wbutton4);
    wcomposite.add_widget(&wbutton5);
    wcomposite.add_widget(&wbutton6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button12.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x27\xaa\x91\x51\x0b\x39\xf7\xf1\xfd\x55"
        "\x4f\xb0\x33\xac\x7a\x45\x56\x16\x69\x12")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button13.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x6b\x18\x4b\x47\x59\xd9\xca\xe7\xe4\xd1"
        "\x57\x26\x23\x8d\x10\x48\x26\x8e\x6d\xcf")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wcomposite.clear();
}


BOOST_AUTO_TEST_CASE(TraceWidgetButtonFocus)
{
    TestDraw drawable(70, 40);

    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 10;
    int xtext = 4;
    int ytext = 1;

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test7", auto_resize, id, fg_color, bg_color, xtext, ytext);

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "button14.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xc5\x83\xfc\x1a\x58\xc0\xc7\xfb\xcd\x10\x54\x90\xf0\xd4\x7d\x4b\x2d\x88\x40\x5f")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.focus();

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "button15.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x83\x52\xd7\xc8\xe9\x6f\x34\x44\x61\x59\x1e\x70\x38\x77\x39\x62\x96\x8e\x5e\xec"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.blur();

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "button16.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xc5\x83\xfc\x1a\x58\xc0\xc7\xfb\xcd\x10\x54\x90\xf0\xd4\x7d\x4b\x2d\x88\x40\x5f")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.focus();

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "button17.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x83\x52\xd7\xc8\xe9\x6f\x34\x44\x61\x59\x1e\x70\x38\x77\x39\x62\x96\x8e\x5e\xec"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

