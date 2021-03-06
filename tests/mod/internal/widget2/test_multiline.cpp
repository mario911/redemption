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
#define BOOST_TEST_MODULE TestWidgetMultiLine
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/multiline.hpp"
#include "internal/widget2/screen.hpp"
// #include "internal/widget2/widget_composite.hpp"
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

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;
    int xtext = 4;
    int ytext = 1;

    TODO("I believe users of this widget may wish to control text position and behavior inside rectangle"
         "ie: text may be centered, aligned left, aligned right, or even upside down, etc"
         "these possibilities (and others) are supported in RDPGlyphIndex")
    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color, xtext, ytext);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "multiline.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x06\x2b\x02\xe5\x16\x1e\x08\xd2\xe8\x66"
                   "\xb6\x9b\xeb\xad\xdb\x98\xa3\x2b\x75\x7b"
    // "\xee\xa2\x81\x4c\x50\xf0\x0d\x1e\x13\x42\x3e\xa2\x08\xf8\xc6\x7c\xea\x1d\x84\x87"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine2)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 100;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x44\x7f\x61\x29\x32\x6d\x8b\x64\xd3\x73"
        "\x39\xf0\x52\x64\x80\xb5\x4a\xcc\x38\x9c")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine3)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -10;
    int16_t y = 500;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x2c\xca\x26\x71\x79\xec\x2d\x34\xa5\x49"
        "\xe9\xc2\xd8\x50\x6c\xda\x50\xb7\xfb\x41")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine4)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 770;
    int16_t y = 500;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xf4\x3a\x68\xcd\xe4\x9d\x3f\xc4\x9c\x29"
        "\x96\x88\x7f\xcc\xf5\x2a\xb0\x6e\xa1\xeb")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine5)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -20;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x67\x7a\x68\x4e\x01\x53\xbf\xa1\x6b\x23"
        "\x0f\x73\xb9\x31\x75\x36\xe5\xf5\xcb\x9a")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine6)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x7f\xc3\xfa\x61\x18\x42\x67\x11\xac\xa2"
        "\xbd\xc6\x7f\x75\x1c\x48\xb4\x38\x30\x92")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLineClip)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wmultiline.rdp_input_invalidate(Rect(20 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xce\x65\x54\x35\xed\x19\x84\x1d\x48\xfd"
        "\x64\xc0\x4f\xfc\xd9\x56\x24\x93\xa2\x4d")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLineClip2)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmultiline.rdp_input_invalidate(Rect(20 + wmultiline.dx(),
                                         5 + wmultiline.dy(),
                                         30,
                                         10));

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x03\xe6\x2d\xc8\x57\x6b\x08\x5f\xb4\xef"
        "\x12\x29\x98\x00\xa9\xe3\xc0\xb4\x08\xa7")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLineTooLong)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = CYAN;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;

    WidgetMultiLine wmultiline(drawable, x, y, parent, notifier,
                               "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                               "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                               "Nam non magna sit amet dui vestibulum feugiat.<br>"
                               "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                               "Nam lacinia purus luctus ante congue facilisis.<br>"
                               "Donec sodales mauris luctus ante ultrices blandit.",
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmultiline.rdp_input_invalidate(wmultiline.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "multiline9.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x8f\xc4\x4d\xb3\xb7\xcd\x94\x7f\xc9\xcc"
        "\xda\x3c\x50\xe5\xd1\x45\x3b\x58\xb7\xda")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}
