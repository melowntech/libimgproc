/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef imgproc_contour_hpp_included_
#define imgproc_contour_hpp_included_

#include <bitset>
#include <map>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include "dbglog/dbglog.hpp"

#include "utility/enum-io.hpp"

#include "./contours.hpp"

namespace bmi = boost::multi_index;

namespace imgproc {

namespace {

typedef detail::FindContourImpl::CellType CellType;

enum : CellType {
    b0000 = 0x0, b0001 = 0x1, b0010 = 0x2, b0011 = 0x3
    , b0100 = 0x4, b0101 = 0x5, b0110 = 0x6, b0111 = 0x7
    , b1000 = 0x8, b1001 = 0x9, b1010 = 0xa, b1011 = 0xb
    , b1100 = 0xc, b1101 = 0xd, b1110 = 0xe, b1111 = 0xf
};


UTILITY_GENERATE_ENUM(Direction,
                      ((r))
                      ((l))
                      ((u))
                      ((d))
                      ((lu))
                      ((ld))
                      ((ru))
                      ((rd))
                      )

struct PointPosition { enum : CellType {
    ll = 0x0
    , lr = 0x2
    , ur = 0x4
    , ul = 0x8
}; };

typedef math::Point2i Vertex;
typedef math::Points2i Vertices;

struct Segment {
    CellType type;
    Direction direction;
    Vertex start;
    Vertex end;

    mutable const Segment *prev;
    mutable const Segment *next;
    mutable const Segment *ringLeader;

    Segment(CellType type, Direction direction
            , Vertex start, Vertex end
            , const Segment *prev, const Segment *next)
        : type(type), direction(direction)
        , start(start), end(end)
        , prev(prev), next(next), ringLeader()
    {}
};

struct StartIdx {};
struct EndIdx {};

typedef boost::multi_index_container<
    Segment
    , bmi::indexed_by<
          bmi::ordered_unique<
              bmi::tag<StartIdx>
              , BOOST_MULTI_INDEX_MEMBER
              (Segment, decltype(Segment::start), start)
              >
          , bmi::ordered_unique<
                bmi::tag<EndIdx>
                , BOOST_MULTI_INDEX_MEMBER
                (Segment, decltype(Segment::end), end)
                >
          >
    > SegmentMap;

inline void distributeRingLeaderPrev(const Segment *s)
{
    // grab current ringLeader, move skip current and write ringLeader to all
    // previous segments
    const auto &ringLeader(s->ringLeader);
    for (s = s->prev; s; s = s->prev) { s->ringLeader = ringLeader; }
}

inline void distributeRingLeaderNext(const Segment *s)
{
    // grab current ringLeader, move skip current and write ringLeader to all
    // next segments
    const auto &ringLeader(s->ringLeader);
    for (s = s->next; s; s = s->next) { s->ringLeader = ringLeader; }
}

} // namespace

struct FindContour::Builder {
    Builder(FindContour &cf, const math::Size2 &rasterSize)
        : cf(cf), contour(rasterSize)
    {}

    template <typename Index>
    const Segment* find(Index &idx, const Vertex &v) {
        auto fidx(idx.find(v));
        return ((fidx == idx.end()) ? nullptr : &*fidx);
    }

    const Segment* findByStart(const Vertex &v) {
        return find(segments.get<StartIdx>(), v);
    }

    const Segment* findByEnd(const Vertex &v) {
        return find(segments.get<EndIdx>(), v);
    }

    void addSegment(CellType type, Direction direction, int i, int j
                    , const Vertex &start, const Vertex &end);

    void add(int i, int j, CellType type);

    void addBorder(int i, int j, CellType type);

    void addAmbiguous(CellType type, int i, int j);

    void extract(const Segment *head);

    void setBorder(CellType type, int i, int j);

