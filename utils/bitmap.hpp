/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni, Martin Potier,
              Meng Tan
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   This file implement the bitmap items data structure
   including RDP RLE compression and decompression algorithms

   It also features storage and color versionning of the bitmap
   returning a pointer on a table, corresponding to the required
   color model.
*/

#ifndef _REDEMPTION_UTILS_BITMAP_HPP__
#define _REDEMPTION_UTILS_BITMAP_HPP__

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>
#include <inttypes.h>
#include <error.h>
#include <errno.h>
#include <png.h>

#include "log.hpp"
#include "bitfu.hpp"
#include "colors.hpp"
#include "stream.hpp"
#include "ssl_calls.hpp"
#include "rect.hpp"

class Bitmap {
public:
    uint8_t original_bpp;
    BGRPalette original_palette;
    uint16_t cx;
    uint16_t cy;

    size_t line_size;
    size_t bmp_size;

    struct CountdownData {
        uint8_t * ptr;
        CountdownData() {
            this->ptr = 0;
        }
        ~CountdownData(){
            if (this->ptr){
                this->ptr[0]--;
                if (!this->ptr[0]){
                    free(this->ptr);
                }
            }
        }
        uint8_t * get() const {
            return this->ptr + 128;
        }
        void alloc(uint32_t size) {
            this->ptr = static_cast<uint8_t*>(malloc(size+128));
            this->ptr[0] = 1;
        }
        void use(const CountdownData & other)
        {
            this->ptr = other.ptr;
            this->ptr[0]++;
        }
    } data_bitmap;

    // Memoize compressed bitmap
    mutable uint8_t * data_compressed;
    mutable size_t data_compressed_size;

    Bitmap(uint8_t bpp, const BGRPalette * palette, uint16_t cx, uint16_t cy, const uint8_t * data, const size_t size, bool compressed=false)
        : original_bpp(bpp)
        , cx(align4(cx))
        , cy(cy)
        , line_size(this->cx * nbbytes(this->original_bpp))
        , bmp_size(this->line_size * cy)
        , data_bitmap()
        , data_compressed(NULL)
        , data_compressed_size(0)
    {
        this->data_bitmap.alloc(this->bmp_size);
//        LOG(LOG_ERR, "Creating bitmap (%p) cx=%u cy=%u size=%u bpp=%u", this, cx, cy, size, bpp);
        if (bpp == 8){
            if (palette){
                memcpy(&this->original_palette, palette, sizeof(BGRPalette));
            }
            else {
                init_palette332(this->original_palette);
            }
        }

        if (compressed) {
            this->decompress(data, cx, cy, size);
        } else {
            uint8_t * dest = this->data_bitmap.get();
            const uint8_t * src = data;
            const size_t & data_width = cx * nbbytes(bpp);
            for (uint16_t i = 0 ; i < this->cy ; i++){
                memcpy(dest, src, data_width);
                bzero(dest + this->line_size, this->line_size - data_width);
                src += data_width;
                dest += this->line_size;
            }
        }
        if (this->cx <= 0 || this->cy <= 0){
            LOG(LOG_ERR, "Bogus empty bitmap!!! cx=%u cy=%u size=%u bpp=%u", this->cx, this->cy, size, this->original_bpp);
        }
    }

    Bitmap(const Bitmap & src_bmp, const Rect & r)
        : original_bpp(src_bmp.original_bpp)
        , cx(align4(r.cx))
        , cy(r.cy)
        , line_size(this->cx * nbbytes(this->original_bpp))
        , bmp_size(this->line_size * this->cy)
        , data_bitmap()
        , data_compressed(NULL)
        , data_compressed_size(0)

    {
        this->data_bitmap.alloc(this->bmp_size);

//        LOG(LOG_ERR, "Creating bitmap (%p) extracting part cx=%u cy=%u size=%u bpp=%u", this, cx, cy, bmp_size, original_bpp);
        if (this->original_bpp == 8){
            memcpy(this->original_palette, src_bmp.original_palette, sizeof(BGRPalette));
        }

        // bitmapDataStream (variable): A variable-sized array of bytes.
        //  Uncompressed bitmap data represents a bitmap as a bottom-up,
        //  left-to-right series of pixels. Each pixel is a whole
        //  number of bytes. Each row contains a multiple of four bytes
        // (including up to three bytes of padding, as necessary).

        // In redemption we ensure a more constraint restriction to avoid padding
        // bitmap width must always be a multiple of 4

        const uint8_t Bpp = nbbytes(this->original_bpp);
        uint8_t *dest = this->data_bitmap.get();
        const uint8_t *src = src_bmp.data_bitmap.get() + src_bmp.line_size * (src_bmp.cy - r.y - this->cy) + r.x * Bpp;
        const unsigned line_to_copy = r.cx * nbbytes(src_bmp.original_bpp);

        for (unsigned i = 0; i < this->cy; i++) {
            memcpy(dest, src, line_to_copy);
            if (line_to_copy < this->line_size){
                bzero(dest + line_to_copy, this->line_size - line_to_copy);
            }
            src += src_bmp.line_size;
            dest += this->line_size;
        }
    }

    TODO("add palette support");
    Bitmap(const uint8_t * vnc_raw, uint16_t vnc_cx, uint16_t vnc_cy, uint8_t vnc_bpp, const Rect & tile)
        : original_bpp(vnc_bpp)
        , cx(align4(tile.cx))
        , cy(tile.cy)
        , line_size(align4(this->cx * nbbytes(this->original_bpp)))
        , bmp_size(this->line_size * this->cy)
        , data_bitmap()
        , data_compressed(NULL)
        , data_compressed_size(0)

    {
//        LOG(LOG_ERR, "Creating bitmap (%p) extracting part cx=%u cy=%u size=%u bpp=%u", this, cx, cy, bmp_size, original_bpp);

        this->data_bitmap.alloc(this->bmp_size);

        // raw: vnc data is a bunch of pixels of size cx * cy * nbbytes(bpp)
        // line 0 is the first line (top-up)

        // bitmapDataStream (variable): A variable-sized array of bytes.
        //  Uncompressed bitmap data represents a bitmap as a bottom-up,
        //  left-to-right series of pixels. Each pixel is a whole
        //  number of bytes. Each row contains a multiple of four bytes
        // (including up to three bytes of padding, as necessary).

        const uint8_t Bpp = nbbytes(this->original_bpp);
        const unsigned src_row_size = vnc_cx * Bpp;
        uint8_t *dest = this->data_bitmap.get();
        const uint8_t *src = vnc_raw + src_row_size * (tile.y + tile.cy - 1) + tile.x * Bpp;
        const uint16_t line_to_copy_size = tile.cx * Bpp;

        for (unsigned i = 0; i < this->cy; i++) {
            memcpy(dest, src, line_to_copy_size);
            if (line_to_copy_size < this->line_size){
                bzero(dest + line_to_copy_size, this->line_size - line_to_copy_size);
            }
            src -= src_row_size;
            dest += this->line_size;
        }
    }

    Bitmap(const char* filename)
        : original_bpp(24)
        , cx(0)
        , cy(0)
        , line_size(0)
        , bmp_size(0)
        , data_bitmap()
        , data_compressed(NULL)
        , data_compressed_size(0)

