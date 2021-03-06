/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CORE_RDP_ORDERS_RDPORDERSPRIMARYMULTIDSTBLT_HPP_
#define _REDEMPTION_CORE_RDP_ORDERS_RDPORDERSPRIMARYMULTIDSTBLT_HPP_

// 2.2.2.2.1.1.1.1 Coord Field (COORD_FIELD)
// =========================================
// The COORD_FIELD structure is used to describe a value in the range -32768
//  to 32767.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     signedValue (variable)    |
// +-------------------------------+

// signedValue (variable): A signed, 1-byte or 2-byte value that describes a
//  coordinate in the range -32768 to 32767.

//  When the controlFlags field (see section 2.2.2.2.1.1.2) of the primary
//   drawing order that contains the COORD_FIELD structure has the
//   TS_DELTA_COORDINATES flag (0x10) set, the signedValue field MUST contain
//   a signed 1-byte value. If the TS_DELTA_COORDINATES flag is not set, the
//   signedValue field MUST contain a 2-byte signed value.

//  The 1-byte format contains a signed delta from the previous value of the
//   Coord field. To obtain the new value of the field, the decoder MUST
//   increment the previous value of the field by the signed delta to produce
//   the current value. The 2-byte format is simply the full value of the
//   field that MUST replace the previous value.

// 2.2.2.2.1.1.1.3 Two-Byte Header Variable Field (VARIABLE2_FIELD)
// ================================================================
// The VARIABLE2_FIELD structure is used to encode a variable-length byte-
//  stream that holds a maximum of 32,767 bytes. This structure is always
//  situated at the end of an order.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |             cbData            |       rgbData (variable)      |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// cbData (2 bytes): A 16-bit, unsigned integer. The number of bytes present
//  in the rgbData field.

// rgbData (variable): Variable-length, binary data. The size of this data,
//  in bytes, is given by the cbData field.

// 2.2.2.2.1.1.1.5 Delta-Encoded Rectangles (DELTA_RECTS_FIELD)
// ============================================================
// The DELTA_RECTS_FIELD structure is used to encode a series of rectangles.
//  Each rectangle is encoded as a (left, top, width, height) quadruple with
//  the left and top components of each quadruple containing the delta from
//  the left and top components of the previous rectangle; the first
//  rectangle in the series is implicitly assumed to be (0, 0, 0, 0). The
//  number of rectangles is order-dependent and not specified by any field
//  within the DELTA_RECTS_FIELD structure. Instead, a separate field within
//  the order that contains the DELTA_RECTS_FIELD structure MUST be used to
//  specify the number of rectangles (this field SHOULD be placed immediately
//  before the DELTA_RECTS_FIELD structure in the order encoding). The
//  maximum number of rectangles that can be encoded by this structure is 45.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                      zeroBits (variable)                      |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |               deltaEncodedRectangles (variable)               |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// zeroBits (variable): A variable-length byte field. The zeroBits field is
//  used to indicate the absence of a left, top, width, or height component.
//  The size, in bytes, of the zeroBits field is given by ceil(NumRects / 2)
//  where NumRects is the number of rectangles being encoded. Each rectangle
//  in the series requires four zero-bits (two rectangles per byte) to
//  indicate whether a left, top, width, or height component is zero (and not
//  present), starting with the most significant bits, so that for the first
//  rectangle the left-zero flag is set at (zeroBits[0] & 0x80), the top-zero
//  flag is set at (zeroBits[0] & 0x40), the width-zero flag is set at
//  (zeroBits[0] & 0x20), and the height-zero flag is set at (zeroBits[0] &
//  0x10).

// deltaEncodedRectangles (variable): A variable-length byte field. The
//  deltaEncodedRectangles field contains a series of (left, top, width,
//  height) quadruples with the left and top components in each quadruple
//  specifying the delta from the left and top components of the previous
//  rectangle in the series; the first rectangle in the series is implicitly
//  assumed to be (0, 0, 0, 0).

//  Assume there are two rectangles specified in (left, top, right, bottom)
//   quadruples:

//   1: (L1, T1, R1, B1)
//   2: (L2, T2, R2, B2)

//  Assuming Rectangle 1 is the first in the series, and by using the (left,
//   top, width, height) quadruple encoding scheme, these two rectangles
//   would be specified as:

//   1: (L1, T1, R1 - L1, B1 - T1)
//   2: (L2 - L1, T2 - T1, R2 - L2, B2 - T2)

//  The presence of the left, top, width, or height component for a given
//   quadruple is dictated by the individual bits of the zeroBits field. If
//   the zero bit is set for a given left, top, width, or height component,
//   its value is unchanged from the previous corresponding left, top, width,
//   or height value in the series (a delta of zero), and no data is
//   provided. If the zero bit is not set for a left, right, width, or height
//   component, its value is encoded in a packed signed format:

//   * If the high bit (0x80) is not set in the first encoding byte, the
//     field is 1 byte long and is encoded as a signed delta in the lower 7
//     bits of the byte.

//   * If the high bit of the first encoding byte is set, the lower 7 bits of
//     the first byte and the 8 bits of the next byte are concatenated (the
//     first byte containing the high order bits) to create a 15-bit signed
//     delta value.

// 2.2.2.2.1.1.2.2 MultiDstBlt (MULTI_DSTBLT_ORDER)
// ================================================
// The MultiDstBlt Primary Drawing Order is used to paint multiple rectangles
//  by using a destination-only raster operation.

//  Encoding order number: 15 (0x0F)
//  Negotiation order number: 15 (0x0F)
//  Number of fields: 7
//  Number of field encoding bytes: 1
//  Maximum encoded field length: 395 bytes

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |      nLeftRect (variable)     |      nTopRect (variable)      |
// +-------------------------------+-------------------------------+
// |       nWidth (variable)       |       nHeight (variable)      |
// +---------------+---------------+-------------------------------+
// |bRop (optional)| nDeltaEntries |   CodedDeltaList (variable)   |
// |               |   (optional)  |                               |
// +---------------+---------------+-------------------------------+

// nLeftRect (variable): A signed, 1-byte or 2-byte field. Left coordinate of
//  the destination rectangle specified using a Coord Field (section
//  2.2.2.2.1.1.1.1).

// nTopRect (variable): A signed, 1-byte or 2-byte field. Top coordinate of
//  the destination rectangle specified using a Coord Field (section
//  2.2.2.2.1.1.1.1).

// nWidth (variable): A signed, 1-byte or 2-byte field. Width of the
//  destination rectangle specified using a Coord Field (section
//  2.2.2.2.1.1.1.1).

// nHeight (variable): A signed, 1-byte or 2-byte field. Height of the
//  destination rectangle specified using a Coord Field (section
//  2.2.2.2.1.1.1.1).

// bRop (1 byte): Index of the ternary raster operation to perform (see
//  section 2.2.2.2.1.1.1.7). The resultant ROP3 operation MUST only depend
//  on the destination bits (there MUST NOT be any dependence on source or
//  pattern bits).

// nDeltaEntries (1 byte): An 8-bit, unsigned integer. The number of bounding
//  rectangles described by the CodedDeltaList field.

// CodedDeltaList (variable): A Two-Byte Header Variable Field (section
//  2.2.2.2.1.1.1.3) structure that encapsulates a Delta-Encoded Rectangles
//  (section 2.2.2.2.1.1.1.5) structure that contains bounding rectangles to
//  use when rendering the order. The number of rectangles described by the
//  Delta-Encoded Rectangles structure is specified by the nDeltaEntries
//  field.

class RDPMultiDstBlt {
public:
    int16_t nLeftRect;
    int16_t nTopRect;
    int16_t nWidth;
    int16_t nHeight;
    uint8_t bRop;
    uint8_t nDeltaEntries;

    struct DeltaEncodedRectangle {
        int16_t leftDelta;
        int16_t topDelta;
        int16_t width;
        int16_t height;
    } deltaEncodedRectangles[45];

    static const uint8_t id(void) {
        return RDP::MULTIDSTBLT;
    }

    RDPMultiDstBlt()
    : nLeftRect(0)
    , nTopRect(0)
    , nWidth(0)
    , nHeight(0)
    , bRop(0)
    , nDeltaEntries(0) {
        ::memset(this->deltaEncodedRectangles, 0, sizeof(this->deltaEncodedRectangles));
    }

    RDPMultiDstBlt(int16_t nLeftRect, int16_t nTopRect, int16_t nWidth, int16_t nHeight, uint8_t bRop, uint8_t nDeltaEntries,
        Stream & deltaEncodedRectangles)
    : nLeftRect(nLeftRect)
    , nTopRect(nTopRect)
    , nWidth(nWidth)
    , nHeight(nHeight)
    , bRop(bRop)
    , nDeltaEntries(nDeltaEntries) {
        ::memset(this->deltaEncodedRectangles, 0, sizeof(this->deltaEncodedRectangles));
        for (int i = 0; i < this->nDeltaEntries; i++) {
            this->deltaEncodedRectangles[i].leftDelta = deltaEncodedRectangles.in_sint16_le();
            this->deltaEncodedRectangles[i].topDelta  = deltaEncodedRectangles.in_sint16_le();
            this->deltaEncodedRectangles[i].width     = deltaEncodedRectangles.in_sint16_le();
            this->deltaEncodedRectangles[i].height    = deltaEncodedRectangles.in_sint16_le();
        }
    }

