/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   new Socket RedTransport class
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_RIO_SOCKET_H_
#define _REDEMPTION_TRANSPORT_RIO_RIO_SOCKET_H_

#include "rio.h"
#include "netutils.hpp"

extern "C" {
    struct RIOSocket {
        int sck;
    };

    /* This method does not allocate space for object itself,
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RIO_ERROR rio_m_RIOSocket_constructor(RIOSocket * self, int sck)
    {
        self->sck = sck;
        return RIO_ERROR_OK;
    }

    /* This method deallocate any space used for subfields if any
    */
    inline RIO_ERROR rio_m_RIOSocket_destructor(RIOSocket * self)
    {
        return RIO_ERROR_CLOSED;
    }

    /* This method return a signature based on the data written
    */
    static inline RIO_ERROR rio_m_RIOSocket_sign(RIOSocket * self, unsigned char * buf, size_t size, size_t * len) {
        memset(buf, 0, (size>=32)?32:size);
        *len = (size>=32)?32:size;
        return RIO_ERROR_OK;
    }

    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data, the return buffer
       has been changed but an error is returned anyway
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOSocket_recv(RIOSocket * self, void * data, size_t len)
    {
        char * pbuffer = (char*)data;
        size_t remaining_len = len;

        while (remaining_len > 0) {
            ssize_t res = ::recv(self->sck, pbuffer, remaining_len, 0);
            switch (res) {
                case -1: /* error, maybe EAGAIN */
                    if (try_again(errno)) {
                        fd_set fds;
                        struct timeval time = { 0, 100000 };
                        FD_ZERO(&fds);
                        FD_SET(self->sck, &fds);
                        select(self->sck + 1, &fds, NULL, NULL, &time);
                        continue;
                    }
                    if (len != remaining_len){
                        return len - remaining_len;
                    }
                    TODO("replace this with actual error management, EOF is not even an option for sockets");
                    rio_m_RIOSocket_destructor(self);
                    return -RIO_ERROR_EOF;
                case 0: /* no data received, socket closed */
                    // if we were not able to receive the amount of data required, this is an error
                    // not need to process the received data as it will end badly
                    rio_m_RIOSocket_destructor(self);
                    return -RIO_ERROR_EOF;
                default: /* some data received */
                    pbuffer += res;
                    remaining_len -= res;
                break;
            }
        }
        return len;
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOSocket_send(RIOSocket * self, const void * data, size_t len)
    {
        size_t total = 0;
        while (total < len) {
            ssize_t sent = ::send(self->sck, &(((uint8_t*)data)[total]), len - total, 0);
            switch (sent){
            case -1:
                if (try_again(errno)) {
                    fd_set wfds;
                    struct timeval time = { 0, 10000 };
                    FD_ZERO(&wfds);
                    FD_SET(self->sck, &wfds);
                    select(self->sck + 1, NULL, &wfds, NULL, &time);
                    continue;
                }
                rio_m_RIOSocket_destructor(self);
                return -RIO_ERROR_EOF;
            case 0:
                rio_m_RIOSocket_destructor(self);
                return -RIO_ERROR_EOF;
            default:
                total = total + sent;
            }
        }
        return len;
    }

    static inline RIO_ERROR rio_m_RIOSocket_seek(RIOSocket * self, int64_t offset, int whence)
    {
        return RIO_ERROR_SEEK_NOT_AVAILABLE;
    }

    static inline RIO_ERROR rio_m_RIOSocket_get_status(RIOSocket * self)
    {
        TODO("when we will keep error value needed for recv we should return the stored error status");
        return RIO_ERROR_OK;
    }
};

#endif