    {
        LOG(LOG_INFO, "loading bitmap %s", filename);

        openfile_t res = this->check_file_type(filename);

        if (res == OPEN_FILE_UNKNOWN) {
            LOG(LOG_ERR, "loading bitmap %s failed, Unknown format type", filename);
            throw Error(ERR_BITMAP_LOAD_UNKNOWN_TYPE_FILE);
        }
        else if (res == OPEN_FILE_PNG) {
            bool bres = this->open_png_file(filename);
            if (!bres) {
                LOG(LOG_ERR, "loading bitmap %s failed", filename);
                throw Error(ERR_BITMAP_PNG_LOAD_FAILED);
            }
        }
        else {
            BGRPalette palette1;
            char type1[4];

            /* header for bmp file */
            struct bmp_header {
                size_t size;
                unsigned image_width;
                unsigned image_height;
                short planes;
                short bit_count;
                int compression;
                int image_size;
                int x_pels_per_meter;
                int y_pels_per_meter;
                int clr_used;
                int clr_important;
                bmp_header() {
                    this->size = 0;
                    this->image_width = 0;
                    this->image_height = 0;
                    this->planes = 0;
                    this->bit_count = 0;
                    this->compression = 0;
                    this->image_size = 0;
                    this->x_pels_per_meter = 0;
                    this->y_pels_per_meter = 0;
                    this->clr_used = 0;
                    this->clr_important = 0;
                }
            } header;

            TODO(" reading of file and bitmap decoding should be kept appart  putting both together makes testing hard. And what if I want to read a bitmap from some network socket instead of a disk file ?");
            int fd =  open(filename, O_RDONLY);
            if (fd == -1) {
                LOG(LOG_ERR, "Widget_load: error loading bitmap from file [%s] %s(%u)\n", filename, strerror(errno), errno);
                throw Error(ERR_BITMAP_LOAD_FAILED);
            }


            /* read file type */
            if (read(fd, type1, 2) != 2) {
                LOG(LOG_ERR, "Widget_load: error bitmap file [%s] read error\n", filename);
                close(fd);
                throw Error(ERR_BITMAP_LOAD_FAILED);
            }
            if ((type1[0] != 'B') || (type1[1] != 'M')) {
                LOG(LOG_ERR, "Widget_load: error bitmap file [%s] not BMP file\n", filename);
                close(fd);
                throw Error(ERR_BITMAP_LOAD_FAILED);
            }

            /* read file size */
            TODO("define some stream aware function to read data from file (to update stream.end by itself). It should probably not be inside stream itself because read primitives are OS dependant, and there is not need to make stream OS dependant.");
                BStream stream(8192);
            if (read(fd, stream.get_data(), 4) < 4){
                LOG(LOG_ERR, "Widget_load: error read file size\n");
                close(fd);
                throw Error(ERR_BITMAP_LOAD_FAILED);
            }
            stream.end = stream.get_data() + 4;
            {
                TODO("Check what is this size ? header size ? used as fixed below ?");
                    /* uint32_t size = */ stream.in_uint32_le();
            }

            // skip some bytes to set file pointer to bmp header
            lseek(fd, 14, SEEK_SET);
            stream.init(8192);
            if (read(fd, stream.get_data(), 40) < 40){
                close(fd);
                LOG(LOG_ERR, "Widget_load: error read file size (2)\n");
                throw Error(ERR_BITMAP_LOAD_FAILED);
            }
            stream.end = stream.get_data() + 40;
            TODO(" we should read header size and use it to read header instead of using magic constant 40");
                header.size = stream.in_uint32_le();
            if (header.size != 40){
                LOG(LOG_INFO, "Wrong header size: expected 40, got %d", header.size);
                assert(header.size == 40);
            }

            header.image_width = stream.in_uint32_le();         // used
            header.image_height = stream.in_uint32_le();        // used
            header.planes = stream.in_uint16_le();
            header.bit_count = stream.in_uint16_le();           // used
            header.compression = stream.in_uint32_le();
            header.image_size = stream.in_uint32_le();
            header.x_pels_per_meter = stream.in_uint32_le();
            header.y_pels_per_meter = stream.in_uint32_le();
            header.clr_used = stream.in_uint32_le();            // used
            header.clr_important = stream.in_uint32_le();

            // skip header (including more fields that we do not read if any)
            lseek(fd, 14 + header.size, SEEK_SET);

            // compute pixel size (in Quartet) and read palette if needed
            int file_Qpp = 1;
            TODO(" add support for loading of 16 bits bmp from file");
                switch (header.bit_count) {
                    // Qpp = groups of 4 bytes per pixel
                case 24:
                    file_Qpp = 6;
                    break;
                case 8:
                    file_Qpp = 2;
                case 4:
                    stream.init(8192);
                    if (read(fd, stream.get_data(), header.clr_used * 4) < header.clr_used * 4){
                        close(fd);
                        throw Error(ERR_BITMAP_LOAD_FAILED);
                    }
                    stream.end = stream.get_data() + header.clr_used * 4;
                    for (int i = 0; i < header.clr_used; i++) {
                        uint8_t r = stream.in_uint8();
                        uint8_t g = stream.in_uint8();
                        uint8_t b = stream.in_uint8();
                        stream.in_skip_bytes(1); // skip alpha channel
                        palette1[i] = (b << 16)|(g << 8)|r;
                    }
                    break;
                default:
                    LOG(LOG_ERR, "Widget_load: error bitmap file [%s]"
                        " unsupported bpp %d\n", filename,
                        header.bit_count);
                    close(fd);
                    throw Error(ERR_BITMAP_LOAD_FAILED);
                }

            LOG(LOG_INFO, "loading file %d x %d x %d", header.image_width, header.image_height, header.bit_count);

            // bitmap loaded from files are always converted to 24 bits
            // this avoid palette problems for 8 bits,
            // and 4 bits is not supported in other parts of code anyway

            // read bitmap data
            {
                size_t size = (header.image_width * header.image_height * file_Qpp) / 2;
                stream.init(size);
                int row_size = (header.image_width * file_Qpp) / 2;
                int padding = align4(row_size) - row_size;
                for (unsigned y = 0; y < header.image_height; y++) {
                    int k = read(fd, stream.get_data() + y * row_size, row_size + padding);
                    if (k != (row_size + padding)) {
                        LOG(LOG_ERR, "Widget_load: read error reading bitmap file [%s] read\n", filename);
                        close(fd);
                        throw Error(ERR_BITMAP_LOAD_FAILED);
                    }
                }
                close(fd); // from now on all is in memory
                stream.end = stream.get_data() + size;
            }

            const uint8_t Bpp = 3;
            this->cx = align4(static_cast<uint16_t>(header.image_width));
            this->cy = static_cast<uint16_t>(header.image_height);
            this->line_size = this->cx * Bpp;
            this->bmp_size = this->line_size * this->cy;

            this->data_bitmap.alloc(this->bmp_size);
            uint8_t * dest = this->data_bitmap.get();

            int k = 0;
            for (unsigned y = 0; y < this->cy ; y++) {
                for (unsigned x = 0 ; x < header.image_width; x++) {
                    uint32_t pixel = 0;
                    switch (header.bit_count){
                    case 24:
                        {
                            uint8_t r = stream.in_uint8();
                            uint8_t g = stream.in_uint8();
                            uint8_t b = stream.in_uint8();
                            pixel = (b << 16) | (g << 8) | r;
                        }
                        break;
                    case 8:
                        pixel = stream.in_uint8();
                        break;
                    case 4:
                        if ((x & 1) == 0) {
                            k = stream.in_uint8();
                            pixel = (k >> 4) & 0xf;
                        }
                        else {
                            pixel = k & 0xf;
                        }
                        pixel = palette1[pixel];
                        break;
                    }

                    uint32_t px = color_decode(pixel, static_cast<uint8_t>(header.bit_count),
                                               palette1);
                    ::out_bytes_le(dest + y * this->line_size + x * Bpp, Bpp, px);
                }
                if (this->line_size > header.image_width * Bpp){
                    bzero(dest + y * this->line_size + header.image_width * Bpp,
                          this->line_size - header.image_width * Bpp);
                }
            }
        }
    }

