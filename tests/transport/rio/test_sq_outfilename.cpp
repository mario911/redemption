/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestSQOutfilename
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "rio/rio.h"
#include "rio/rio_impl.h"

// Sequence are used to control chaining of chunked RIO
// Insequence provide RIOs that will be used for reading
// sq_get_trans(sq) : returns the current RIO
// sq_next(sq)      : goes to next RIO (the next call to sq_get_trans will change RIO)

// the 2 API functions below are currently tailored to the need of video traces
// providing some access to all purpose meta informations attached to chunks
// could probably be of interest.

// sq_timestamp(sq, tv) : set current timestamp, useful only for (some) output sequence
//    (the last timestamp setted will be used as the end timestamp of the current chunk if needed)
//    (and as the first timestamp of the next open chunk)
// sq_get_chunk_info(sq, &num, path, path_len, &tv_begin, &tv_end) : useful only for (some) input sequence
// returns informations on the current read chunk (the chunk currently returned by get_trans)
// num get the chunk number (1 for the first chunk)
// path returns the name of the chunk (ie: filename)
// tv_begin and tv_end are start and stop of chunk


BOOST_AUTO_TEST_CASE(TestSeqOutfilename)
{
    // get value of first available fd1
    int fd1 = ::open("TESTOFS-000000.wrm", O_WRONLY|O_CREAT, S_IRUSR|S_IRUSR);
    ::close(fd1);
    // cleanup possible remain of previous test
    ::unlink("TESTOFS-000000.wrm");
    ::unlink("TESTOFS-000001.wrm");
    ::unlink("TESTOFS-000002.wrm");

    // SQOutfilename is a Sequence for a chain of files used for writing defined by their names
    // the names follow one of the following formats:
    // SQF_PATH_FILE_PID_COUNT_EXTENSION  
    // SQF_PATH_FILE_COUNT_EXTENSION
    // prefix and extension are provided by caller
    // count is managed by the sequence (num_chunk - 1)

    // sq_next() change filename to the next one in the list by incrementing count by one and closing current trans
    // (ie: after a call to sq_next() user *must* call sq_get_trans(), the previous trans can't be used any more)
    // the next call to sq_get_trans() will open a new trans with name containing the new count

    timeval tv = {};

    RIO_ERROR status = RIO_ERROR_OK;    
    const int groupid = 0;
    SQ * sq = sq_new_outfilename(&status, SQF_PATH_FILE_COUNT_EXTENSION, "", "TESTOFS", ".wrm", groupid);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    RIO * rt = NULL;

    tv.tv_sec += 10; sq_timestamp(sq, &tv);
    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.infile.fd);
    
    tv.tv_sec += 10; sq_timestamp(sq, &tv);
    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.infile.fd);
    
    tv.tv_sec += 10; sq_timestamp(sq, &tv);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sq));

    tv.tv_sec += 10; sq_timestamp(sq, &tv);
    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.infile.fd);
    
    tv.tv_sec += 10; sq_timestamp(sq, &tv);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sq));

    tv.tv_sec += 10; sq_timestamp(sq, &tv);
    rt = sq_get_trans(sq, &status);
    BOOST_CHECK(rt != NULL);
    BOOST_CHECK_EQUAL(fd1, rt->u.infile.fd);

    tv.tv_sec += 10; sq_timestamp(sq, &tv);

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sq));

    sq_delete(sq);
    
    // tearDown: cleanup files after test
    BOOST_CHECK_EQUAL(0, ::unlink("TESTOFS-000000.wrm"));
    BOOST_CHECK_EQUAL(0, ::unlink("TESTOFS-000001.wrm"));
    BOOST_CHECK_EQUAL(0, ::unlink("TESTOFS-000002.wrm"));
    BOOST_CHECK_EQUAL(-1, ::unlink("TESTOFS-000003.wrm"));
}

