#include <memory>

#include "dbglog/dbglog.hpp"

#include "./error.hpp"
#include "./texturing.hpp"

namespace imgproc { namespace tx {

namespace {

typedef decltype(math::area(*static_cast<math::Size2*>(nullptr)))
    AreaType;

struct Node {
    bool allocated; ///< node is allocated
    int x, y; ///< this node position
    math::Size2 size; ///< this node size
    AreaType remaining; ///< space remaining in this node and its subtree
    Node *son[2]; ///< son[0] is space below, son[1] is space to the right

    Node(int x, int y, const math::Size2 &size)
        : allocated(false), x(x), y(y), size(size), remaining(math::area(size))
    {
        son[0] = son[1] = nullptr;
    }

    Node(const math::Size2 &size)
        : allocated(false), x(0), y(0), size(size), remaining(math::area(size))
    {
        son[0] = son[1] = nullptr;
    }

    ~Node() {
        delete son[0];
        delete son[1];
    }

    ///< find leaf large enough
    Node* findSpace(const math::Size2 &expected);
};

Node* Node::findSpace(const math::Size2 &expected)
{
    // if the node is too small (whether it's free or not), return immediately
    if ((expected.width > size.width) || (expected.height > size.height)) {
        return nullptr;
    }

    // the node is large enough and free, we're finished
    const auto area(math::area(expected));

    if (!allocated)  {
        remaining -= area;
        return this;
    }

    // try searching in both subtrees
    for (int i = 0; i < 2; ++i) {
        Node *node(nullptr);
        if (son[i] && (son[i]->remaining >= area)
            && (node = son[i]->findSpace(expected)))
        {
            remaining -= area;
            return node;
        }
    }

    // the rectangle won't fit
    return nullptr;
}

} // namespace

PackedTexture pack(const std::vector<math::Extents2> &uvPatches)
{
    PackedTexture result;
    result.patches.assign(uvPatches.begin(), uvPatches.end());
    auto &packSize(result.size);

    std::vector<Patch*> patches;
    patches.reserve(result.patches.size());
    for (auto &patch : result.patches) {
        patches.push_back(&patch);
    }

    LOG(debug) << "Packing " << patches.size() << " rectangles.";

    // sort rectangles by width
    std::sort(patches.begin(), patches.end(),
              [](const Patch *l, const Patch *r)
    {
        return l->size().width > r->size().width;
    });

    auto inflate([&]()
    {
        if (packSize.width <= packSize.height) {
            packSize.width *= 2;
        } else {
            packSize.height *= 2;
        }
    });

    // calculate total area of the rectangles
    {
        AreaType total(0);
        for (const auto *patch : patches) {
            total += math::area(patch->size());
        }
        LOG(debug) << "Total area: " << total << " pixels";

        // initialize packing area
        packSize = math::Size2(64, 64);

        while (math::area(packSize) < total) { inflate(); }
    }

    LOG(debug) << "Initial packing area: " << packSize << ".";

    // tries to pack patches into one texture, returns true when bigger
    // texturing pane is needed
    auto tryToPack([&]() -> bool
    {
        if ((packSize.width > (1<<17)) || (packSize.height > (1<<17))) {
            LOGTHROW(err2, AreaTooLarge)
                << "Packing area too large (" << packSize << ").";
        }

        Node root(packSize);

        for (auto *patch : patches) {
            const auto &psize(patch->size());

            // find a space to place the patchect
            auto *node(root.findSpace(psize));

            if (!node) { return true; }

            // we have the final position for the rect
            patch->place(math::Point2i(node->x, node->y));

            // allocate the node and put the remaining free space into new nodes
            node->allocated = true;

            if (psize.height < node->size.height) {
                node->son[0] = new Node
                    (node->x, node->y + psize.height
                     , math::Size2(psize.width
                                   , node->size.height - psize.height));
            }

            if (psize.width < node->size.width) {
                node->son[1] = new Node
                    (node->x + psize.width, node->y
                     , math::Size2(node->size.width - psize.width
                                   , node->size.height));
            }
        }

        return false;
    });

    // try to pack until patches fit
    while (tryToPack()) {
        // if there is not enough room, double the space and start over
        inflate();

        LOG(debug) << "Patch won't fit, retrying with "
                   << packSize.width << "x" << packSize.height << ".";
    }

    return result;
}

} } // namespace imgproc::tx