    const uint8_t* data() const
    {
        return this->data_bitmap.get();
    }

    typedef enum {
        OPEN_FILE_UNKNOWN,
        OPEN_FILE_BMP,
        OPEN_FILE_PNG,
    } openfile_t;

    openfile_t check_file_type(const char * filename) {
        openfile_t res = OPEN_FILE_UNKNOWN;
        char type1[8];
        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
            LOG(LOG_ERR, "Widget_load: error loading bitmap from file [%s] %s(%u)\n", filename, strerror(errno), errno);
            return res;
        }
        else if (read(fd, type1, 2) != 2) {
            LOG(LOG_ERR, "Widget_load: error bitmap file [%s] read error\n", filename);
        }
        else if ((type1[0] == 'B') && (type1[1] == 'M')) {
            LOG(LOG_INFO, "Widget_load: image file [%s] is BMP file\n", filename);
            res = OPEN_FILE_BMP;
        }
        else if (read(fd, &type1[2], 6) != 6) {
            LOG(LOG_ERR, "Widget_load: error bitmap file [%s] read error\n", filename);
        }
        else if (png_check_sig(reinterpret_cast<png_bytep>(type1), 8)) {
            LOG(LOG_INFO, "Widget_load: image file [%s] is PNG file\n", filename);
            res = OPEN_FILE_PNG;
        }
        else {
            LOG(LOG_ERR, "Widget_load: error bitmap file [%s] not BMP or PNG file\n", filename);
        }
        close(fd);

        return res;
    } // openfile_t check_file_type(const char * filename)

    bool open_png_file(const char * filename) {

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr) {
            return false;
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, NULL, NULL);
            return false;
        }

        FILE * fd = fopen(filename, "rb");
        if (!fd) {
            return false;
        }
        png_init_io(png_ptr, fd);

        png_read_info(png_ptr, info_ptr);

        png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
        png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
        png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
        png_byte color_type = png_get_color_type(png_ptr, info_ptr);

        if (color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png_ptr);

        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
            png_set_gray_1_2_4_to_8(png_ptr);

        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png_ptr);

        if (bit_depth == 16)
            png_set_strip_16(png_ptr);
        else if (bit_depth < 8)
            png_set_packing(png_ptr);

        png_read_update_info(png_ptr, info_ptr);

        // THIS WORKS
        BStream stream(8192);
        int Bpp = 3;
        this->cx = static_cast<uint16_t>(width);
        this->cy = static_cast<uint16_t>(height);
        this->line_size = this->cx * Bpp;
        this->bmp_size = this->line_size * this->cy;
        stream.init(this->bmp_size);

        unsigned char * row = stream.get_data();
        for (size_t k = 0 ; k < this->cy ; ++k) {
            png_read_row(png_ptr, row, NULL);
            row += this->line_size;
        }

        stream.end = stream.get_data() + this->bmp_size;
        png_read_end(png_ptr, info_ptr);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fd);

        this->data_bitmap.alloc(this->bmp_size);
        uint8_t * dest = this->data_bitmap.get();

        for (unsigned y = 0; y < this->cy ; y++) {
            for (unsigned x = 0 ; x < this->cx; x++) {
                uint32_t pixel = 0;
                uint8_t r = stream.in_uint8();
                uint8_t g = stream.in_uint8();
                uint8_t b = stream.in_uint8();
                pixel = (r << 16) | (g << 8) | b;

                ::out_bytes_le(dest + (this->cy - y - 1) * this->line_size + x * Bpp, Bpp, pixel);
            }
        }


        return true;
    } // bool open_png_file(const char * filename)

