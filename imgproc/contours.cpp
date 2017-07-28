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

#include "./contours.hpp"

namespace bmi = boost::multi_index;

namespace imgproc {

namespace {

enum {
    b0000 = 0x0, b0001 = 0x1, b0010 = 0x2, b0011 = 0x3
    , b0100 = 0x4, b0101 = 0x5, b0110 = 0x6, b0111 = 0x7
    , b1000 = 0x8, b1001 = 0x9, b1010 = 0xa, b1011 = 0xb
    , b1100 = 0xc, b1101 = 0xd, b1110 = 0xe, b1111 = 0xf
};

typedef math::Point2i Vertex;
typedef math::Points2i Vertices;

struct Segment;

struct Ring {
    typedef std::shared_ptr<Ring> pointer;

    Ring(const Segment *head) : head(head) {}

    const Segment *head;

    typedef std::set<const Ring*> set;
};

struct Segment {
    std::uint8_t fullType;
    std::uint8_t type;
    Vertex start;
    Vertex end;

    struct Links {
        const Segment *prev;
        const Segment *next;
        Ring::pointer ring;

        Links(const Segment *prev, const Segment *next)
            : prev(prev), next(next)
        {}
    };

    mutable Links links;

    Segment(std::uint8_t fullType, std::uint8_t type
            , Vertex start, Vertex end
            , const Segment *prev, const Segment *next)
        : fullType(fullType), type(type), start(start), end(end)
        , links(prev, next)
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

struct ContourBuilder {
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

    Ring::pointer newRing(const Segment *head) {
        Ring::pointer ring(new Ring(head)
                           , [&](Ring *ring) {
                               rings.erase(ring);
                               delete ring;
                           });

        rings.insert(ring.get());
        return ring;
    }

    void addSegment(std::uint8_t fullType, std::uint8_t type
                    , const Vertex &start, const Vertex &end)
    {
        auto *prev(findByEnd(start));
        auto *next(findByStart(end));

        const auto &s
            (*segments.insert(Segment(fullType, type, start, end
                                      , prev, next)).first);

        // LOG(info4) << "Segment " << s.start << " -> " << s.end << "> "
        //            << &s;

        // stranded segment -> we're done here
        if (!prev && !next) { return; }

        // insert new segment

        // link
        Ring::pointer pRing;
        Ring::pointer nRing;
        if (prev) {
            prev->links.next = &s;
            pRing = prev->links.ring;
        }
        if (next) {
            next->links.prev = &s;
            nRing = next->links.ring;
        }

        const auto distributeRingPrev([&](const Segment *s) -> void
        {
            // grab current ring, move skip current and write ring to all
            // previous segments
            const auto &ring(s->links.ring);
            for (s = s->links.prev; s; s = s->links.prev) {
                s->links.ring = ring;
            }
        });

        const auto distributeRingNext([&](const Segment *s) -> void
        {
            // grab current ring, move skip current and write ring to all
            // next segments
            const auto &ring(s->links.ring);
            for (s = s->links.next; s; s = s->links.next) {
                s->links.ring = ring;
            }
        });

        // unify/create ring
        if (!pRing && !nRing) {
            // no ring, create for this segment
            s.links.ring = newRing(&s);
            // and copy to others (there is at most one segment in each
            // direction -> simple assignment)
            if (next) { next->links.ring = s.links.ring; }
            if (prev) { prev->links.ring = s.links.ring; }
        } else if (!pRing) {
            // distribute ring from next node to all its previous segments
            distributeRingPrev(next);
        } else if (!nRing) {
            // distribute ring from prev node to all its next segments
            distributeRingNext(prev);
        } else if (pRing != nRing) {
            // both valid but different, prefer prev segment
            distributeRingNext(prev);
        } else {
            // both are the same, just assing
            s.links.ring = pRing;
        }
    }

    void add(int x, int y, std::uint8_t type) {
        // 2x scale to get rid of non-integral values
        x *= 2;
        y *= 2;

        // LOG(info4) << "Adding inner: " << x << ", " << y;

        switch (type) {
        case b0000: return;

        case b0001:
            return addSegment(type, type, { x, y + 1 }, { x + 1, y + 2 });

        case b0010:
            return addSegment(type, type, { x + 1, y + 2 }, { x + 2, y + 1 });

        case b0011:
            return addSegment(type, type, { x, y + 1 }, { x + 2, y + 1 });

        case b0100:
            return addSegment(type, type, { x + 2, y + 1 }, { x + 1, y });

        case b0101: // b0111 + b1101
            addSegment(type, b0111, { x, y + 1 }, { x + 1, y });
            return addSegment(type, b1101, { x + 2, y + 1 }, { x + 1, y + 2 });

        case b0110:
            return addSegment(type, type, { x + 1, y + 2 }, { x + 1, y });

        case b0111:
            return addSegment(type, type, { x, y + 1 }, { x + 1, y });

        case b1000:
            return addSegment(type, type, { x + 1, y }, { x, y + 1 });

        case b1001:
            return addSegment(type, type, { x + 1, y }, { x + 1, y + 2 });

        case b1010: // b1011 + b1110
            addSegment(type, b1011, { x + 1, y }, { x + 2, y + 1 });
            return addSegment(type, b1110, { x + 1, y + 2 }, { x, y + 1 });

        case b1011:
            return addSegment(type, type, { x + 1, y }, { x + 2, y + 1 });

        case b1100:
            return addSegment(type, type, { x + 2, y + 1 }, { x, y + 1 });

        case b1101:
            return addSegment(type, type, { x + 2, y + 1 }, { x + 1, y + 2 });

        case b1110:
            return addSegment(type, type, { x + 1, y + 2 }, { x, y + 1 });

        case b1111: return;
        }
    }

