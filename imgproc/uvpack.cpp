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
#include <iostream>
#include <fstream>

#include "dbglog/dbglog.hpp"

#include "error.hpp"
#include "uvpack.hpp"

namespace imgproc {

void UVRect::update(double x, double y)
{
    if (x < min.x) min.x = x;
    if (x > max.x) max.x = x;

    if (y < min.y) min.y = y;
    if (y > max.y) max.y = y;
}


bool UVRect::merge(UVRect& other)
{
    UVRect test;
    test.min = UVCoord(std::min(min.x, other.min.x),
                       std::min(min.y, other.min.y));
    test.max = UVCoord(std::max(max.x, other.max.x),
                       std::max(max.y, other.max.y));

    // success if merged rect has smaller area than the two original rects
    if (test.area() < this->area() + other.area())
    {
        this->min = test.min;
        this->max = test.max;
        return true;
    }

    return false;
}


RectPacker::Node* RectPacker::Node::findSpace(int rw, int rh)
{
    // if the node is too small (whether it's free or not), return immediately
    if (rw > width || rh > height) return 0;

    // the node is large enough and free, we're finished
    int area = rw*rh;
    if (rect == 0)
    {
        remaining -= area;
        return this;
    }

    // try searching in both subtrees
    for (int i = 0; i < 2; i++)
    {
        Node* node;
        if (son[i] && son[i]->remaining >= area
            && (node = son[i]->findSpace(rw, rh)))
        {
            remaining -= area;
            return node;
        }
    }

    // the rectangle won't fit
    return 0;
}


void RectPacker::pack()
{
    LOG(debug) << "Packing " << list.size() << " rectangles.";

    // sort rectangles by width
    std::sort(list.begin(), list.end(),
              [](const UVRect* a, const UVRect* b)
                { return a->width() > b->width(); } );

    // calculate total area of the rectangles
    long long total = 0;
    for (UVRect* rect : list)
        total += rect->width() * rect->height();
    LOG(debug) << "Total area: " << total << " pixels";

    // initialize packing area
    packWidth = packHeight = 64;
    while ((long long)(packWidth) * packHeight < total)
    {
        if (packWidth <= packHeight)
            packWidth *= 2;
        else
            packHeight *= 2;
    }
    LOG(debug) << "Initial packing area: " << packWidth << "x" << packHeight;

 retry:
    if (packWidth > (1<<17) || packHeight > (1<<17)) {
        LOGTHROW(err2, AreaTooLarge)
            << "Packing area too large ("
            << packWidth << "x" << packHeight << ").";
    }

    Node* root = new Node(0, 0, packWidth, packHeight);

    for (UVRect* rect : list)
    {
        int rw = rect->width(),
            rh = rect->height();

        // find a space to place the rect
        Node* node = root->findSpace(rw, rh);

        if (!node)
        {
            // if there is not enough room, double the space and start over
            if (packWidth <= packHeight)
                packWidth *= 2;
            else
                packHeight *= 2;

            LOG(debug) << "Rectangles won't fit, retrying with "
                       << packWidth << "x" << packHeight;

            delete root;
            goto retry;
        }

        // we have the final position for the rect
        rect->packX = node->x;
        rect->packY = node->y;

        // allocate the node and put the remaining free space into new nodes
        node->rect = rect;
        if (rh < node->height) {
            node->son[0] = new Node(node->x, node->y + rh,
                                    rw, node->height - rh);
        }
        if (rw < node->width) {
            node->son[1] = new Node(node->x + rw, node->y,
                                    node->width - rw, node->height);
        }
    }

    delete root;
    list.clear();
}

} // namespace imgproc
