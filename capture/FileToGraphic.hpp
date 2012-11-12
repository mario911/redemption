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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Jonathan Poelen

   RDPGraphicDevice is an abstract class that describe a device able to
   proceed RDP Drawing Orders. How the drawing will be actually done
   depends on the implementation.
   - It may be sent on the wire,
   - Used to draw on some internal bitmap,
   - etc.

*/

#if !defined(__FILE_TO_GRAPHIC_HPP__)
#define __FILE_TO_GRAPHIC_HPP__

#include <sys/time.h>
#include <ctime>

#include "transport.hpp"
#include "RDP/caches/bmpcache.hpp"
#include "RDP/RDPGraphicDevice.hpp"
#include "RDP/RDPDrawable.hpp"
#include "RDP/RDPSerializer.hpp"
#include "difftimeval.hpp"
#include "png.hpp"
#include "meta_file.hpp"


class InChunkedImageTransport : public Transport {
    uint16_t chunk_type; 
    uint32_t chunk_size;
    uint16_t chunk_count;
    Transport * trans;
    BStream stream;
public:
    InChunkedImageTransport(uint16_t chunk_type, uint32_t chunk_size, Transport * trans) 
        : chunk_type(chunk_type)
        , chunk_size(chunk_size)
        , chunk_count(1)
        , trans(trans) 
    {
        this->stream.init(this->chunk_size - 8);
        this->trans->recv(&stream.end, this->chunk_size - 8);
    }
    
    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        size_t total_len = 0;
        while (total_len < len){
            if (static_cast<size_t>(stream.end - stream.p) >= static_cast<size_t>(len - total_len)){
                stream.in_copy_bytes(*pbuffer + total_len, len - total_len);
                total_len += len - total_len;
                *pbuffer += len;
                return;
            }
            uint16_t remaining = stream.end - stream.p;
            stream.in_copy_bytes(*pbuffer + total_len, remaining);
            total_len += remaining;
            switch (this->chunk_type){
            case PARTIAL_IMAGE_CHUNK:
            {
                BStream header(8);
                this->trans->recv(&header.end, 8);
                this->chunk_type = header.in_uint16_le();
                this->chunk_size = header.in_uint32_le();
                this->chunk_count = header.in_uint16_le();
                this->stream.init(this->chunk_size - 8);
                this->trans->recv(&this->stream.end, this->chunk_size - 8);
            }
            break;
            case LAST_IMAGE_CHUNK:
                LOG(LOG_ERR, "Failed to read embedded image from WRM (transport closed)");
                throw Error(ERR_TRANSPORT_NO_MORE_DATA);
            break;
            default:
                LOG(LOG_ERR, "Failed to read embedded image from WRM");
                throw Error(ERR_TRANSPORT_READ_FAILED);
            break;
            }
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error)
    {
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV);
    }
};

struct FileToGraphic
{
    enum {
        HEADER_SIZE = 8,
    };
    BStream stream;

    Transport * trans;

    Rect screen_rect;

    // Internal state of orders
    RDPOrderCommon common;
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPScrBlt scrblt;
    RDPOpaqueRect opaquerect;
    RDPMemBlt memblt;
    RDPLineTo lineto;
    RDPGlyphIndex glyphindex;

    BmpCache * bmp_cache;

    // variables used to read batch of orders "chunks"
    uint32_t chunk_size;
    uint16_t chunk_type;
    uint16_t chunk_count;
    uint16_t remaining_order_count;

    timeval synctime_now;
    timeval record_now;

    uint16_t nbconsumers;
    RDPGraphicDevice * consumers[10];

    uint16_t nbdrawables;
    RDPDrawable * drawables[10];
    
    bool meta_ok;
    bool timestamp_ok;
    bool real_time;

    BGRPalette palette;

    DataMetaFile data_meta;
    
    const timeval begin_capture;
    const timeval end_capture;