    bool operator==(const RDPMultiDstBlt & other) const {
        return  (this->nLeftRect     == other.nLeftRect)
             && (this->nTopRect      == other.nTopRect)
             && (this->nWidth        == other.nWidth)
             && (this->nHeight       == other.nHeight)
             && (this->bRop          == other.bRop)
             && (this->nDeltaEntries == other.nDeltaEntries)
             && !memcmp(this->deltaEncodedRectangles, other.deltaEncodedRectangles, sizeof(DeltaEncodedRectangle) * this->nDeltaEntries)
             ;
    }

    void emit(Stream & stream, RDPOrderCommon & common, const RDPOrderCommon & oldcommon, const RDPMultiDstBlt & oldcmd) const {
        RDPPrimaryOrderHeader header(RDP::STANDARD, 0);

        int16_t nLeftRect = 0;
        int16_t nTopRect  = 0;
        int16_t nWidth    = 0;
        int16_t nHeight   = 0;
        if (!common.clip.contains(Rect(nLeftRect, nTopRect, nWidth, nHeight))) {
            header.control |= RDP::BOUNDS;
        }
        else {
            for (uint8_t i = 0; i < this->nDeltaEntries; i++) {
                nLeftRect += this->deltaEncodedRectangles[i].leftDelta;
                nTopRect  += this->deltaEncodedRectangles[i].topDelta;
                nWidth    =  this->deltaEncodedRectangles[i].width;
                nHeight   =  this->deltaEncodedRectangles[i].height;

                if (!common.clip.contains(Rect(nLeftRect, nTopRect, nWidth, nHeight))) {
                    header.control |= RDP::BOUNDS;
                    break;
                }
            }
        }

        header.control |= (is_1_byte(this->nLeftRect - oldcmd.nLeftRect) && is_1_byte(this->nTopRect - oldcmd.nTopRect) &&
            is_1_byte(this->nWidth - oldcmd.nWidth) && is_1_byte(this->nHeight - oldcmd.nHeight)) * RDP::DELTA;

        header.fields =
                (this->nLeftRect     != oldcmd.nLeftRect    ) * 0x0001
              | (this->nTopRect      != oldcmd.nTopRect     ) * 0x0002
              | (this->nWidth        != oldcmd.nWidth       ) * 0x0004
              | (this->nHeight       != oldcmd.nHeight      ) * 0x0008
              | (this->bRop          != oldcmd.bRop         ) * 0x0010
              | (this->nDeltaEntries != oldcmd.nDeltaEntries) * 0x0020
              | (
                 (this->nDeltaEntries != oldcmd.nDeltaEntries) ||
                 memcmp(this->deltaEncodedRectangles, oldcmd.deltaEncodedRectangles,
                        this->nDeltaEntries * sizeof(DeltaEncodedRectangle))
                                                            ) * 0x0040
              ;

        common.emit(stream, header, oldcommon);

        header.emit_coord(stream, 0x0001, this->nLeftRect, oldcmd.nLeftRect);
        header.emit_coord(stream, 0x0002, this->nTopRect,  oldcmd.nTopRect);
        header.emit_coord(stream, 0x0004, this->nWidth,    oldcmd.nWidth);
        header.emit_coord(stream, 0x0008, this->nHeight,   oldcmd.nHeight);

        if (header.fields & 0x0010) { stream.out_uint8(this->bRop); }

        if (header.fields & 0x0020) { stream.out_uint8(this->nDeltaEntries); }

        if (header.fields & 0x0040) {
            uint32_t offset_cbData = stream.get_offset();
            stream.out_clear_bytes(2);

            uint8_t * zeroBit = stream.out_uint8p((this->nDeltaEntries + 1) / 2);
            *zeroBit = 0;

            for (uint8_t i = 0, m2 = 0; i < this->nDeltaEntries; i++, m2++) {
                if (m2 == 2) {
                    m2 = 0;
                }

                if (i && !m2) {
                    *(++zeroBit) = 0;
                }

                if (!this->deltaEncodedRectangles[i].leftDelta) {
                    *zeroBit |= (1 << (7 - m2 * 4));
                }
                else {
                    stream.out_DEP(this->deltaEncodedRectangles[i].leftDelta);
                }

                if (!this->deltaEncodedRectangles[i].topDelta) {
                    *zeroBit |= (1 << (6 - m2 * 4));
                }
                else {
                    stream.out_DEP(this->deltaEncodedRectangles[i].topDelta);
                }

                if (!this->deltaEncodedRectangles[i].width) {
                    *zeroBit |= (1 << (5 - m2 * 4));
                }
                else {
                    stream.out_DEP(this->deltaEncodedRectangles[i].width);
                }

                if (!this->deltaEncodedRectangles[i].height) {
                    *zeroBit |= (1 << (4 - m2 * 4));
                }
                else {
                    stream.out_DEP(this->deltaEncodedRectangles[i].height);
                }
            }

            stream.set_out_uint16_le(stream.get_offset() - offset_cbData - 2, offset_cbData);
        }
    }   // void emit(Stream & stream, RDPOrderCommon & common, const RDPOrderCommon & oldcommon, const RDPMultiDstBlt & oldcmd) const

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header) {
        // LOG(LOG_INFO, "RDPMultiDstBlt::receive: header fields=0x%02X", header.fields);

        header.receive_coord(stream, 0x0001, this->nLeftRect);
        header.receive_coord(stream, 0x0002, this->nTopRect);
        header.receive_coord(stream, 0x0004, this->nWidth);
        header.receive_coord(stream, 0x0008, this->nHeight);

        if (header.fields & 0x0010) {
            this->bRop = stream.in_uint8();
        }

        if (header.fields & 0x0020) {
            this->nDeltaEntries = stream.in_uint8();
        }

        if (header.fields & 0x0040) {
            uint16_t cbData = stream.in_uint16_le();
            // LOG(LOG_INFO, "cbData=%d", cbData);

            SubStream rgbData(stream, stream.get_offset(), cbData);
            stream.in_skip_bytes(cbData);
            // hexdump_d(rgbData.p, rgbData.size());

            uint8_t zeroBitsSize = ((this->nDeltaEntries + 1) / 2);
            // LOG(LOG_INFO, "zeroBitsSize=%d", zeroBitsSize);

            SubStream zeroBits(rgbData, rgbData.get_offset(), zeroBitsSize);
            rgbData.in_skip_bytes(zeroBitsSize);

            uint8_t zeroBit = 0;

            for (uint8_t i = 0, m2 = 0; i < this->nDeltaEntries; i++, m2++) {
                if (m2 == 2) {
                    m2 = 0;
                }

                if (!m2) {
                    zeroBit = zeroBits.in_uint8();
                    // LOG(LOG_INFO, "0x%02X", zeroBit);
                }

                this->deltaEncodedRectangles[i].leftDelta = (!(zeroBit & 0x80) ? rgbData.in_DEP() : 0);
                this->deltaEncodedRectangles[i].topDelta  = (!(zeroBit & 0x40) ? rgbData.in_DEP() : 0);
                this->deltaEncodedRectangles[i].width     = (!(zeroBit & 0x20) ? rgbData.in_DEP() : 0);
                this->deltaEncodedRectangles[i].height    = (!(zeroBit & 0x10) ? rgbData.in_DEP() : 0);

/*
                LOG(LOG_INFO, "RDPMultiDstBlt::receive: delta rectangle=(%d, %d, %d, %d)",
                    this->deltaEncodedRectangles[i].leftDelta, this->deltaEncodedRectangles[i].topDelta,
                    this->deltaEncodedRectangles[i].width, this->deltaEncodedRectangles[i].height);
*/

                zeroBit <<= 4;
            }
        }
    }   // void receive(Stream & stream, const RDPPrimaryOrderHeader & header)

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const {
        size_t lg = 0;
        lg += common.str(buffer + lg, sz - lg, true);
        lg += snprintf(buffer + lg, sz - lg,
            "MultiDstBlt(nLeftRect=%d nTopRect=%d nWidth=%d nHeight=%d bRop=0x%02X nDeltaEntries=%d "
                "CodedDeltaList=(",
            this->nLeftRect, this->nTopRect, this->nWidth, this->nHeight, this->bRop, this->nDeltaEntries);
        for (uint8_t i = 0; i < this->nDeltaEntries; i++) {
            if (i) {
                lg += snprintf(buffer + lg, sz - lg, " ");
            }
            lg += snprintf(buffer + lg, sz - lg, "(%d, %d, %d, %d)", this->deltaEncodedRectangles[i].leftDelta,
                this->deltaEncodedRectangles[i].topDelta, this->deltaEncodedRectangles[i].width,
                this->deltaEncodedRectangles[i].height);
        }
        lg += snprintf(buffer + lg, sz - lg, "))");
        if (lg >= sz) {
            return sz;
        }
        return lg;
    }

    void log(int level, const Rect & clip) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer), RDPOrderCommon(this->id(), clip));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }

    void print(const Rect & clip) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer), RDPOrderCommon(this->id(), clip));
        buffer[sizeof(buffer) - 1] = 0;
        printf("%s", buffer);
    }
};  // class RDPMultiDstBlt

#endif  // #ifndef _REDEMPTION_CORE_RDP_ORDERS_RDPORDERSPRIMARYMULTIDSTBLT_HPP_
