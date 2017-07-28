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

namespace imgproc {

namespace {

typedef int VertexIndex;
typedef int SegmentIndex;

typedef math::Point2i Vertex;
typedef math::Points2i Vertices;

struct Segment {
    std::uint8_t type;
    Vertex start;
    Vertex end;
    SegmentIndex next;
    SegmentIndex prev;
    SegmentIndex ring;

    Segment(std::uint8_t type, Vertex start, Vertex end, SegmentIndex ring)
        : type(type), start(start), end(end), next(-1), prev(-1), ring(ring)
    {}

    typedef std::vector<Segment> list;
    typedef std::map<Vertex, SegmentIndex> map;
};


struct ContourBuilder {
    SegmentIndex findSegment(const Segment::map &where, const Vertex &v)
        const
    {
        auto fwhere(where.find(v));
        return ((fwhere == where.end()) ? -1 : fwhere->second);
    }

    Segment* get(SegmentIndex si) {
        if (si < 0) { return nullptr; }
        return &segments[si];
    }

    Segment* getPrev(const Segment &s) { return get(s.prev); }
    Segment* getNext(const Segment &s) { return get(s.next); }

    void commonRing(Segment &s1, Segment &s2)
    {
        if (s1.ring < s2.ring) {
            auto *current(&s2);
            do {
                current->ring = s1.ring;
            } while ((current = getNext(*current)));
        } else if (s2.ring < s1.ring) {
            auto *current(&s1);
            do {
                current->ring = s2.ring;
            } while ((current = getPrev(*current)));
        }
    }

    void addSegment(std::uint8_t type, const Vertex &start, const Vertex &end)
    {
        // make room for new segment
        if (segments.capacity() == segments.size()) {
            segments.reserve(2 * segments.size());
        }

        // find indices of previous and next segments
        auto prevIndex(findSegment(segmentsByEnd, start));
        auto nextIndex(findSegment(segmentsByStart, end));

        auto *prev(get(prevIndex));
        auto *next(get(nextIndex));

        // create segment
        const auto si(segments.size());
        segments.emplace_back(type, start, end, si);
        auto &s(segments.back());

        bool startRecorded(false);
        bool endRecorded(false);

        // link previous segment
        if (prev) {
            // linke there
            s.prev = prevIndex;
            // and back
            prev->next = si;
            // use common ring
            commonRing(*prev, s);
        }

        // link next segment
        if (next) {
            // link there
            s.next = nextIndex;
            // and back
            next->prev = si;
            // use common ring
            commonRing(s, *next);
        }

        // remember segment by point
        if (!startRecorded) {
            segmentsByStart.insert(Segment::map::value_type(s.start, si));
        }

        if (!endRecorded) {
            segmentsByEnd.insert(Segment::map::value_type(s.end, si));
        }
    }

    void add(int x, int y, std::uint8_t type) {
        // LOG(info4) << "(" << x << ", " << y << "): " << std::bitset<4>(type);

        // 2x scale to get rid of non-integral values
        x *= 2;
        y *= 2;

        switch (type) {
        case 0x0: return;

        case 0x1: return addSegment(type, { x, y + 1 }, { x + 1, y + 2 });
        case 0x2: return addSegment(type, { x + 1, y + 2 }, { x + 2, y + 1 });
        case 0x3: return addSegment(type, { x, y + 1 }, { x + 2, y + 1 });
        case 0x4: return addSegment(type, { x + 2, y + 1 }, { x + 1, y });
        case 0x5:
            addSegment(0x7, { x, y + 1 }, { x + 1, y });
            return addSegment(0xd, { x + 2, y + 1 }, { x + 1, y + 2 });
        case 0x6: return addSegment(type, { x + 1, y + 2 }, { x + 1, y });
        case 0x7: return addSegment(type, { x, y + 1 }, { x + 1, y });
        case 0x8: return addSegment(type, { x + 1, y }, { x, y + 1 });
        case 0x9: return addSegment(type, { x + 1, y }, { x + 1, y + 2 });
        case 0xa: // 0xb + 0xe
            addSegment(0xb, { x + 1, y }, { x + 2, y + 1 });
            return addSegment(0xe, { x + 1, y + 2 }, { x, y + 1 });
        case 0xb: return addSegment(type, { x + 1, y }, { x + 2, y + 1 });
        case 0xc: return addSegment(type, { x + 2, y + 1 }, { x, y + 1 });
        case 0xd: return addSegment(type, { x + 2, y + 1 }, { x + 1, y + 2 });
        case 0xe: return addSegment(type, { x + 1, y + 2 }, { x, y + 1 });
        case 0xf: return;
        }
    }

    math::Points2 extract(SegmentIndex start) {
        LOG(info4) << "Processing ring: " << start;
        math::Points2 points;

        SegmentIndex si(start);
        do {
            LOG(info4) << "    adding: " << si;
            const auto &s(segments[si]);
            if (s.ring != start) {
                LOGTHROW(info4, std::runtime_error)
                    << "Segment: " << si << " doesn't belong to ring "
                    << start << " but " << s.ring << ".";
            }
            if (s.next < 0) {
                LOGTHROW(info4, std::runtime_error)
                    << "Segment: " << si << " in ring "
                    << s.ring << " has no next segment.";
            }

            points.emplace_back(s.start(0) / 2.0, s.start(1) / 2.0);
            si = s.next;
        } while (si != start);

        return points;
    }

    Contours extract() {
        Contours contours;

        SegmentIndex ring(-1);
        for (SegmentIndex si(0), se(segments.size()); si < se; ++si) {
            const auto &s(segments[si]);
            if (s.ring == ring) { continue; }
            contours.push_back(extract(si));
            ring = si;
        }

        return contours;
    }

    Segment::list segments;
    Segment::map segmentsByStart;
    Segment::map segmentsByEnd;
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

    for (int j(-1), je(size.height); j != je; ++j) {
        for (int i(-1), ie(size.width); i != ie; ++i) {
            cb.add(i, j, getFlag(i, j + 1, 1)
                   | getFlag(i + 1, j + 1, 2)
                   | getFlag(i + 1,  j, 4)
                   | getFlag(i, j, 8));
        }
    }

    return cb.extract();
}

} // namespace imgproc

#endif // imgproc_contour_hpp_included_