    FileToGraphic(Transport * trans, const timeval begin_capture, const timeval end_capture, bool real_time = false)
    : stream(65536), trans(trans),
     // Internal state of orders
    common(RDP::PATBLT, Rect(0, 0, 1, 1)),
    destblt(Rect(), 0),
    patblt(Rect(), 0, 0, 0, RDPBrush()),
    scrblt(Rect(), 0, 0, 0),
    opaquerect(Rect(), 0),
    memblt(0, Rect(), 0, 0, 0, 0),
    lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
    glyphindex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)""),
    bmp_cache(NULL),
    // variables used to read batch of orders "chunks"
    chunk_size(0),
    chunk_type(0),
    chunk_count(0),
    remaining_order_count(0),
    nbconsumers(0),
    nbdrawables(0),
    meta_ok(false),
    timestamp_ok(false),
    real_time(real_time),
    data_meta(),
    begin_capture(begin_capture),
    end_capture(end_capture)
    {
        init_palette332(this->palette); // We don't really care movies are always 24 bits for now
        
        while (this->next_order()){
            this->interpret_order();
            if (this->meta_ok && this->timestamp_ok){
                break;
            }
        }
    }

    ~FileToGraphic()
    {
        delete this->bmp_cache;
    }

    void add_consumer(RDPGraphicDevice * consumer)
    {
        this->consumers[this->nbconsumers++] = consumer;
    }
    TODO("I believe we should only have one shared drawable for all customers. If it's actually so it will be both simpler and more efficient")
    void add_consumer(RDPDrawable * consumer)
    {
        this->drawables[this->nbdrawables++] = consumer;
    }

    bool next_order()
    REDOC("order count set this->stream.p to the beginning of the next order."
          "Most of the times it means not changing it, except when it must read next chunk"
          "when remaining order count is 0."
          "It update chunk headers (merely remaining orders count) and"
          " reads the next chunk if necessary.") 
    {   
        if (this->chunk_type != LAST_IMAGE_CHUNK 
        && this->chunk_type != PARTIAL_IMAGE_CHUNK){
            if ((this->stream.p == this->stream.end) 
            && (this->remaining_order_count)){
                LOG(LOG_ERR, "Incomplete order batch at chunk %u "
                             "order [%u/%u] "
                             "remaining [%u/%u]",
                             this->chunk_type,
                             (this->chunk_count-this->remaining_order_count), this->chunk_count,
                             (this->stream.end - this->stream.p), this->chunk_size);
                return false;
            }
            if ((this->stream.p != this->stream.end) 
            && (this->remaining_order_count == -1)){
                LOG(LOG_ERR, "Incomplete order batch at chunk %u "
                             "order [%u/%u] "
                             "remaining [%u/%u]",
                             this->chunk_type,
                             (this->chunk_count-this->remaining_order_count), this->chunk_count,
                             (this->stream.end - this->stream.p), this->chunk_size);
                return false;
            }
        }

        if (!this->remaining_order_count){
            try {
                BStream header(HEADER_SIZE);
                this->trans->recv(&header.end, HEADER_SIZE);
                this->chunk_type = header.in_uint16_le();
                this->chunk_size = header.in_uint32_le();
                this->remaining_order_count = this->chunk_count = header.in_uint16_le();

                if (this->chunk_type != LAST_IMAGE_CHUNK 
                && this->chunk_type != PARTIAL_IMAGE_CHUNK){
                    if (this->chunk_size > 65536){
                        return false;
                    }
                    this->stream.reset();
                    this->trans->recv(&this->stream.end, this->chunk_size - HEADER_SIZE);
                }
            }
            catch (Error & e){
                LOG(LOG_INFO,"receive error %u : end of transport", e.id);
                // receive error, end of transport
                return false;
            }
        }
        if (this->remaining_order_count > 0){this->remaining_order_count--;}
        return true;
    }

    void interpret_order()
    {
        switch (this->chunk_type){
        case RDP_UPDATE_ORDERS:
        {
            if (!this->meta_ok){
                LOG(LOG_ERR,"Drawing orders chunk must be preceded by a META chunk to get drawing device size");
                throw Error(ERR_WRM);
            }
            if (!this->timestamp_ok){
                LOG(LOG_ERR,"Drawing orders chunk must be preceded by a TIMESTAMP chunk to get drawing timing\n");
                throw Error(ERR_WRM);
            }
            uint8_t control = this->stream.in_uint8();
            if (!control & RDP::STANDARD){
                /* error, this should always be set */
                LOG(LOG_ERR, "Non standard order detected : protocol error");
                throw Error(ERR_WRM);
            }
            else if (control & RDP::SECONDARY) {
                using namespace RDP;
                RDPSecondaryOrderHeader header(this->stream);
                uint8_t *next_order = this->stream.p + header.length + 7;
                switch (header.type) {
                case TS_CACHE_BITMAP_COMPRESSED:
                case TS_CACHE_BITMAP_UNCOMPRESSED:
                {
                    RDPBmpCache cmd;
                    cmd.receive(this->stream, control, header, this->palette);
                    this->bmp_cache->put(cmd.id, cmd.idx, cmd.bmp);
                }
                break;
                case TS_CACHE_COLOR_TABLE:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_COLOR_TABLE (%d)", header.type);
//                    this->process_colormap(this->stream, control, header, mod);
                    break;
                case TS_CACHE_GLYPH:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_GLYPH (%d)", header.type);
//                    this->rdp_orders_process_fontcache(this->stream, header.flags, mod);
                    break;
                case TS_CACHE_BITMAP_COMPRESSED_REV2:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV2 (%d)", header.type);
                  break;
                case TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_UNCOMPRESSED_REV2 (%d)", header.type);
                  break;
                case TS_CACHE_BITMAP_COMPRESSED_REV3:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV3 (%d)", header.type);
                  break;
                default:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER (%d)", header.type);
                    /* error, unknown order */
                    break;
                }
                stream.p = next_order;
            }
            else {
                RDPPrimaryOrderHeader header = this->common.receive(this->stream, control);
                const Rect & clip = (control & RDP::BOUNDS)?this->common.clip:this->screen_rect;
                switch (this->common.order) {
                case RDP::GLYPHINDEX:
                    this->glyphindex.receive(this->stream, header);
                    for (size_t i = 0; i < this->nbconsumers ; i++){
                        this->consumers[i]->draw(this->glyphindex, clip);
                    }
                    for (size_t i = 0; i < this->nbdrawables ; i++){
                        this->drawables[i]->draw(this->glyphindex, clip);
                    }
                    break;
                case RDP::DESTBLT:
                    this->destblt.receive(this->stream, header);
                    for (size_t i = 0; i < this->nbconsumers ; i++){
                        this->consumers[i]->draw(this->destblt, clip);
                    }
                    for (size_t i = 0; i < this->nbdrawables ; i++){
                        this->drawables[i]->draw(this->destblt, clip);
                    }
                    break;
                case RDP::PATBLT:
                    this->patblt.receive(this->stream, header);
                    for (size_t i = 0; i < this->nbconsumers ; i++){
                        this->consumers[i]->draw(this->patblt, clip);
                    }
                    for (size_t i = 0; i < this->nbdrawables ; i++){
                        this->drawables[i]->draw(this->patblt, clip);
                    }
                    break;
                case RDP::SCREENBLT:
                    this->scrblt.receive(this->stream, header);
                    for (size_t i = 0; i < this->nbconsumers ; i++){
                        this->consumers[i]->draw(this->scrblt, clip);
                    }
                    for (size_t i = 0; i < this->nbdrawables ; i++){
                        this->drawables[i]->draw(this->scrblt, clip);
                    }
                    break;
                case RDP::LINE:
                    this->lineto.receive(this->stream, header);
                    for (size_t i = 0; i < this->nbconsumers ; i++){
                        this->consumers[i]->draw(this->lineto, clip);
                    }
                    for (size_t i = 0; i < this->nbdrawables ; i++){
                        this->drawables[i]->draw(this->lineto, clip);
                    }
                    break;
                case RDP::RECT:
                    this->opaquerect.receive(this->stream, header);
                    for (size_t i = 0; i < this->nbconsumers ; i++){
                        this->consumers[i]->draw(this->opaquerect, clip);
                    }
                    for (size_t i = 0; i < this->nbdrawables ; i++){
                        this->drawables[i]->draw(this->opaquerect, clip);
                    }
                    break;
                case RDP::MEMBLT:
                    {
                        this->memblt.receive(this->stream, header);
                        const Bitmap * bmp = this->bmp_cache->get(this->memblt.cache_id, this->memblt.cache_idx);
                        if (!bmp){
                            LOG(LOG_ERR, "Memblt bitmap not found in cache at (%u, %u)", this->memblt.cache_id, this->memblt.cache_idx);
                            throw Error(ERR_WRM);
                        }
                        else {
                            for (size_t i = 0; i < this->nbconsumers ; i++){
                                this->consumers[i]->draw(this->memblt, clip, *bmp);
                            }
                            for (size_t i = 0; i < this->nbdrawables ; i++){
                                this->drawables[i]->draw(this->memblt, clip, *bmp);
                            }
                        }
                    }
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->common.order);
                    throw Error(ERR_WRM);
                }
            }
            }
            break;
            case TIMESTAMP:
            {
                const uint64_t ucoeff = 1000000;
                uint64_t last_movie_usec = this->record_now.tv_sec * ucoeff + this->record_now.tv_usec;
                uint64_t movie_usec = this->stream.in_uint64_le();
                this->record_now.tv_sec  = movie_usec / ucoeff; 
                this->record_now.tv_usec = movie_usec % ucoeff;

                if (!this->timestamp_ok){
                    if (this->real_time) {
                        gettimeofday(&this->synctime_now, 0);
                    }
                    else {
                        this->synctime_now = this->record_now;
                    }
                    this->timestamp_ok = true;
                }
                else {
                    if (this->real_time){
                        struct timeval now;
                        gettimeofday(&now, 0);
                        uint64_t elapsed = difftimeval(now, this->synctime_now);
                        this->synctime_now = now;
                        
                        uint64_t movie_elapsed = movie_usec - last_movie_usec;
                        if (elapsed <= movie_elapsed){
                            struct timespec wtime =
                                { static_cast<uint32_t>((movie_elapsed - elapsed) / ucoeff)
                                , static_cast<uint32_t>((movie_elapsed - elapsed) % ucoeff * 1000)
                                };
                            nanosleep(&wtime, NULL);
                        }
                    }
                    else {
                        this->synctime_now = this->record_now; 
                    }
                }
            }
            break;
            case META_FILE:
            TODO("meta should contain some WRM version identifier")
            TODO("Cache meta_data (sizes, number of entries) should be put in META chunk")
            {
                uint16_t version = this->stream.in_uint16_le();
                (void)version; // for now there is only one, we do not yet have problems
                uint16_t width = this->stream.in_uint16_le();
                uint16_t height = this->stream.in_uint16_le();
                uint16_t bpp    =  this->stream.in_uint16_le();
                uint16_t small_entries = this->stream.in_uint16_le();
                uint16_t small_size = this->stream.in_uint16_le();
                uint16_t medium_entries = this->stream.in_uint16_le();
                uint16_t medium_size = this->stream.in_uint16_le();
                uint16_t big_entries = this->stream.in_uint16_le();
                uint16_t big_size = this->stream.in_uint16_le();

                this->stream.p = this->stream.end;

                if (!this->meta_ok){
                    this->bmp_cache = new BmpCache(bpp, small_entries, small_size, medium_entries, medium_size, big_entries, big_size);
                    this->screen_rect = Rect(0, 0, width, height);
                    this->meta_ok = true;
                }
                else {
                    if (this->screen_rect.cx != width || this->screen_rect.cy != height){
                        LOG(LOG_ERR,"Inconsistant redundant meta chunk");
                        throw Error(ERR_WRM);
                    }
                }
            }
            break;
            case SAVE_STATE:
                // RDPOrderCommon common;
                this->common.order = this->stream.in_uint8();
                this->common.clip.x = this->stream.in_uint16_le();
                this->common.clip.y = this->stream.in_uint16_le();
                this->common.clip.cx = this->stream.in_uint16_le();
                this->common.clip.cy = this->stream.in_uint16_le();

                // RDPDestBlt destblt;
                this->destblt.rect.x = this->stream.in_uint16_le();
                this->destblt.rect.y = this->stream.in_uint16_le();
                this->destblt.rect.cx = this->stream.in_uint16_le();
                this->destblt.rect.cy = this->stream.in_uint16_le();
                this->destblt.rop = this->stream.in_uint8();

                // RDPPatBlt patblt;
                this->patblt.rect.x = this->stream.in_uint16_le();
                this->patblt.rect.y = this->stream.in_uint16_le();
                this->patblt.rect.cx = this->stream.in_uint16_le();
                this->patblt.rect.cy = this->stream.in_uint16_le();
                this->patblt.rop = this->stream.in_uint8();
                this->patblt.back_color = this->stream.in_uint32_le();
                this->patblt.fore_color = this->stream.in_uint32_le();
                this->patblt.brush.org_x = this->stream.in_uint8();
                this->patblt.brush.org_y = this->stream.in_uint8();
                this->patblt.brush.style = this->stream.in_uint8();
                this->patblt.brush.hatch = this->stream.in_uint8();
                this->stream.in_copy_bytes(this->patblt.brush.extra, 7);

                // RDPScrBlt scrblt;
                this->scrblt.rect.x = this->stream.in_uint16_le();
                this->scrblt.rect.y = this->stream.in_uint16_le();
                this->scrblt.rect.cx = this->stream.in_uint16_le();
                this->scrblt.rect.cy = this->stream.in_uint16_le();
                this->scrblt.rop = this->stream.in_uint8();
                this->scrblt.srcx = this->stream.in_uint16_le();
                this->scrblt.srcy = this->stream.in_uint16_le();

                // RDPOpaqueRect opaquerect;
                this->opaquerect.rect.x  = this->stream.in_uint16_le();
                this->opaquerect.rect.y  = this->stream.in_uint16_le();
                this->opaquerect.rect.cx = this->stream.in_uint16_le();
                this->opaquerect.rect.cy = this->stream.in_uint16_le();
                {
                    uint8_t red              = this->stream.in_uint8();
                    uint8_t green            = this->stream.in_uint8();
                    uint8_t blue             = this->stream.in_uint8();
                    this->opaquerect.color = red | green << 8 | blue << 16;
                }

                // RDPMemBlt memblt;
                this->memblt.cache_id = this->stream.in_uint16_le();
                this->memblt.rect.x  = this->stream.in_uint16_le();
                this->memblt.rect.y  = this->stream.in_uint16_le();
                this->memblt.rect.cx = this->stream.in_uint16_le();
                this->memblt.rect.cy = this->stream.in_uint16_le();
                this->memblt.rop = this->stream.in_uint8();
                this->memblt.srcx    = this->stream.in_uint8();
                this->memblt.srcy    = this->stream.in_uint8();
                this->memblt.cache_idx = this->stream.in_uint16_le();

                // RDPLineTo lineto;
                this->lineto.back_mode = this->stream.in_uint8();
                this->lineto.startx = this->stream.in_uint16_le();
                this->lineto.starty = this->stream.in_uint16_le();
                this->lineto.endx = this->stream.in_uint16_le();
                this->lineto.endy = this->stream.in_uint16_le();
                this->lineto.back_color = this->stream.in_uint32_le();
                this->lineto.rop2 = this->stream.in_uint8();
                this->lineto.pen.style = this->stream.in_uint8();
                this->lineto.pen.width = this->stream.in_sint8();
                this->lineto.pen.color = this->stream.in_uint32_le();

                // RDPGlyphIndex glyphindex;
                this->glyphindex.cache_id  = this->stream.in_uint8();
                this->glyphindex.fl_accel  = this->stream.in_sint16_le();
                this->glyphindex.ui_charinc  = this->stream.in_sint16_le();
                this->glyphindex.f_op_redundant = this->stream.in_sint16_le();
                this->glyphindex.back_color = this->stream.in_uint32_le();
                this->glyphindex.fore_color = this->stream.in_uint32_le();
                this->glyphindex.bk.x  = this->stream.in_uint16_le();
                this->glyphindex.bk.y  = this->stream.in_uint16_le();
                this->glyphindex.bk.cx = this->stream.in_uint16_le();
                this->glyphindex.bk.cy = this->stream.in_uint16_le();
                this->glyphindex.op.x  = this->stream.in_uint16_le();
                this->glyphindex.op.y  = this->stream.in_uint16_le();
                this->glyphindex.op.cx = this->stream.in_uint16_le();
                this->glyphindex.op.cy = this->stream.in_uint16_le();
                this->glyphindex.brush.org_x = this->stream.in_uint8();
                this->glyphindex.brush.org_y = this->stream.in_uint8();
                this->glyphindex.brush.style = this->stream.in_uint8();
                this->glyphindex.brush.hatch = this->stream.in_uint8();
                this->stream.in_copy_bytes(this->glyphindex.brush.extra, 7);
                this->glyphindex.glyph_x = this->stream.in_sint16_le();
                this->glyphindex.glyph_y = this->stream.in_sint16_le();
                this->glyphindex.data_len = this->stream.in_uint8();
                this->stream.in_copy_bytes(this->glyphindex.data, 256);
            break;

            case LAST_IMAGE_CHUNK:
            case PARTIAL_IMAGE_CHUNK:
            {
                InChunkedImageTransport chunk_trans(this->chunk_type, this->chunk_size, this->trans);

                if (this->nbdrawables){
                    ::transport_read_png24(&chunk_trans, this->drawables[0]->drawable.data,
                                 this->drawables[0]->drawable.width, 
                                 this->drawables[0]->drawable.height,
                                 this->drawables[0]->drawable.rowsize
                                );
                    for (size_t i = 1 ; i < this->nbdrawables ; i++){
                        unsigned char * row = this->drawables[0]->drawable.data;
                        for (size_t k = 0 ; k < this->drawables[0]->drawable.height ; ++k) {
                            this->drawables[i]->set_row(k, row);
                            row += this->drawables[0]->drawable.rowsize;
                        }
                    }
                }
                else {
                    REDOC("If no drawable is available ignore images chunks");
                    this->stream.init(this->chunk_size - HEADER_SIZE);
                    this->trans->recv(&this->stream.end, this->chunk_size - HEADER_SIZE);
                }
                this->remaining_order_count = 0;
            }
            break;
            default:
                LOG(LOG_ERR, "unknown chunk type %d", this->chunk_type);
                throw Error(ERR_WRM);
            break;
        }
    }
    
    void play()
    {
        bool send_initial_image = true;
        if ((begin_capture.tv_sec == 0) 
            ||(begin_capture.tv_sec > this->record_now.tv_sec)
            || (begin_capture.tv_sec == this->record_now.tv_sec && begin_capture.tv_usec > this->record_now.tv_usec)){
                send_initial_image = false;
        }

        while (this->next_order()){
            this->interpret_order();
            if ((begin_capture.tv_sec == 0) 
            || (begin_capture.tv_sec > this->record_now.tv_sec)
            || (begin_capture.tv_sec == this->record_now.tv_sec && begin_capture.tv_usec > this->record_now.tv_usec)){
                if (send_initial_image){
                    send_initial_image = false;
                }
                for (size_t i = 0; i < this->nbconsumers ; i++){
                    this->consumers[i]->snapshot(this->record_now, 0, 0, true, false);
                }
                for (size_t i = 0; i < this->nbdrawables ; i++){
                    this->drawables[i]->snapshot(this->record_now, 0, 0, true, false);
                }
            }
            if (end_capture.tv_sec 
            && ((end_capture.tv_sec > this->record_now.tv_sec)
               || (begin_capture.tv_sec == this->record_now.tv_sec && begin_capture.tv_usec > this->record_now.tv_usec))){
                break;
            }
        }
    }
};

#endif