private:



    TODO("move that function to external definition");

    const char * get_opcode(uint8_t opcode){
        enum {
            FILL    = 0,
            MIX     = 1,
            FOM     = 2,
            COLOR   = 3,
            COPY    = 4,
            MIX_SET = 6,
            FOM_SET = 7,
            BICOLOR = 8,
            SPECIAL_FGBG_1 = 9,
            SPECIAL_FGBG_2 = 10,
            WHITE = 13,
            BLACK = 14
        };

        switch (opcode){
            case FILL:
                return "FILL";
            case MIX:
                return "MIX";
            case FOM:
                return "FOM";
            case COLOR:
                return "COLOR";
            case COPY:
                return "COPY";
            case MIX_SET:
                return "MIX_SET";
            case FOM_SET:
                return "FOM_SET";
            case BICOLOR:
                return "BICOLOR";
            case SPECIAL_FGBG_1:
                return "SPECIAL_FGBG_1";
            case SPECIAL_FGBG_2:
                return "SPECIAL_FGBG_2";
            case WHITE:
                return "WHITE";
            case BLACK:
                return "BLACK";
            default:
                return "Unknown Opcode";
        };
    }

    void decompress(const uint8_t* input, uint16_t src_cx, uint16_t src_cy, size_t size) const
    {
        const uint8_t Bpp = nbbytes(this->original_bpp);
        const uint16_t & dst_cx = this->cx;
        uint8_t* pmin = this->data_bitmap.get();
        uint8_t* pmax = pmin + this->bmp_size;
        uint16_t out_x_count = 0;
        unsigned yprev = 0;
        uint8_t* out = pmin;
        const uint8_t* end = input + size;
        unsigned color1;
        unsigned color2;
        unsigned mix;
        uint8_t code;
        unsigned mask = 0;
        unsigned fom_mask = 0;
        unsigned count = 0;
        int bicolor = 0;

        color1 = 0;
        color2 = 0;
        mix = 0xFFFFFFFF;

        enum {
            FILL    = 0,
            MIX     = 1,
            FOM     = 2,
            COLOR   = 3,
            COPY    = 4,
            MIX_SET = 6,
            FOM_SET = 7,
            BICOLOR = 8,
            SPECIAL_FGBG_1 = 9,
            SPECIAL_FGBG_2 = 10,
            WHITE = 13,
            BLACK = 14
        };

        uint8_t opcode;
        uint8_t lastopcode = 0xFF;

        while (input < end) {

            // Read RLE operators, handle short and long forms
            code = input[0]; input++;

            switch (code >> 4) {
            case 0xf:
                switch (code){
                    case 0xFD:
                        opcode = WHITE;
                        count = 1;
                    break;
                    case 0xFE:
                        opcode = BLACK;
                        count = 1;
                    break;
                    case 0xFA:
                        opcode = SPECIAL_FGBG_2;
                        count = 8;
                    break;
                    case 0xF9:
                        opcode = SPECIAL_FGBG_1;
                        count = 8;
                    break;
                    case 0xF8:
                        opcode = code & 0xf;
                        assert(opcode != 11 && opcode != 12 && opcode != 15);
                        count = input[0]|(input[1] << 8);
                        count += count;
                        input += 2;
                    break;
                    default:
                        opcode = code & 0xf;
                        assert(opcode != 11 && opcode != 12 && opcode != 15);
                        count = input[0]|(input[1] << 8);
                        input += 2;
                        // Opcodes 0xFB, 0xFC, 0xFF are some unknown orders of length 1 ?
                    break;
                }
            break;
            case 0x0e: // Bicolor, short form (1 or 2 bytes)
                opcode = BICOLOR;
                count = code & 0xf;
                if (!count){
                    count = input[0] + 16; input++;
                }
                count += count;
                break;
            case 0x0d:  // FOM SET, short form  (1 or 2 bytes)
                opcode = FOM_SET;
                count = code & 0x0F;
                if (count){
                    count <<= 3;
                }
                else {
                    count = input[0] + 1; input++;
                }
            break;
            case 0x05:
            case 0x04:  // FOM, short form  (1 or 2 bytes)
                opcode = FOM;
                count = code & 0x1F;
                if (count){
                    count <<= 3;
                }
                else {
                    count = input[0] + 1; input++;
                }
            break;
            case 0x0c: // MIX SET, short form (1 or 2 bytes)
                opcode = MIX_SET;
                count = code & 0x0f;
                if (!count){
                    count = input[0] + 16; input++;
                }
            break;
            default:
                opcode = static_cast<uint8_t>(code >> 5); // FILL, MIX, FOM, COLOR, COPY
                count = code & 0x1f;
                if (!count){
                    count = input[0] + 32; input++;
                }

                assert(opcode < 5);
                break;
            }

            /* Read preliminary data */
            switch (opcode) {
            case FOM:
                mask = 1;
                fom_mask = input[0]; input++;
            break;
            case SPECIAL_FGBG_1:
                mask = 1;
                fom_mask = 3;
            break;
            case SPECIAL_FGBG_2:
                mask = 1;
                fom_mask = 5;
            break;
            case BICOLOR:
                bicolor = 0;
                color1 = this->get_pixel(Bpp, input);
                input += Bpp;
                color2 = this->get_pixel(Bpp, input);
                input += Bpp;
                break;
            case COLOR:
                color2 = this->get_pixel(Bpp, input);
                input += Bpp;
                break;
            case MIX_SET:
                mix = this->get_pixel(Bpp, input);
                input += Bpp;
            break;
            case FOM_SET:
                mix = this->get_pixel(Bpp, input);
                input += Bpp;
                mask = 1;
                fom_mask = input[0]; input++;
                break;
            default: // for FILL, MIX or COPY nothing to do here
                break;
            }

            // MAGIC MIX of one pixel to comply with crap in Bitmap RLE compression
            if ((opcode == FILL)
            && (opcode == lastopcode)
            && (out != pmin + this->line_size)){
                yprev = (out - this->line_size < pmin) ? 0 : this->get_pixel(Bpp, out - this->line_size);
                out_bytes_le(out, Bpp, yprev ^ mix);
                count--;
                out += Bpp;
                out_x_count += 1;
                if (out_x_count == dst_cx){
                    bzero(out, (dst_cx - src_cx) * Bpp);
                    out_x_count = 0;
                }
            }
            lastopcode = opcode;

//            LOG(LOG_INFO, "%s %u", this->get_opcode(opcode), count);

            /* Output body */
            while (count > 0) {
                if(out >= pmax) {
                    LOG(LOG_WARNING, "Decompressed bitmap too large. Dying.");
                    throw Error(ERR_BITMAP_DECOMPRESSED_DATA_TOO_LARGE);
                }
                yprev = (out - this->line_size < pmin) ? 0 : this->get_pixel(Bpp, out - this->line_size);

                switch (opcode) {
                case FILL:
                    out_bytes_le(out, Bpp, yprev);
                    break;
                case MIX_SET:
                case MIX:
                    out_bytes_le(out, Bpp, yprev ^ mix);
                    break;
                case FOM_SET:
                case FOM:
                    if (mask == 0x100){
                        mask = 1;
                        fom_mask = input[0]; input++;
                    }
                case SPECIAL_FGBG_1:
                case SPECIAL_FGBG_2:
                    if (mask & fom_mask){
                        out_bytes_le(out, Bpp, yprev ^ mix);
                    }
                    else {
                        out_bytes_le(out, Bpp, yprev);
                    }
                    mask <<= 1;
                    break;
                case COLOR:
                    out_bytes_le(out, Bpp, color2);
                    break;
                case COPY:
                    out_bytes_le(out, Bpp, this->get_pixel(Bpp, input));
                    input += Bpp;
                    break;
                case BICOLOR:
                    if (bicolor) {
                        out_bytes_le(out, Bpp, color2);
                        bicolor = 0;
                    }
                    else {
                        out_bytes_le(out, Bpp, color1);
                        bicolor = 1;
                    }
                break;
                case WHITE:
                    out_bytes_le(out, Bpp, 0xFFFFFFFF);
                break;
                case BLACK:
                    out_bytes_le(out, Bpp, 0);
                break;
                default:
                    assert(false);
                    break;
                }
                count--;
                out += Bpp;
                out_x_count += 1;
                if (out_x_count == dst_cx){
                    bzero(out, (dst_cx - src_cx) * Bpp);
                    out_x_count = 0;
                }
            }
        }
        return;
    }

