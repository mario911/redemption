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

   Template for new SQ_Outfilename sequence class
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_SQ_OUTFILENAME_H_
#define _REDEMPTION_TRANSPORT_RIO_SQ_OUTFILENAME_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rio.h"

extern "C" {
    struct SQOutfilename {
        int         fd;
        RIO       * trans;
        timeval     start_tv;
        timeval     stop_tv;
        RIO       * tracker;
        SQ_FORMAT   format;
        char        path[1024];
        char        filename[1024];
        char        extension[12];
        char        tempnam[2048];
        unsigned    pid;
        unsigned    count;
        int         groupid;
    };

    static inline RIO_ERROR sq_m_SQOutfilename_constructor(
          SQOutfilename * self, SQ_FORMAT format, const char * path
        , const char * filename, const char * extension, const int groupid)
    {
        self->fd     = -1;
        self->trans  = NULL;
        self->count  = 0;
        self->format = format;
        self->pid    = getpid();
        if (strlen(path) > sizeof(self->path) - 1) {
            return RIO_ERROR_STRING_PATH_TOO_LONG;
        }
        strcpy(self->path, path);
        if (strlen(filename) > sizeof(self->filename) - 1) {
            return RIO_ERROR_STRING_FILENAME_TOO_LONG;
        }
        strcpy(self->filename, filename);

        if (strlen(extension) > sizeof(self->extension) - 1) {
            return RIO_ERROR_STRING_EXTENSION_TOO_LONG;
        }
        strcpy(self->extension, extension);
        self->groupid = groupid;
        memset(self->tempnam, 0, sizeof(self->tempnam));
        return RIO_ERROR_OK;
    }

    static inline size_t _sq_im_SQOutfilename_get_name(
        const SQOutfilename * self, char * buffer, size_t size, int count)
    {
        size_t res = 0;
        switch (self->format) {
        default:
        case SQF_PATH_FILE_PID_COUNT_EXTENSION:
            res = snprintf( buffer, size, "%s%s-%06u-%06u%s", self->path
                          , self->filename, self->pid, count, self->extension);
        break;
        case SQF_PATH_FILE_COUNT_EXTENSION:
            res = snprintf( buffer, size, "%s%s-%06u%s", self->path
                          , self->filename, count, self->extension);
        break;
        case SQF_PATH_FILE_PID_EXTENSION:
            res = snprintf( buffer, size, "%s%s-%06u%s", self->path
                          , self->filename, self->pid, self->extension);
        break;
        case SQF_PATH_FILE_EXTENSION:
            res = snprintf( buffer, size, "%s%s%s", self->path
                          , self->filename, self->extension);
        break;
        }
        return res;
    }

    // internal utility method, used to get name of files used for target transports
    // it is called internally, but actual goal is to enable tests to check and remove the created files afterward.
    // not a part of external sequence API
    static inline size_t sq_im_SQOutfilename_get_name(
        const SQOutfilename * self, char * buffer, size_t size, int count)
    {
        size_t res = 0;
        if (!self->tempnam[0] || ((unsigned)count != self->count)) {
            return _sq_im_SQOutfilename_get_name(self, buffer, size, count);
        }
        else if (size > 0) {
            strncpy(buffer, self->tempnam, size - 1);
            buffer[size - 1] = 0;
        }
        return res;
    }

    static RIO_ERROR sq_m_SQOutfilename_timestamp(SQOutfilename * self, timeval * tv)
    {
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQOutfilename_destructor(SQOutfilename * self)
    {
        if (self->trans) {
            char tmpname[1024];
            _sq_im_SQOutfilename_get_name(self, tmpname, sizeof(tmpname), self->count);
            rio_delete(self->trans);
            int res = close(self->fd);
            if (res < 0) {
                LOG(LOG_ERR, "closing file failed erro=%u : %s\n", errno, strerror(errno));
                return RIO_ERROR_CLOSE_FAILED;
            }
            // LOG(LOG_INFO, "\"%s\" -> \"%s\".", self->tempnam, tmpname);
            res = rename(self->tempnam, tmpname);
            if (res < 0) {
                LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                   , self->tempnam, tmpname, errno, strerror(errno));
                return RIO_ERROR_RENAME;
            }
            memset(self->tempnam, 0, sizeof(self->tempnam));
            self->trans = NULL;
        }
        return RIO_ERROR_CLOSED;
    }

    static inline RIO * sq_m_SQOutfilename_get_trans(SQOutfilename * self, RIO_ERROR * status)
    {
        if (status && (*status != RIO_ERROR_OK)) { return self->trans; }
        if (!self->trans) {
            snprintf(self->tempnam, sizeof(self->tempnam), "%sred-XXXXXX.tmp", self->path);
            TODO("add rights information to constructor");
            self->fd = ::mkostemps(self->tempnam, 4, O_WRONLY | O_CREAT);
            if (self->fd < 0) {
                if (status) { *status = RIO_ERROR_CREAT; }
                return self->trans;
            }
            if (chmod( self->tempnam
                     , (self->groupid ? (S_IRUSR | S_IRGRP) : S_IRUSR)) == -1) {
                LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                   , self->tempnam, strerror(errno), errno
                   , (self->groupid ? "u+r, g+r" : "u+r"));
            }
            self->trans = rio_new_outfile(status, self->fd);
        }
        return self->trans;
    }

    static inline RIO_ERROR sq_m_SQOutfilename_next(SQOutfilename * self)
    {
        sq_m_SQOutfilename_destructor(self);
        self->count += 1;
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQOutfilename_get_chunk_info(
          SQOutfilename * self, unsigned * num_chunk, char * path, size_t path_len
        , timeval * begin, timeval * end)
    {
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQOutfilename_full_clear(SQOutfilename * self)
    {
        if (self->trans) {
            char tmpname[1024];
            _sq_im_SQOutfilename_get_name(self, tmpname, sizeof(tmpname), self->count);
            rio_delete(self->trans);
            int res = close(self->fd);
            if (res < 0) {
                LOG(LOG_ERR, "closing file failed erro=%u : %s\n", errno, strerror(errno));
                return RIO_ERROR_CLOSE_FAILED;
            }
            // LOG(LOG_INFO, "\"%s\" -> \"%s\".", self->tempnam, tmpname);
            res = rename(self->tempnam, tmpname);
            if (res < 0) {
                LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                   , self->tempnam, tmpname, errno, strerror(errno));
                return RIO_ERROR_RENAME;
            }
            memset(self->tempnam, 0, sizeof(self->tempnam));
            self->trans = NULL;
        }
        for (int i = static_cast<int>(self->count); i >= 0; i--) {
            char tmpname[1024];
            _sq_im_SQOutfilename_get_name(self, tmpname, sizeof(tmpname), i);
            unlink(tmpname);
        }
        return RIO_ERROR_CLOSED;
    }
};

#endif
