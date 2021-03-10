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
#include <memory>

#include "dbglog/dbglog.hpp"

#include "error.hpp"
#include "texturing.hpp"

namespace imgproc { namespace tx {

namespace {

typedef decltype(math::area(*static_cast<math::Size2*>(nullptr)))
    AreaType;

const math::Size2 MaxSize(1 << 17, 1 << 17);

class Node {
public:
    Node(int x, int y, int width, int height)
        : allocated(false), x(x), y(y), width(width), height(height)
        , remaining(width * height)
        , below(), right()
    {}

    Node(const math::Size2 &size)
        : allocated(false), x(0), y(0)
        , width(size.width), height(size.height)
        , remaining(math::area(size))
        , below(), right()
    {}

    ~Node() {
        delete below;
        delete right;
    }

    ///< allocate leaf for patch
    bool allocateSpace(Patch &patch) {
        const auto &patchSize(patch.size());
        return allocateSpace(patch, patchSize, math::area(patchSize));
    }

private:
    bool allocateSpace(Patch &patch, const math::Size2 &patchSize
                       , AreaType patchArea);

    void assign(Patch &patch, const math::Size2 &patchSize, AreaType area);

    bool allocated; ///< node is allocated
    int x, y; ///< this node position
    int width; ///< this node width
    int height; ///< this node height
    AreaType remaining; ///< space remaining in this node and its subtree
    Node *below;
    Node *right;
};

void Node::assign(Patch &patch, const math::Size2 &psize, AreaType patchArea)
{
    // mark as allocated
    allocated = true;

    // update current area
    remaining -= patchArea;

    // we have the final position for the rect
    patch.place(math::Point2i(x, y));

    // create node below this node if there is any space left
    if (psize.height < height) {
        below = new Node(x, y + psize.height
                         , psize.width, height - psize.height);
    }

    // create node to the right of this node if there is any space left
    if (psize.width < width) {
        right = new Node(x + psize.width, y
                         , width - psize.width, height);
    }
}

bool Node::allocateSpace(Patch &patch, const math::Size2 &patchSize
                         , AreaType patchArea)
{
    // if the node is too small (whether it's free or not), return immediately
    if ((patchSize.width > width) || (patchSize.height > height)) {
        return false;
    }

    // the node is large enough and free, we're finished
    if (!allocated) {
        // node is available, assign patch
        assign(patch, patchSize, patchArea);
        return true;
    }

    // descend to child node
    auto descend([&](Node *child) -> bool
    {
        // sanity check before descent
        if (!child || (child->remaining < patchArea)) { return {}; }

        // descend to child
        auto spaceAllocated(child->allocateSpace(patch, patchSize, patchArea));

        // update area if patch has been assigned its new position
        if (spaceAllocated) { remaining -= patchArea; }

        // return result
        return spaceAllocated;
    });

    // try searching in both subtrees and return result
    return descend(below) || descend(right);
}

} // namespace

math::Size2 pack(Patch::list &patches, float scale,
                 boost::optional<math::Size2i> maxAllowed)
{
    LOG(debug) << "Packing " << patches.size() << " rectangles.";

    // sort rectangles by width
    std::sort(patches.begin(), patches.end(),
              [](const Patch *l, const Patch *r)
    {
        return l->size().width > r->size().width;
    });

    // initial size
    math::Size2 packSize(64, 64);

    auto inflate([&]()
    {
        if (maxAllowed && packSize == *maxAllowed) {
            LOGTHROW(err2, AreaTooLarge)
                << "Won't fit: maximum (allowed) size reached: " << maxAllowed
                << ".";
        }

        // inflate area
        if (packSize.width <= packSize.height) {
            packSize.width *= scale;

            if (maxAllowed) {
                packSize.width = std::min(packSize.width, maxAllowed->width);
            }
        } else {
            packSize.height *= scale;

            if (maxAllowed) {
                packSize.height = std::min(packSize.height, maxAllowed->height);
            }
        }

        // and check
        if ((packSize.width > MaxSize.width)
            || (packSize.height > MaxSize.height))
        {
            LOGTHROW(err2, AreaTooLarge)
                << "Packing area too large (" << packSize << ").";
        }
    });

    // calculate total area of the rectangles
    {
        AreaType total(0);
        for (const auto *patch : patches) {
            total += math::area(patch->size());
        }
        // LOG(debug) << "Total area: " << total << " pixels";
        LOG(debug) << "Total area: " << total << " pixels";

        // inflate to hold total area
        while (math::area(packSize) < total) { inflate(); }
    }

    // LOG(debug) << "Initial packing area: " << packSize << ".";
    LOG(debug) << "Initial packing area: " << packSize << ".";

    // tries to pack patches into one texture, returns false when bigger
    // texturing pane is needed
    auto tryToPack([&]() -> bool
    {
        Node root(packSize);

        for (auto *patch : patches) {
            // try to allocate space for this patch
            if (!root.allocateSpace(*patch)) {
                // report failure
                return false;
            }
        }

        // no failure
        return true;
    });

    // try to pack until patches fit
    while (!tryToPack()) {
        // if there is not enough room, double the space and start over
        inflate();

        LOG(debug) << "Patch won't fit, retrying with "
                   << packSize.width << "x" << packSize.height << ".";
    }

    LOG(debug) << "Packed size: " << packSize;

    return packSize;
}

} } // namespace imgproc::tx
