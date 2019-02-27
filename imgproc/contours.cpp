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

#include <bitset>
#include <map>
#include <list>
#include <algorithm>
#include <limits>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include "dbglog/dbglog.hpp"

#include "math/geometry.hpp"

#include "utility/streams.hpp"

#include "contours.hpp"

namespace bmi = boost::multi_index;

namespace imgproc {

namespace {

typedef std::uint8_t CellType;

/** Image cell type
 */
enum : CellType {
    b0000 = 0x0, b0001 = 0x1, b0010 = 0x2, b0011 = 0x3
    , b0100 = 0x4, b0101 = 0x5, b0110 = 0x6, b0111 = 0x7
    , b1000 = 0x8, b1001 = 0x9, b1010 = 0xa, b1011 = 0xb
    , b1100 = 0xc, b1101 = 0xd, b1110 = 0xe, b1111 = 0xf
};

/** Segment orientation
 */
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

const char* arrow(Direction d) UTILITY_POSSIBLY_UNUSED;
inline const char* arrow(Direction d)
{
    switch (d) {
    case Direction::r: return "\xe2\x86\x92";
    case Direction::l: return "\xe2\x86\x90";
    case Direction::u: return "\xe2\x86\x91";
    case Direction::d: return "\xe2\x86\x93";
    case Direction::lu: return "\xe2\x86\x96";
    case Direction::ld: return "\xe2\x86\x99";
    case Direction::ru: return "\xe2\x86\x97";
    case Direction::rd: return "\xe2\x86\x98";
    }
    return "?";
}

typedef math::Point2i Vertex;
typedef math::Points2i Vertices;

struct Segment {
    CellType type;
    Direction direction;
    Vertex start;
    Vertex end;
    bool keystone;

    mutable const Segment *prev;
    mutable const Segment *next;
    mutable const Segment *ringLeader;

    Segment(CellType type, Direction direction, Vertex start, Vertex end
            , const Segment *prev, const Segment *next
            , bool keystone = false)
        : type(type), direction(direction)
        , start(start), end(end), keystone(keystone)
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

typedef std::vector<int> RingKeystones;
typedef std::vector<RingKeystones> MultiRingKeystones;

} // namespace

struct Builder {
    Builder(const math::Size2 &rasterSize, const ContourParameters &params)
        : params(&params), contour(rasterSize)
        , offset(params.pixelOrigin == PixelOrigin::center
                 ? math::Point2d() : math::Point2d(0.5, 0.5))
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
                    , const Vertex &start, const Vertex &end
                    , bool keystone = false);

    void addMitre(int i, int j, CellType type, CellType &mtype);

    void add(int i, int j, CellType type, CellType &mtype);

    void addAmbiguous(CellType type, int i, int j, CellType &mtype);

    void extract(const Segment *head);

    void setBorder(CellType type, int i, int j);