    FindContour &cf;
    SegmentMap segments;
    Contour contour;
};


void FindContour::Builder::setBorder(CellType type, int i, int j)
{
#define SET_BORDER(X, Y) contour.border.set(i + X, j + Y)

    switch (type) {
    case b0000: return;

    case b0001: return SET_BORDER(0, 1);
    case b0010: return SET_BORDER(1, 1);
    case b0100: return SET_BORDER(1, 0);
    case b1000: return SET_BORDER(0, 0);

    case b0011:
        SET_BORDER(0, 1);
        return SET_BORDER(1, 1);
    case b0110:
        SET_BORDER(1, 0);
        return SET_BORDER(1, 1);
    case b1100:
        SET_BORDER(0, 0);
        return SET_BORDER(1, 0);
    case b1001:
        SET_BORDER(0, 0);
        return SET_BORDER(0, 1);

    case b0101:
    case b0111:
    case b1010:
    case b1011:
    case b1101:
    case b1110:
        SET_BORDER(0, 0);
        SET_BORDER(1, 0);
        SET_BORDER(0, 1);
        return SET_BORDER(1, 1);

    case b1111: return;
    }

#undef SET_BORDER
}

void FindContour::Builder::addSegment(CellType type
                                      , Direction direction
                                      , int i, int j
                                      , const Vertex &start, const Vertex &end)
{
    setBorder(type, i, j);

    // mark in raster
    auto *prev(findByEnd(start));
    auto *next(findByStart(end));

    const auto &s
        (*segments.insert(Segment(type, direction, start, end
                                  , prev, next)).first);

    // LOG(info4) << "Segment " << s.start << " -> " << s.end << "> "
    //            << &s;

    // stranded segment -> we're done here
    if (!prev && !next) { return; }

    // insert new segment

    // link prev
    const Segment *pRingLeader(nullptr);
    if (prev) {
        prev->next = &s;
        pRingLeader = prev->ringLeader;
    }

    // link next
    const Segment *nRingLeader(nullptr);
    if (next) {
        next->prev = &s;
        nRingLeader = next->ringLeader;
    }

    // unify/create ringLeader
    if (!pRingLeader && !nRingLeader) {
        // no ringLeader, create for this segment
        s.ringLeader = &s;
        // and copy to others (there is at most one segment in each
        // direction -> simple assignment)
        if (next) { next->ringLeader = s.ringLeader; }
        if (prev) { prev->ringLeader = s.ringLeader; }
    } else if (!pRingLeader) {
        // distribute ringLeader from next node to all its previous segments
        distributeRingLeaderPrev(next);
    } else if (!nRingLeader) {
        // distribute ringLeader from prev node to all its next segments
        distributeRingLeaderNext(prev);
    } else if (pRingLeader != nRingLeader) {
        // both valid but different, prefer prev segment
        distributeRingLeaderNext(prev);
    } else {
        // both are the same -> we've just closed the ringLeader
        s.ringLeader = pRingLeader;

        // new ringLeader, extract contour
        extract(pRingLeader);
    }
}

#define ADD_SEGMENT(D, X1, Y1, X2, Y2)                                  \
    addSegment(type, Direction::D, i, j                                 \
               , { x + X1, y + Y1 }, { x + X2, y + Y2 })

void FindContour::Builder::addAmbiguous(CellType otype, int i, int j)
{
    // 2x scale to get rid of non-integral values
    auto x(i * 2);
    auto y(j * 2);

    Vertex v(x, y);

    auto type(cf.ambiguousType(v, otype));

    if (type == otype) {
        // same type
        switch (type) {
        case b0101: // b0111 + b1101
            ADD_SEGMENT(ru, 0, 1, 1, 0);
            return ADD_SEGMENT(ld, 2, 1, 1, 2);

        case b1010: // b1011 + b1110
            ADD_SEGMENT(rd, 1, 0, 2, 1);
            return ADD_SEGMENT(ru, 1, 2, 0, 1);
        }
    } else {
        // inverse type -> switch direction
        switch (type) {
        case b0101: // b1000 + b0010
            ADD_SEGMENT(ld, 1, 0, 0, 1);
            return ADD_SEGMENT(ru, 1, 2, 2, 1);

        case b1010: // b0100 + b0001
            ADD_SEGMENT(lu, 2, 1, 1, 0);
            return ADD_SEGMENT(rd, 0, 1, 1, 2);
        }
    }
}

void FindContour::Builder::add(int i, int j, CellType type)
{
    // 2x scale to get rid of non-integral values
    auto x(i * 2);
    auto y(j * 2);

    // LOG(info4) << "Adding inner: " << x << ", " << y;

    switch (type) {
    case b0000: return;
    case b0001: return ADD_SEGMENT(rd, 0, 1, 1, 2);
    case b0010: return ADD_SEGMENT(ru, 1, 2, 2, 1);
    case b0011: return ADD_SEGMENT(r, 0, 1, 2, 1);
    case b0100: return ADD_SEGMENT(lu, 2, 1, 1, 0);
    case b0101: return addAmbiguous(type, i, j);
    case b0110: return ADD_SEGMENT(u, 1, 2, 1, 0);
    case b0111: return ADD_SEGMENT(lu, 0, 1, 1, 0);
    case b1000: return ADD_SEGMENT(ld, 1, 0, 0, 1);
    case b1001: return ADD_SEGMENT(d, 1, 0, 1, 2);
    case b1010: return addAmbiguous(type, i, j);
    case b1011: return ADD_SEGMENT(rd, 1, 0, 2, 1);
    case b1100: return ADD_SEGMENT(l, 2, 1, 0, 1);
    case b1101: return ADD_SEGMENT(ld, 2, 1, 1, 2);
    case b1110: return ADD_SEGMENT(ru, 1, 2, 0, 1);
    case b1111: return;
    }
}

void FindContour::Builder::addBorder(int i, int j, CellType type)
{
    // 2x scale to get rid of non-integral values
    auto x(i * 2);
    auto y(j * 2);

    // LOG(info4) << "Adding border: " << x << ", " << y;

    switch (type) {
    case b0000: return;

    case b0001:
        ADD_SEGMENT(r, 0, 1, 1, 1);
        return ADD_SEGMENT(d, 1, 1, 1, 2);

    case b0010:
        ADD_SEGMENT(u, 1, 2, 1, 1);
        ADD_SEGMENT(r, 1, 1, 2, 1);

    case b0011: return ADD_SEGMENT(r, 0, 1, 2, 1);

    case b0100:
        ADD_SEGMENT(l, 2, 1, 1, 1);
        return ADD_SEGMENT(u, 1, 1, 1, 0);

    case b0101: // b0111 + b1101
        ADD_SEGMENT(u, 0, 1, 0, 0);
        ADD_SEGMENT(r, 0, 0, 1, 0);
        ADD_SEGMENT(d, 2, 1, 2, 2);
        return ADD_SEGMENT(l, 2, 2, 1, 2);

    case b0110: return ADD_SEGMENT(u, 1, 2, 1, 0);

    case b0111:
        ADD_SEGMENT(u, 0, 1, 0, 0);
        return ADD_SEGMENT(r, 0, 0, 1, 0);

    case b1000:
        ADD_SEGMENT(d, 1, 0, 1, 1);
        return ADD_SEGMENT(l, 1, 1, 0, 1);

    case b1001: return ADD_SEGMENT(d, 1, 0, 1, 2);

    case b1010: // b1011 + b1110
        ADD_SEGMENT(r, 1, 0, 2, 0);
        ADD_SEGMENT(d, 2, 0, 2, 1);
        ADD_SEGMENT(l, 1, 2, 0, 2);
        return ADD_SEGMENT(u, 0, 2, 0, 1);

    case b1011:
        ADD_SEGMENT(r, 1, 0, 2, 0);
        return ADD_SEGMENT(d, 2, 0, 2, 1);

    case b1100: return ADD_SEGMENT(l, 2, 1, 0, 1);

    case b1101:
        ADD_SEGMENT(d, 2, 1, 2, 2);
        return ADD_SEGMENT(l, 2, 2, 1, 2);

    case b1110:
        ADD_SEGMENT(l, 1, 2, 0, 2);
        return ADD_SEGMENT(u, 0, 2, 0, 1);

    case b1111: return;
    }
}

#undef ADD_SEGMENT

void FindContour::Builder::extract(const Segment *head)
{
    // LOG(info4) << "Processing ring from: " << head;
    contour.rings.emplace_back();
    auto &ring(contour.rings.back());

    const auto addVertex([&](const Vertex &v) {
            ring.emplace_back(v(0) / 2.0, v(1) / 2.0);
        });

    // add first vertex
    addVertex(head->start);

    // end segment
    const auto end((head->type != head->prev->type) ? head : head->prev);

    // process full ringLeader
    for (const auto *p(head), *s(head->next); s != end; p = s, s = s->next)
    {
        if (s->ringLeader != head) {
            LOGTHROW(info4, std::runtime_error)
                << "Segment: " << s << " doesn't belong to ring "
                << head << " but " << s->ringLeader << ".";
        }

        if (!s->next) {
            LOGTHROW(info4, std::runtime_error)
                << "Segment: type: [" << std::bitset<4>(s->type) << "/"
                << s->direction << "] "
                << "<" << s->start << " -> " << s->end << "> "
                << s << " in ringLeader "
                << s->ringLeader << " has no next segment.";
            }

        // add vertex only when direction differs
        if (s->direction != p->direction) {
            addVertex(s->start);
        }
    }
}

Contour FindContour::operator()(const Contour::Raster &raster)
{
    const auto size(raster.dims());

    Builder cb(*this, size);

    const auto getFlag([&](int x, int y, CellType flag) -> CellType
    {
        return (raster.get(x, y) ? flag : 0);
    });

    const auto getFlags([&](int x, int y) -> CellType
    {
        return (getFlag(x, y + 1, 1)
                | getFlag(x + 1, y + 1, 2)
                | getFlag(x + 1,  y, 4)
                | getFlag(x, y, 8));
    });

#define ADD(x, y) cb.add(x, y, getFlags(x, y))
#define ADD_BORDER(x, y) cb.addBorder(x, y, getFlags(x, y))

    int xend(size.width - 1);
    int yend(size.height - 1);

    // first row
    for (int i(-1); i <= xend; ++i) { ADD_BORDER(i, -1); }

    // all inner rows
    for (int j(0); j < yend; ++j) {
        // first column
        ADD_BORDER(-1, j);

        // all inner columns
        for (int i(0); i < xend; ++i) { ADD(i, j); }

        // last column
        ADD_BORDER(xend, j);
    }

    // last row
    for (int i(-1); i <= xend; ++i) { ADD_BORDER(i, yend); }

#undef ADD
#undef ADD_BORDER

    return cb.contour;
}

} // namespace imgproc

#endif // imgproc_contour_hpp_included_