public:

    enum {
        FLAG_NONE = 0,
        FLAG_FILL = 1,
        FLAG_MIX  = 2,
        FLAG_FOM  = 3,
        FLAG_MIX_SET = 6,
        FLAG_FOM_SET = 7,
        FLAG_COLOR = 8,
        FLAG_BICOLOR = 9,
    };

    unsigned get_pixel(const uint8_t Bpp, const uint8_t * const p) const
    {
        return in_uint32_from_nb_bytes_le(Bpp, p);
    }

    unsigned get_pixel_above(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * const p) const
    {
        return ((p-this->line_size) < pmin)
        ? 0
        : this->get_pixel(Bpp, p - this->line_size);
    }

    unsigned get_color_count(const uint8_t Bpp, const uint8_t * pmax, const uint8_t * p, unsigned color) const
    {
        unsigned acc = 0;
        while (p < pmax && this->get_pixel(Bpp, p) == color){
            acc++;
            p = p + Bpp;
        }
        return acc;
    }

    unsigned get_bicolor_count(const uint8_t Bpp, const uint8_t * pmax, const uint8_t * p, unsigned color1, unsigned color2) const
    {
        unsigned acc = 0;
        while ((p < pmax)
            && (color1 == this->get_pixel(Bpp, p))
            && (p + Bpp < pmax)
            && (color2 == this->get_pixel(Bpp, p + Bpp))) {
                acc = acc + 2;
                p = p + 2 * Bpp;
        }
        return acc;
    }

    unsigned get_fill_count(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p) const
    {
        unsigned acc = 0;
        while  (p + Bpp <= pmax) {
            unsigned pixel = this->get_pixel(Bpp, p);
            unsigned ypixel = this->get_pixel_above(Bpp, pmin, p);
            if (ypixel != pixel){
                break;
            }
            p += Bpp;
            acc += 1;
        }
        return acc;
    }

    unsigned get_mix_count(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned foreground) const
    {
        unsigned acc = 0;
        while (p + Bpp <= pmax){
            if (this->get_pixel_above(Bpp, pmin, p) ^ foreground ^ this->get_pixel(Bpp, p)){
                break;
            }
            p += Bpp;
            acc += 1;
        }
        return acc;
    }

    unsigned get_fom_count(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned foreground, bool fill) const
    {
        unsigned acc = 0;
        while (true){
            unsigned count = 0;
            while  (p + Bpp <= pmax) {
                unsigned pixel = this->get_pixel(Bpp, p);
                unsigned ypixel = this->get_pixel_above(Bpp, pmin, p);
                if (ypixel ^ pixel ^ (fill?0:foreground)){
                    break;
                }
                p += Bpp;
                count += 1;
                if (count >= 9) {
                    return acc;
                }
            }
            if (!count){
                break;
            }
            acc += count;
            fill ^= true;
        }
        return acc;
    }

    void get_fom_masks(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * p, uint8_t * mask, const unsigned count) const
    {
        unsigned i = 0;
        for (i = 0; i < count; i += 8)
        {
            mask[i>>3] = 0;
        }
        for (i = 0 ; i < count; i++, p += Bpp)
        {
            if (get_pixel(Bpp, p) != get_pixel_above(Bpp, pmin, p)){
                mask[i>>3] |= static_cast<uint8_t>(0x01 << (i & 7));
            }
        }
    }

    unsigned get_fom_count_set(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned & foreground, unsigned & flags) const
    {
        // flags : 1 = fill, 2 = MIX, 3 = (1+2) = FOM
        flags = FLAG_FILL;
        unsigned fill_count = this->get_fill_count(Bpp, pmin, pmax, p);
        if (fill_count) {
            if (fill_count < 8) {
                unsigned fom_count = this->get_fom_count(Bpp, pmin, pmax, p + fill_count * Bpp, foreground, false);
                if (fom_count){
                    flags = FLAG_FOM;
                    fill_count += fom_count;
                }
            }
            return fill_count;
        }
        // fill_count and mix_count can't match at the same time.
        // this would mean that foreground is black, and we will never set
        // it to black, as it's useless because fill_count allready does that.
        // Hence it's ok to check them independently.
        if  (p + Bpp <= pmax) {
            flags = FLAG_MIX;
            // if there is a pixel we are always able to mix (at worse we will set foreground ourself)
            foreground = this->get_pixel_above(Bpp, pmin, p) ^ this->get_pixel(Bpp, p);
            unsigned mix_count = 1 + this->get_mix_count(Bpp, pmin, pmax, p + Bpp, foreground);
            if (mix_count < 8) {
                unsigned fom_count = 0;
                fom_count = this->get_fom_count(Bpp, pmin, pmax, p + mix_count * Bpp, foreground, true);
                if (fom_count){
                    flags = FLAG_FOM;
                    mix_count += fom_count;
                }
            }
            return mix_count;
        }
        flags = FLAG_NONE;
        return 0;
    }

    TODO(" simplify and enhance compression using 1 pixel orders BLACK or WHITE.");
    void compress(Stream & outbuffer) const
    {
        if (this->data_compressed) {
            outbuffer.out_copy_bytes(this->data_compressed, this->data_compressed_size);
            return;
        }

        struct RLE_OutStream {
            Stream & stream;
            RLE_OutStream(Stream & outbuffer)
            : stream(outbuffer)
            {
            }

            // =========================================================================
            // Helper methods for RDP RLE bitmap compression support
            // =========================================================================
            void out_count(const int in_count, const int mask){
                if (in_count < 32) {
                    this->stream.out_uint8(static_cast<uint8_t>((mask << 5) | in_count));
                }
                else if (in_count < 256 + 32){
                    this->stream.out_uint8(static_cast<uint8_t>(mask << 5));
                    this->stream.out_uint8(static_cast<uint8_t>(in_count - 32));
                }
                else {
                    this->stream.out_uint8(static_cast<uint8_t>(0xf0 | mask));
                    this->stream.out_uint16_le(in_count);
                }
            }

            // Background Run Orders
            // ~~~~~~~~~~~~~~~~~~~~~

            // A Background Run Order encodes a run of pixels where each pixel in the
            // run matches the uncompressed pixel on the previous scanline. If there is
            // no previous scanline then each pixel in the run MUST be black.

            // When encountering back-to-back background runs, the decompressor MUST
            // write a one-pixel foreground run to the destination buffer before
            // processing the second background run if both runs occur on the first
            // scanline or after the first scanline (if the first run is on the first
            // scanline, and the second run is on the second scanline, then a one-pixel
            // foreground run MUST NOT be written to the destination buffer). This
            // one-pixel foreground run is counted in the length of the run.

            // The run length encodes the number of pixels in the run. There is no data
            // associated with Background Run Orders.

            // +-----------------------+-----------------------------------------------+
            // | 0x0 REGULAR_BG_RUN    | The compression order encodes a regular-form  |
            // |                       | background run. The run length is stored in   |
            // |                       | the five low-order bits of  the order header  |
            // |                       | byte. If this value is zero, then the run     |
            // |                       | length is encoded in the byte following the   |
            // |                       | order header and MUST be incremented by 32 to |
            // |                       | give the final value.                         |
            // +-----------------------+-----------------------------------------------+
            // | 0xF0 MEGA_MEGA_BG_RUN | The compression order encodes a MEGA_MEGA     |
            // |                       | background run. The run length is stored in   |
            // |                       | the two bytes following the order header      |
            // |                       | (in little-endian format).                    |
            // +-----------------------+-----------------------------------------------+

            void out_fill_count(const int in_count)
            {
                this->out_count(in_count, 0x00);
            }

            // Foreground Run Orders
            // ~~~~~~~~~~~~~~~~~~~~~

            // A Foreground Run Order encodes a run of pixels where each pixel in the
            // run matches the uncompressed pixel on the previous scanline XOR’ed with
            // the current foreground color. If there is no previous scanline, then
            // each pixel in the run MUST be set to the current foreground color (the
            // initial foreground color is white).

            // The run length encodes the number of pixels in the run.
            // If the order is a "set" variant, then in addition to encoding a run of
            // pixels, the order also encodes a new foreground color (in little-endian
            // format) in the bytes following the optional run length. The current
            // foreground color MUST be updated with the new value before writing
            // the run to the destination buffer.

            // +---------------------------+-------------------------------------------+
            // | 0x1 REGULAR_FG_RUN        | The compression order encodes a           |
            // |                           | regular-form foreground run. The run      |
            // |                           | length is stored in the five low-order    |
            // |                           | bits of the order header byte. If this    |
            // |                           | value is zero, then the run length is     |
            // |                           | encoded in the byte following the order   |
            // |                           | header and MUST be incremented by 32 to   |
            // |                           | give the final value.                     |
            // +---------------------------+-------------------------------------------+
            // | 0xF1 MEGA_MEGA_FG_RUN     | The compression order encodes a MEGA_MEGA |
            // |                           | foreground run. The run length is stored  |
            // |                           | in the two bytes following the order      |
            // |                           | header (in little-endian format).         |
            // +---------------------------+-------------------------------------------+
            // | 0xC LITE_SET_FG_FG_RUN    | The compression order encodes a "set"     |
            // |                           | variant lite-form foreground run. The run |
            // |                           | length is stored in the four low-order    |
            // |                           | bits of the order header byte. If this    |
            // |                           | value is zero, then the run length is     |
            // |                           | encoded in the byte following the order   |
            // |                           | header and MUST be incremented by 16 to   |
            // |                           | give the final value.                     |
            // +---------------------------+-------------------------------------------+
            // | 0xF6 MEGA_MEGA_SET_FG_RUN | The compression order encodes a "set"     |
            // |                           | variant MEGA_MEGA foreground run. The run |
            // |                           | length is stored in the two bytes         |
            // |                           | following the order header (in            |
            // |                           | little-endian format).                    |
            // +---------------------------+-------------------------------------------+

            void out_mix_count(const int in_count)
            {
                this->out_count(in_count, 0x01);
            }

            void out_mix_count_set(const int in_count, const uint8_t Bpp, unsigned new_foreground)
            {
                const uint8_t mask = 0x06;
                if (in_count < 16) {
                    this->stream.out_uint8(static_cast<uint8_t>(0xc0 | in_count));
                }
                else if (in_count < 256 + 16){
                    this->stream.out_uint8(0xc0);
                    this->stream.out_uint8(static_cast<uint8_t>(in_count - 16));
                }
                else {
                    this->stream.out_uint8(0xf0 | mask);
                    this->stream.out_uint16_le(in_count);
                }
                this->stream.out_bytes_le(Bpp, new_foreground);
            }

            // Foreground / Background Image Orders
            // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

            // A Foreground/Background Image Order encodes a binary image where each
            // pixel in the image that is not on the first scanline fulfils exactly one
            // of the following two properties:

            // (a) The pixel matches the uncompressed pixel on the previous scanline
            // XOR'ed with the current foreground color.

            // (b) The pixel matches the uncompressed pixel on the previous scanline.

            // If the pixel is on the first scanline then it fulfils exactly one of the
            // following two properties:

            // (c) The pixel is the current foreground color.

            // (d) The pixel is black.

            // The binary image is encoded as a sequence of byte-sized bitmasks which
            // follow the optional run length (the last bitmask in the sequence can be
            // smaller than one byte in size). If the order is a "set" variant then the
            // bitmasks MUST follow the bytes which specify the new foreground color.
            // Each bit in the encoded bitmask sequence represents one pixel in the
            // image. A bit that has a value of 1 represents a pixel that fulfils
            // either property (a) or (c), while a bit that has a value of 0 represents
            // a pixel that fulfils either property (b) or (d). The individual bitmasks
            // MUST each be processed from the low-order bit to the high-order bit.

            // The run length encodes the number of pixels in the run.

            // If the order is a "set" variant, then in addition to encoding a binary
            // image, the order also encodes a new foreground color (in little-endian
            // format) in the bytes following the optional run length. The current
            // foreground color MUST be updated with the new value before writing
            // the run to the destination buffer.

            // +--------------------------------+--------------------------------------+
            // | 0x2 REGULAR_FGBG_IMAGE         | The compression order encodes a      |
            // |                                | regular-form foreground/background   |
            // |                                | image. The run length is encoded in  |
            // |                                | the five low-order bits of the order |
            // |                                | header byte and MUST be multiplied   |
            // |                                | by 8 to give the final value. If     |
            // |                                | this value is zero, then the run     |
            // |                                | length is encoded in the byte        |
            // |                                | following the order header and MUST  |
            // |                                | be incremented by 1 to give the      |
            // |                                | final value.                         |
            // +--------------------------------+--------------------------------------+
            // | 0xF2 MEGA_MEGA_FGBG_IMAGE      | The compression order encodes a      |
            // |                                | MEGA_MEGA foreground/background      |
            // |                                | image. The run length is stored in   |
            // |                                | the two bytes following the order    |
            // |                                | header (in little-endian format).    |
            // +--------------------------------+--------------------------------------+
            // | 0xD LITE_SET_FG_FGBG_IMAGE     | The compression order encodes a      |
            // |                                | "set" variant lite-form              |
            // |                                | foreground/background image. The run |
            // |                                | length is encoded in the four        |
            // |                                | low-order bits of the order header   |
            // |                                | byte and MUST be multiplied by 8 to  |
            // |                                | give the final value. If this value  |
            // |                                | is zero, then the run length is      |
            // |                                | encoded in the byte following the    |
            // |                                | order header and MUST be incremented |
            // |                                | by 1 to give the final value.        |
            // +--------------------------------+--------------------------------------+
            // | 0xF7 MEGA_MEGA_SET_FGBG_IMAGE  | The compression order encodes a      |
            // |                                | "set" variant MEGA_MEGA              |
            // |                                | foreground/background image. The run |
            // |                                | length is stored in the two bytes    |
            // |                                | following the order header (in       |
            // |                                | little-endian format).               |
            // +-----------------------------------------------------------------------+

            void out_fom_count(const int in_count)
            {
                if (in_count < 256){
                    if (in_count & 7){
                        this->stream.out_uint8(0x40);
                        this->stream.out_uint8(static_cast<uint8_t>(in_count - 1));
                    }
                    else{
                        this->stream.out_uint8(static_cast<uint8_t>(0x40 | (in_count >> 3)));
                    }
                }
                else{
                    this->stream.out_uint8(0xf2);
                    this->stream.out_uint16_le(in_count);
                }
            }

            void out_fom_sequence(const int count, const uint8_t * masks) {
                this->out_fom_count(count);
                this->stream.out_copy_bytes(masks, nbbytes_large(count));
            }

            void out_fom_count_set(const int in_count)
            {
                if (in_count < 256){
                    if (in_count & 0x87){
                        this->stream.out_uint8(0xD0);
                        this->stream.out_uint8(static_cast<uint8_t>(in_count - 1));
                    }
                    else{
                        this->stream.out_uint8(static_cast<uint8_t>(0xD0 | (in_count >> 3)));
                    }
                }
                else{
                    this->stream.out_uint8(0xf7);
                    this->stream.out_uint16_le(in_count);
                }
            }

            void out_fom_sequence_set(const uint8_t Bpp, const int count,
                                      const unsigned foreground, const uint8_t * masks) {
                this->out_fom_count_set(count);
                this->stream.out_bytes_le(Bpp, foreground);
                this->stream.out_copy_bytes(masks, nbbytes_large(count));
            }

            // Color Run Orders
            // ~~~~~~~~~~~~~~~~

            // A Color Run Order encodes a run of pixels where each pixel is the same
            // color. The color is encoded (in little-endian format) in the bytes
            // following the optional run length.

            // The run length encodes the number of pixels in the run.

            // +--------------------------+--------------------------------------------+
            // | 0x3 REGULAR_COLOR_RUN    | The compression order encodes a            |
            // |                          | regular-form color run. The run length is  |
            // |                          | stored in the five low-order bits of the   |
            // |                          | order header byte. If this value is zero,  |
            // |                          | then the run length is encoded in the byte |
            // |                          | following the order header and MUST be     |
            // |                          | incremented by 32 to give the final value. |
            // +--------------------------+--------------------------------------------+
            // | 0xF3 MEGA_MEGA_COLOR_RUN | The compression order encodes a MEGA_MEGA  |
            // |                          | color run. The run length is stored in the |
            // |                          | two bytes following the order header (in   |
            // |                          | little-endian format).                     |
            // +--------------------------+--------------------------------------------+

            void out_color_sequence(const uint8_t Bpp, const int count, const uint32_t color)
            {
                this->out_color_count(count);
                this->stream.out_bytes_le(Bpp, color);
            }

            void out_color_count(const int in_count)
            {
                this->out_count(in_count, 0x03);
            }

            // Color Image Orders
            // ~~~~~~~~~~~~~~~~~~

            // A Color Image Order encodes a run of uncompressed pixels.

            // The run length encodes the number of pixels in the run. So, to compute
            // the actual number of bytes which follow the optional run length, the run
            // length MUST be multiplied by the color depth (in bits-per-pixel) of the
            // bitmap data.

            // +-----------------------------+-----------------------------------------+
            // | 0x4 REGULAR_COLOR_IMAGE     | The compression order encodes a         |
            // |                             | regular-form color image. The run       |
            // |                             | length is stored in the five low-order  |
            // |                             | bits of the order header byte. If this  |
            // |                             | value is zero, then the run length is   |
            // |                             | encoded in the byte following the order |
            // |                             | header and MUST be incremented by 32 to |
            // |                             | give the final value.                   |
            // +-----------------------------+-----------------------------------------+
            // | 0xF4 MEGA_MEGA_COLOR_IMAGE  | The compression order encodes a         |
            // |                             | MEGA_MEGA color image. The run length   |
            // |                             | is stored in the two bytes following    |
            // |                             | the order header (in little-endian      |
            // |                             | format).                                |
            // +-----------------------------+-----------------------------------------+

            void out_copy_sequence(const uint8_t Bpp, const int count, const uint8_t * data)
            {
                this->out_copy_count(count);
                this->stream.out_copy_bytes(data, count * Bpp);
            }

            void out_copy_count(const int in_count)
            {
                this->out_count(in_count, 0x04);
            }

            // Dithered Run Orders
            // ~~~~~~~~~~~~~~~~~~~

            // A Dithered Run Order encodes a run of pixels which is composed of two
            // alternating colors. The two colors are encoded (in little-endian format)
            // in the bytes following the optional run length.

            // The run length encodes the number of pixel-pairs in the run (not pixels).

            // +-----------------------------+-----------------------------------------+
            // | 0xE LITE_DITHERED_RUN       | The compression order encodes a         |
            // |                             | lite-form dithered run. The run length  |
            // |                             | is stored in the four low-order bits of |
            // |                             | the order header byte. If this value is |
            // |                             | zero, then the run length is encoded in |
            // |                             | the byte following the order header and |
            // |                             | MUST be incremented by 16 to give the   |
            // |                             | final value.                            |
            // +-----------------------------+-----------------------------------------+
            // | 0xF8 MEGA_MEGA_DITHERED_RUN | The compression order encodes a         |
            // |                             | MEGA_MEGA dithered run. The run length  |
            // |                             | is stored in the two bytes following    |
            // |                             | the order header (in little-endian      |
            // |                             | format).                                |
            // +-----------------------------+-----------------------------------------+

            void out_bicolor_sequence(const uint8_t Bpp, const int count,
                                      const unsigned color1, const unsigned color2)
            {
                this->out_bicolor_count(count);
                this->stream.out_bytes_le(Bpp, color1);
                this->stream.out_bytes_le(Bpp, color2);
            }

            void out_bicolor_count(const int in_count)
            {
                const uint8_t mask = 0x08;
                if (in_count / 2 < 16){
                    this->stream.out_uint8(static_cast<uint8_t>(0xe0 | (in_count / 2)));
                }
                else if (in_count / 2 < 256 + 16){
                    this->stream.out_uint8(static_cast<uint8_t>(0xe0));
                    this->stream.out_uint8(static_cast<uint8_t>(in_count / 2 - 16));
                }
                else{
                    this->stream.out_uint8(0xf0 | mask);
                    this->stream.out_uint16_le(in_count / 2);
                }
            }


        } out(outbuffer);

        uint8_t * tmp_data_compressed = out.stream.p;

        const uint8_t Bpp = nbbytes(this->original_bpp);
        const uint8_t * pmin = this->data_bitmap.get();
        const uint8_t * p = pmin;

        // white with the right length : either 0xFF or 0xFFFF or 0xFFFFFF
        unsigned foreground = ~(-1 << (Bpp*8));
        unsigned new_foreground = foreground;
        unsigned flags = 0;
        uint8_t masks[512];
        unsigned copy_count = 0;
        const uint8_t * pmax = 0;

        uint32_t color = 0;
        uint32_t color2 = 0;

        for (int part = 0 ; part < 2 ; part++){
            // As far as I can see the specs of bitmap RLE compressor is crap here
            // Fill orders between first scanline and all others must be splitted
            // (or on windows RDP clients black pixels are inserted at beginning of line,
            // on rdesktop this corner case works just fine)...
            // but if the first scanline contains two successive FILL or
            // if all the remaining scanlines contains two consecutive fill
            // orders, a magic MIX pixel is inserted between fills.
            // This explains the surprising loop above and the test below.pp
            if (part){
                pmax = pmin + this->bmp_size;
            }
            else {
                pmax = pmin + align4(this->cx * nbbytes(this->original_bpp));
            }
            while (p < pmax)
            {
                uint32_t fom_count = this->get_fom_count_set(Bpp, pmin, pmax, p, new_foreground, flags);
                uint32_t color_count = 0;
                uint32_t bicolor_count = 0;

                if (p + Bpp < pmax){
                    color = this->get_pixel(Bpp, p);
                    color2 = this->get_pixel(Bpp, p + Bpp);

                    if (color == color2){
                        color_count = this->get_color_count(Bpp, pmax, p, color);
                    }
                    else {
                        bicolor_count = this->get_bicolor_count(Bpp, pmax, p, color, color2);
                    }
                }

                const unsigned fom_cost = 1                            // header
                    + (foreground != new_foreground) * Bpp             // set
                    + (flags == FLAG_FOM) * nbbytes_large(fom_count);  // mask
                const unsigned copy_fom_cost = 1 * (copy_count == 0) + fom_count * Bpp;     // pixels
                const unsigned color_cost = 1 + Bpp;
                const unsigned bicolor_cost = 1 + 2*Bpp;

                if ((fom_count >= color_count || (color_count == 0))
                && ((fom_count >= bicolor_count) || (bicolor_count == 0) || (bicolor_count < 4))
                && fom_cost < copy_fom_cost) {
                    switch (flags){
                        case FLAG_FOM:
                            this->get_fom_masks(Bpp, pmin, p, masks, fom_count);
                            if (new_foreground != foreground){
                                flags = FLAG_FOM_SET;
                            }
                        break;
                        case FLAG_MIX:
                            if (new_foreground != foreground){
                                flags = FLAG_MIX_SET;
                            }
                        break;
                        default:
                        break;
                    }
                }
                else {
                    unsigned copy_color_cost = (copy_count == 0) + color_count * Bpp;       // copy + pixels
                    unsigned copy_bicolor_cost = (copy_count == 0) + bicolor_count * Bpp;   // copy + pixels

                    if ((color_cost < copy_color_cost) && (color_count > 0)){
                        flags = FLAG_COLOR;
                    }
                    else if ((bicolor_cost < copy_bicolor_cost) && (bicolor_count > 0)){
                        flags = FLAG_BICOLOR;
                    }
                    else {
                        flags = FLAG_NONE;
                        copy_count++;
                    }
                }

                if (flags && copy_count > 0){
                    out.out_copy_sequence(Bpp, copy_count, p - copy_count * Bpp);
                    copy_count = 0;
                }

                switch (flags){
                    case FLAG_BICOLOR:
                        out.out_bicolor_sequence(Bpp, bicolor_count, color, color2);
                        p+= bicolor_count * Bpp;
                    break;

                    case FLAG_COLOR:
                        out.out_color_sequence(Bpp, color_count, color);
                        p+= color_count * Bpp;
                    break;

                    case FLAG_FOM_SET:
                        out.out_fom_sequence_set(Bpp, fom_count, new_foreground, masks);
                        foreground = new_foreground;
                        p+= fom_count * Bpp;
                    break;

                    case FLAG_MIX_SET:
                        out.out_mix_count_set(fom_count, Bpp, new_foreground);
                        foreground = new_foreground;
                        p+= fom_count * Bpp;
                    break;

                    case FLAG_FOM:
                        out.out_fom_sequence(fom_count, masks);
                        p+= fom_count * Bpp;
                    break;

                    case FLAG_MIX:
                        out.out_mix_count(fom_count);
                        p+= fom_count * Bpp;
                    break;

                    case FLAG_FILL:
                        out.out_fill_count(fom_count);
                        p+= fom_count * Bpp;
                    break;

                    default: // copy, but wait until next good sequence before actual sending
                        p += Bpp;
                    break;
                }
            }

            if (copy_count > 0){
                out.out_copy_sequence(Bpp, copy_count, p - copy_count * Bpp);
                copy_count = 0;
            }
        }

        // Memoize result of compression
        this->data_compressed_size = out.stream.p - tmp_data_compressed;
        this->data_compressed = static_cast<uint8_t*>(malloc(this->data_compressed_size));
        if (this->data_compressed) {
            memcpy(this->data_compressed, tmp_data_compressed, this->data_compressed_size);
        }
    }

    void compute_sha1(uint8_t (&sig)[20]) const
    {
        SslSha1 sha1;
        uint16_t rowsize = static_cast<uint16_t>(this->cx * nbbytes(this->original_bpp));
        for (size_t y = 0; y < static_cast<size_t>(this->cy); y++){
            sha1.update(FixedSizeStream(this->data_bitmap.get() + y * rowsize, rowsize));
        }
        sha1.final(sig);
    }

    ~Bitmap(){
        if (this->data_compressed) {
            free(this->data_compressed);
            this->data_compressed = NULL;
        }
    }

    Bitmap(uint8_t out_bpp, const Bitmap& bmp)
    : original_bpp(out_bpp)
    , cx(align4(bmp.cx))
    , cy(bmp.cy)
    , line_size(this->cx * nbbytes(this->original_bpp))
    , bmp_size(this->line_size * cy)
    , data_bitmap()
    , data_compressed(NULL)
    , data_compressed_size(0)

    {
//        LOG(LOG_ERR, "Creating bitmap (%p) (copy constructor) cx=%u cy=%u size=%u bpp=%u", this, cx, cy, bmp_size, original_bpp);

        if (out_bpp != bmp.original_bpp){
            this->data_bitmap.alloc(this->bmp_size);
            uint8_t * dest = this->data_bitmap.get();
            const uint8_t * src = bmp.data_bitmap.get();
            const uint8_t src_nbbytes = nbbytes(bmp.original_bpp);
            const uint8_t Bpp = nbbytes(out_bpp);

            for (size_t y = 0; y < bmp.cy ; y++) {
                for (size_t x = 0; x < bmp.cx ; x++) {
                    uint32_t pixel = in_uint32_from_nb_bytes_le(src_nbbytes, src);

                    pixel = color_decode(pixel, bmp.original_bpp, bmp.original_palette);
                    if (out_bpp == 16 || out_bpp == 15 || out_bpp == 8){
                        pixel = RGBtoBGR(pixel);
                    }
                    pixel = color_encode(pixel, out_bpp);

                    out_bytes_le(dest, Bpp, pixel);
                    src += src_nbbytes;
                    dest += Bpp;
                }
                TODO("padding code should not be necessary as source bmp width is already aligned");
                if (this->line_size < bmp.cx * Bpp){
                    uint16_t padding = this->line_size - bmp.cx * Bpp;
                    bzero(dest, padding);
                    dest += padding;
                }
                TODO("padding code should not be necessary for source either as source bmp width is already aligned");
                src += bmp.line_size - bmp.cx * nbbytes(bmp.original_bpp);
            }
        }
        else {
            this->data_bitmap.use(bmp.data_bitmap);
        }

        if (out_bpp == 8){
            if (bmp.original_palette){
                memcpy(&this->original_palette, bmp.original_palette, sizeof(BGRPalette));
            }
            else {
                init_palette332(this->original_palette);
            }
        }
    }

    Bitmap(uint8_t bpp, const BGRPalette * palette, uint16_t cx, uint16_t cy)
        : original_bpp(bpp)
        , cx(align4(cx))
        , cy(cy)
        , line_size(this->cx * nbbytes(this->original_bpp))
        , bmp_size(this->line_size * cy)
        , data_bitmap()
        , data_compressed(NULL)
        , data_compressed_size(0)
    {
        this->data_bitmap.alloc(this->bmp_size);
//        LOG(LOG_ERR, "Creating bitmap (%p) cx=%u cy=%u size=%u bpp=%u", this, cx, cy, size, bpp);
        if (bpp == 8){
            if (palette){
                memcpy(&this->original_palette, palette, sizeof(BGRPalette));
            }
            else {
                init_palette332(this->original_palette);
            }
        }

        if (this->cx <= 0 || this->cy <= 0){
            LOG(LOG_ERR, "Bogus empty bitmap!!! cx=%u cy=%u bpp=%u", this->cx, this->cy, this->original_bpp);
        }
    }
};

#endif