    const ContourParameters *params;
    SegmentMap segments;
    Contour contour;
    math::Point2d offset;
    MultiRingKeystones multiKeystones;
};

void Builder::setBorder(CellType type, int i, int j)
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

void Builder::addSegment(CellType type, Direction direction, int i, int j
                         , const Vertex &start, const Vertex &end
                         , bool keystone)
{
    setBorder(type, i, j);

    // mark in raster
    auto *prev(findByEnd(start));
    auto *next(findByStart(end));

    const auto &s
        (*segments.insert(Segment(type, direction, start, end
                                  , prev, next, keystone)).first);

    // LOG(info4) << "Segment " << s.start << " -> " << s.end << "> " << &s;

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

#define ADD_SEGMENT(D, X1, Y1, X2, Y2)                         \
    addSegment(type, Direction::D, i, j                        \
               , { x + X1, y + Y1 }, { x + X2, y + Y2 })

#define ADD_KEY_SEGMENT(D, X1, Y1, X2, Y2)                     \
    addSegment(type, Direction::D, i, j                        \
               , { x + X1, y + Y1 }, { x + X2, y + Y2 }, true)

void Builder::addAmbiguous(CellType type, int i, int j, CellType &mtype)
{
    // 2x scale to get rid of non-integral values
    auto x(i * 2);
    auto y(j * 2);

    Vertex v(x, y);

    // if mapped type not set, use it
    if (!mtype) { mtype = type; }

    if (mtype == type) {
        // same type
        switch (mtype) {
        case b0101: // b0111 + b1101
            ADD_SEGMENT(ru, 0, 1, 1, 0);
            return ADD_SEGMENT(ld, 2, 1, 1, 2);

        case b1010: // b1011 + b1110
            ADD_SEGMENT(rd, 1, 0, 2, 1);
            return ADD_SEGMENT(lu, 1, 2, 0, 1);
        }
    } else {
        // inverse type -> switch direction
        switch (mtype) {
        case b0101: // b1000 + b0010
            ADD_SEGMENT(ld, 1, 0, 0, 1);
            return ADD_SEGMENT(ru, 1, 2, 2, 1);

        case b1010: // b0100 + b0001
            ADD_SEGMENT(lu, 2, 1, 1, 0);
            return ADD_SEGMENT(rd, 0, 1, 1, 2);
        }
    }
}

void Builder::addMitre(int i, int j, CellType type, CellType &mtype)
{
    // 2x scale to get rid of non-integral values
    auto x(i * 2);
    auto y(j * 2);

    // LOG(info4) << "Adding mitre: " << x << ", " << y;

    switch (type) {
    case b0000: return;
    case b0001: return ADD_SEGMENT(rd, 0, 1, 1, 2);
    case b0010: return ADD_SEGMENT(ru, 1, 2, 2, 1);
    case b0011: return ADD_SEGMENT(r, 0, 1, 2, 1);
    case b0100: return ADD_SEGMENT(lu, 2, 1, 1, 0);
    case b0101: return addAmbiguous(type, i, j, mtype);
    case b0110: return ADD_SEGMENT(u, 1, 2, 1, 0);
    case b0111: return ADD_SEGMENT(ru, 0, 1, 1, 0);
    case b1000: return ADD_SEGMENT(ld, 1, 0, 0, 1);
    case b1001: return ADD_SEGMENT(d, 1, 0, 1, 2);
    case b1010: return addAmbiguous(type, i, j, mtype);
    case b1011: return ADD_SEGMENT(rd, 1, 0, 2, 1);
    case b1100: return ADD_SEGMENT(l, 2, 1, 0, 1);
    case b1101: return ADD_SEGMENT(ld, 2, 1, 1, 2);
    case b1110: return ADD_SEGMENT(lu, 1, 2, 0, 1);
    case b1111: return;
    }
}

void Builder::add(int i, int j, CellType type, CellType &mtype)
{
    // 2x scale to get rid of non-integral values
    auto x(i * 2);
    auto y(j * 2);

    // LOG(info4) << "Adding : " << x << ", " << y;

    switch (type) {
    case b0000: return;

    case b0001:
        ADD_SEGMENT(r, 0, 1, 1, 1);
        return ADD_KEY_SEGMENT(d, 1, 1, 1, 2);

    case b0010:
        ADD_SEGMENT(u, 1, 2, 1, 1);
        return ADD_KEY_SEGMENT(r, 1, 1, 2, 1);

    case b0011: return ADD_SEGMENT(r, 0, 1, 2, 1);

    case b0100:
        ADD_SEGMENT(l, 2, 1, 1, 1);
        return ADD_KEY_SEGMENT(u, 1, 1, 1, 0);

    case b0101: return addAmbiguous(type, i, j, mtype);

    case b0110: return ADD_SEGMENT(u, 1, 2, 1, 0);

    case b0111:
        ADD_SEGMENT(r, 0, 1, 1, 1);
        return ADD_KEY_SEGMENT(u, 1, 1, 1, 0);

    case b1000:
        ADD_SEGMENT(d, 1, 0, 1, 1);
        return ADD_KEY_SEGMENT(l, 1, 1, 0, 1);

    case b1001: return ADD_SEGMENT(d, 1, 0, 1, 2);

    case b1010: return addAmbiguous(type, i, j, mtype);

    case b1011:
        ADD_SEGMENT(d, 1, 0, 1, 1);
        return ADD_KEY_SEGMENT(r, 1, 1, 2, 1);

    case b1100: return ADD_SEGMENT(l, 2, 1, 0, 1);

    case b1101:
        ADD_SEGMENT(l, 2, 1, 1, 1);
        return ADD_KEY_SEGMENT(d, 1, 1, 1, 2);

    case b1110:
        ADD_SEGMENT(u, 1, 2, 1, 1);
        return ADD_KEY_SEGMENT(l, 1, 1, 0, 1);

    case b1111: return;
    }
}

#undef ADD_SEGMENT

inline void log(const Segment *s, const math::Point2d &offset)
{
#if 0
    LOG(info4) << "    [" << std::bitset<4>(s->type) << "]: "
               << arrow(s->direction) << " (" << s->direction
               << "): " << s->start
               << " -> " << s->end
               << " [" << (s->start(0) / 2.0 + offset(0))
               << "," << (s->start(1) / 2.0 + offset(1))
               << " -> " << (s->end(0) / 2.0 + offset(0))
               << "," << (s->end(1) / 2.0 + offset(1))
               << "]";
#else
    (void) s;
    (void) offset;
#endif
}

void Builder::extract(const Segment *head)
{
    // LOG(info4) << "Processing ring from: " << head;
    const auto *ringLeader(head);

    contour.rings.emplace_back();
    auto &ring(contour.rings.back());

    multiKeystones.emplace_back();
    auto &keystones(multiKeystones.back());

    const auto addVertex([&](const Segment &s) {
#if 0
            LOG(info4) << "        * added "
                       << (s.start(0) / 2.0 + offset(0))
                       << "," << (s.start(1) / 2.0 + offset(1))
                       << " -> " << (s.end(0) / 2.0 + offset(0))
                       << "," << (s.end(1) / 2.0 + offset(1));
#endif
            ring.emplace_back(s.start(0) / 2.0 + offset(0)
                              , s.start(1) / 2.0 + offset(1));
        });

    // end segment
    const Segment *end(head);

    switch (params->simplification) {
    case ChainSimplification::none:
        // nothing special, just add first vertex
        log(head, offset);
        addVertex(*head);
        break;

    case ChainSimplification::simple:
    case ChainSimplification::rdp:
        // find first direction break
        while (head->direction == head->prev->direction) {
            head = head->prev;
        }

        log(head, offset);
        addVertex(*head);
        end = head;

        if ((params->simplification == ChainSimplification::rdp)
            && (head->keystone))
         {
             // remember keystone index for head
             keystones.push_back(0);
        }
        break;
    }

    // process full ringLeader
    for (const auto *p(head), *s(head->next); s != end; p = s, s = s->next)
    {
        if (s->ringLeader != ringLeader) {
            LOGTHROW(info4, std::runtime_error)
                << "Segment: " << s << " doesn't belong to ring "
                << ringLeader << " but " << s->ringLeader << ".";
        }

        if (!s->next) {
            LOGTHROW(info4, std::runtime_error)
                << "Segment: type: [" << std::bitset<4>(s->type) << "/"
                << s->direction << "] "
                << "<" << s->start << " -> " << s->end << "> "
                << s << " in ringLeader "
                << s->ringLeader << " has no next segment.";
            }

        log(s, offset);

        // add vertex only when direction differs
        switch (params->simplification) {
        case ChainSimplification::none:
            addVertex(*s);
            break;

        case ChainSimplification::simple:
            if (s->direction != p->direction) {
                addVertex(*s);
            }
            break;

        case ChainSimplification::rdp:
            if (s->keystone) {
                // remember keystone index
                keystones.push_back(int(ring.size()));
                addVertex(*s);
            } else if (s->direction != p->direction) {
                addVertex(*s);
            }
            break;
        }
    }
}

namespace {

/** Perpendicular distance of point `p` from line defined by points `s` and `e`.
 */
class PointDistance {
public:
    PointDistance(const math::Point2d &s, const math::Point2d &e)
        : s_(s), e_(e), diff_(e_ - s_)
        , length_(boost::numeric::ublas::norm_2(diff_))
        , tail_(e_(0) * s_(1) - e_(1) * s_(0))
    {}

    double operator()(const math::Point2d &p) const {
        auto d(std::abs(p(0) * diff_(1) - p(1) * diff_(0) + tail_) / length_);
        return d;
    }

private:
    const math::Point2d s_;
    const math::Point2d e_;
    const math::Point2d diff_;
    const double length_;
    const double tail_;
};

const math::Point2d Infinity(std::numeric_limits<double>::max()
                             , std::numeric_limits<double>::max());

struct RDP {
    RDP(const math::Polygon &r, RingKeystones keystones, double epsilon)
        : ring_(r), epsilon_(epsilon)
        , size_(ring_.size())
        , valid_(ring_.size(), false)
        , flip_(false)
    {
        // too small polygon -> as-is
        if (ring_.size() < 5) {
            std::fill(valid_.begin(), valid_.end(), true);
            return;
        }

        // start vertex: either first keystone or start of polygon
        if (keystones.empty()) {
            // no keystone, use point that is lexicographically less
            std::size_t start(0);
            for (std::size_t i(1); i != size_; ++i) {
                if (ring_[i] < ring_[start]) { start = i; }
            }

            start = flip(start, keystones);

            const auto pivot(start + (size_ / 2));
            valid_[start] = valid_[normalize(pivot)] = true;
            process(start, pivot);
            process(pivot, start + size_);
            return;
        }

        // flip at fist keystone
        flip(keystones.front(), keystones);

        if (keystones.size() == 1) {
            // just one keystone, inject artificial one
            const auto k(keystones.front());
            auto pivot(size_ / 2);
            valid_[k] = valid_[normalize(k + pivot)] = true;
            process(k, k + pivot);
            process(k + pivot, k + size_);
            return;
        }

        // more keystones available, we can work piecewise
        // two keystones available: use it as a full first segment
        auto prev(keystones.back());
        for (auto i : keystones) {
            valid_[i] = true;
            process(prev, (prev > i) ? i + size_ : i);
            prev = i;
        }
    }

    math::Polygon operator()() const {
        math::Polygon out;

        auto ivalid(valid_.begin());
        for (const auto &p : ring_) {
            if (*ivalid++) { out.push_back(p); }
        }

        // reverse back
        if (flip_) { std::reverse(out.begin(), out.end()); }

        return out;
    }

private:
    inline std::size_t normalize(std::size_t index) const {
        return index % size_;
    }

    inline const math::Point2d& get(std::size_t index) const {
        return ring_[normalize(index)];
    }

    std::size_t flip(std::size_t point, RingKeystones &keystones) {
        // always normalize point itself
        point = normalize(point);

        // previous point in the ring, normalized (covers case (point == 0))
        const auto prev(normalize(point + size_ - 1));

        // orientation
        flip_ = (math::ccw(ring_[prev] // already normalized
                           , ring_[point] // already normalized
                           , ring_[normalize(point + 1)])
                 < 0.0);
        if (flip_) {
            // reverse ring
            std::reverse(ring_.begin(), ring_.end());

            // flip point index, normalized
            point = size_ - point - 1;

            // reverse keystones
            for (auto &p : keystones) { p = int(size_) - p - 1; }
        }

        return point;
    }

    /** Simplify segment between points ring[start] and ring[end]
     */
    void process(std::size_t start, std::size_t end) {
        if ((end - start) < 4) {
            // too short segment, include
            for (; start < end; ++start) {
                valid_[normalize(start)] = true;
            }
            return;
        }

        const PointDistance distance(get(start), get(end));

        double maxDistance(0.0);
        std::size_t outlierIndex(0);
        math::Point2d outlier(Infinity);
        for (auto i(start + 1); i < end; ++i) {
            const auto &p(get(i));
            const auto d(distance(p));
            if ((d > maxDistance) || ((d == maxDistance) && (p < outlier))) {
                maxDistance = d;
                outlierIndex = i;
                outlier = p;
            }
        }

        if (maxDistance > epsilon_) {
            // too much curvature -> split
            valid_[normalize(start)] = true;
            valid_[normalize(outlierIndex)] = true;
            valid_[normalize(end)] = true;
            process(start, outlierIndex);
            process(outlierIndex, end);
            return;
        }

        // in bounds, can replace with straight segment
        valid_[normalize(start)] = true;
        valid_[normalize(end)] = true;
    }

    math::Polygon ring_;
    const double epsilon_;
    const std::size_t size_;
    std::vector<char> valid_; // do not use bool, vector<bool> sucks
    bool flip_;
};

} // namespace

struct FindContours::Impl {
    Impl(const math::Size2 &rasterSize, int colorCount
         , const ContourParameters &params)
        : size(rasterSize), colors(colorCount), params(params)
        , cells(colors)
    {
        for (int i(0); i < colors; ++i) {
            builders.emplace_back(size, params);
        }
    }

    void feed(int x, int y, int ul, int ur, int lr, int ll);

    const math::Size2 size;
    const int colors;
    const ContourParameters params;

    // this optimization of storage makes this class non-reentrant!
    std::vector<CellType> cells;

    std::vector<Builder> builders;
};

FindContours::FindContours(const math::Size2 &rasterSize, int colorCount
                           , const ContourParameters &params)
    : impl_(new Impl(rasterSize, colorCount, params))
{}

FindContours::~FindContours() {}

void FindContours::operator()(int x, int y, int ul, int ur, int lr, int ll)
{
    impl_->feed(x, y, ul, ur, lr, ll);
}

const math::Size2 FindContours::rasterSize() const {
    return impl_->size;
}

Contour::list FindContours::contours() {
    if (impl_->params.simplification == ChainSimplification::rdp) {
        // simplify rings
        for (auto &builder : impl_->builders) {
            auto imultiKeystones(builder.multiKeystones.begin());
            for (auto &ring : builder.contour.rings) {
                ring = RDP(ring, *imultiKeystones++
                           , impl_->params.rdpMaxError)();
            }
        }
    }

    // steal contours
    Contour::list contours;
    for (auto &builder : impl_->builders) {
        contours.push_back(std::move(builder.contour));
    }
    return contours;
}

void FindContours::Impl::feed(int x, int y, int ul, int ur, int lr, int ll)
{
    const auto cellValue([&](int c) -> CellType
    {
        return ((ul == c) << 3 | (ur == c) << 2 | (lr == c) << 1 | (ll == c));
    });

    // compute cell value for all cells and count non-zero cells (i.e. number of
    // different areas meeting in this cell)
    int cardinality(0);
    for (int c(0); c < colors; ++c) {
        cardinality += bool((cells[c] = cellValue(c)));
    }
    // virtual areas for invalid pixels (<0) and border pixels (>= colors)
    cardinality += ((ul < 0) || (ur < 0) || (lr < 0) || (ll < 0));
    cardinality += ((ul >= colors) || (ur  >= colors)
                    || (lr  >= colors) || (ll  >= colors));

    CellType ambiguous(0);

    auto icells(cells.begin());
    if (cardinality > 2) {
        // more than two areas meet at this place, use 90 degree connection
        for (auto &builder : builders) {
            builder.add(x, y, *icells++, ambiguous);
        }
    } else {
        // use mitre connections
        for (auto &builder : builders) {
            builder.addMitre(x, y, *icells++, ambiguous);
        }
    }
}

Contour findContour(const Contour::Raster &raster
                    , const ContourParameters &params)
{
    const auto size(raster.dims());

    Builder cb(size, params);

    const auto getFlags([&](int x, int y) -> CellType
    {
        return (CellType(raster.get(x, y + 1))
                | CellType((raster.get(x + 1, y + 1)) << 1)
                | CellType((raster.get(x + 1, y)) << 2)
                | CellType((raster.get(x, y)) << 3));
    });

    CellType dummy(0);

#define ADD_MITRE(x, y) cb.addMitre(x, y, getFlags(x, y), dummy)
#define ADD(x, y) cb.add(x, y, getFlags(x, y), dummy)

    int xend(size.width - 1);
    int yend(size.height - 1);

    // first row
    for (int i(-1); i <= xend; ++i) { ADD(i, -1); }

    // all inner rows
    for (int j(0); j < yend; ++j) {
        // first column
        ADD_MITRE(-1, j);

        // all inner columns
        for (int i(0); i < xend; ++i) { ADD_MITRE(i, j); }

        // last column
        ADD(xend, j);
    }

    // last row
    for (int i(-1); i <= xend; ++i) { ADD(i, yend); }

#undef ADD_MITRE
#undef ADD

    if (params.simplification == ChainSimplification::rdp) {
        // simplify rings
        auto imultiKeystones(cb.multiKeystones.begin());
        for (auto &ring : cb.contour.rings) {
            ring = RDP(ring, *imultiKeystones++, params.rdpMaxError)();
        }
    }

    // steal contour
    return std::move(cb.contour);
}

} // namespace imgproc