    void addBorder(int x, int y, std::uint8_t type) {
        // 2x scale to get rid of non-integral values
        x *= 2;
        y *= 2;

        // LOG(info4) << "Adding border: " << x << ", " << y;

        switch (type) {
        case b0000: return;

        case b0001:
            addSegment(type, b0011, { x, y + 1 }, { x + 1, y + 1 });
            return addSegment(type, b1001, { x + 1, y + 1 }, { x + 1, y + 2 });

        case b0010:
            addSegment(type, b0110, { x + 1, y + 2 }, { x + 1, y + 1 });
            return addSegment(type, b0011, { x + 1, y + 1 }, { x + 2, y + 1 });

        case b0011:
            return addSegment(type, type, { x, y + 1 }, { x + 2, y + 1 });

        case b0100:
            addSegment(type, b1100, { x + 2, y + 1 }, { x + 1, y + 1 });
            return addSegment(type, b0110, { x + 1, y + 1 }, { x + 1, y });

        case b0101: // b0111 + b1101
            addSegment(type, b0110, { x, y + 1 }, { x, y });
            addSegment(type, b0011, { x, y }, { x + 1, y });
            addSegment(type, b1001, { x + 2, y + 1 }, { x + 2, y + 2 });
            return addSegment(type, b1100, { x + 2, y + 2 }, { x + 1, y + 2 });

        case b0110:
            return addSegment(type, type, { x + 1, y + 2 }, { x + 1, y });

        case b0111:
            addSegment(type, b0110, { x, y + 1 }, { x, y });
            return addSegment(type, b0011, { x, y }, { x + 1, y });

        case b1000:
            addSegment(type, b1001, { x + 1, y }, { x + 1, y + 1 });
            return addSegment(type, b1100, { x + 1, y + 1 }, { x, y + 1 });

        case b1001:
            return addSegment(type, type, { x + 1, y }, { x + 1, y + 2 });

        case b1010: // b1011 + b1110
            addSegment(type, b0011, { x + 1, y }, { x + 2, y });
            addSegment(type, b1001, { x + 2, y }, { x + 2, y + 1 });
            addSegment(type, b1100, { x + 1, y + 2 }, { x, y + 2 });
            return addSegment(type, b0110, { x, y + 2 }, { x, y + 1 });

        case b1011:
            addSegment(type, b0011, { x + 1, y }, { x + 2, y });
            return addSegment(type, b1001, { x + 2, y }, { x + 2, y + 1 });

        case b1100:
            return addSegment(type, type, { x + 2, y + 1 }, { x, y + 1 });

        case b1101:
            addSegment(type, b1001, { x + 2, y + 1 }, { x + 2, y + 2 });
            return addSegment(type, b1100, { x + 2, y + 2 }, { x + 1, y + 2 });

        case b1110:
            addSegment(type, b1100, { x + 1, y + 2 }, { x, y + 2 });
            return addSegment(type, b0110, { x, y + 2 }, { x, y + 1 });

        case b1111: return;
        }
    }

    math::Points2 extract(const Ring *ring) {
        LOG(info4) << "Processing ring starting at: " << ring->head;
        math::Points2 points;

        const Segment *s(ring->head);
        do {
            LOG(info4) << "    adding: " << s;

            if (s->links.ring.get() != ring) {
                LOGTHROW(info4, std::runtime_error)
                    << "Segment: " << s << " doesn't belong to ring "
                    << ring << " but " << s->links.ring << ".";
            }

            if (!s->links.next) {
                LOGTHROW(info4, std::runtime_error)
                    << "Segment: type: [" << std::bitset<4>(s->fullType) << "/"
                    << std::bitset<4>(s->type) << "] "
                    << "<" << s->start << " -> " << s->end << "> "
                    << s << " in ring "
                    << s->links.ring << " has no next segment.";
            }

            points.emplace_back(s->start(0) / 2.0, s->start(1) / 2.0);
            s = s->links.next;
        } while (s != ring->head);

        return points;
    }

    Contours extract() {
        Contours contours;

        LOG(info4) << "Rings: " << rings.size();

        for (const auto &ring : rings) {
            contours.push_back(extract(ring));
        }

        return contours;
    }

    Ring::set rings;
    SegmentMap segments;
};

} // namespace

Contours findContours(const bitfield::RasterMask &mask)
{
    (void) mask;

    ContourBuilder cb;

    const auto size(mask.dims());

    const auto getFlag([&](int x, int y, std::uint8_t flag) -> std::uint8_t
    {
        return (mask.get(x, y) ? flag : 0);
    });

    const auto add([&](int x, int y)
    {
        cb.add(x, y, getFlag(x, y + 1, 1)
               | getFlag(x + 1, y + 1, 2)
               | getFlag(x + 1,  y, 4)
               | getFlag(x, y, 8));
    });

    const auto addBorder([&](int x, int y)
    {
        cb.addBorder(x, y, getFlag(x, y + 1, 1)
                     | getFlag(x + 1, y + 1, 2)
                     | getFlag(x + 1,  y, 4)
                     | getFlag(x, y, 8));
    });

    int xend(size.width - 1);
    int yend(size.height - 1);

    // first row
    for (int i(-1); i <= xend; ++i) { addBorder(i, -1); }

    // all inner rows
    for (int j(0); j < yend; ++j) {
        // first column
        addBorder(-1, j);

        // all inner columns
        for (int i(0); i < xend; ++i) { add(i, j); }

        // last column
        addBorder(xend, j);
    }

    // last row
    for (int i(-1); i <= xend; ++i) { addBorder(i, yend); }

    return cb.extract();
}

} // namespace imgproc

#endif // imgproc_contour_hpp_included_
