#pragma once
#include <algorithm>
#include <cstdint>
#include <numeric>

#include "Base/Types.h"

namespace slug::baker
{

struct PackSize
{
    int w = 0;
    int h = 0;
};

struct PackPlacement
{
    int  page    = -1;
    int  x       = 0;
    int  y       = 0;
    bool bPlaced = false;
};

struct PackResult
{
    Vector<PackPlacement> placements = {};
    int                   pageCount  = 0;
};

[[nodiscard]] inline PackResult PackShelf(
    const Span<const PackSize> _sizes,
    const int                  _pageSize,
    const int                  _padding)
{
    struct Shelf
    {
        int y     = 0;
        int h     = 0;
        int nextX = 0;
    };
    struct Page
    {
        Vector<Shelf> shelves = {};
        int           nextY   = 0;
    };

    PackResult result;
    result.placements.assign(_sizes.size(), {});

    Vector<size_t> order(_sizes.size());
    std::iota(order.begin(), order.end(), size_t { 0 });
    std::stable_sort(order.begin(), order.end(), [&](const size_t _a, const size_t _b) {
        if (_sizes[_a].h != _sizes[_b].h)
        {
            return _sizes[_a].h > _sizes[_b].h;
        }
        return _sizes[_a].w > _sizes[_b].w;
    });

    Vector<Page> pages;
    for (const size_t index: order)
    {
        const PackSize& size = _sizes[index];
        if (size.w <= 0 || size.h <= 0)
        {
            result.placements[index] = { 0, 0, 0, true };
            continue;
        }

        const int footW = size.w + _padding;
        const int footH = size.h + _padding;
        if (footW + _padding > _pageSize || footH + _padding > _pageSize)
        {
            continue;
        }

        int  bestPage  = -1;
        int  bestShelf = -1;
        long bestWaste = 0;
        for (size_t p = 0; p < pages.size(); ++p)
        {
            Page& page = pages[p];
            for (size_t s = 0; s < page.shelves.size(); ++s)
            {
                const Shelf& shelf = page.shelves[s];
                if (shelf.h >= footH && shelf.nextX + footW <= _pageSize)
                {
                    const long waste = static_cast<long>(shelf.h - footH) * 4096 + (_pageSize - shelf.nextX - footW);
                    if (bestPage < 0 || waste < bestWaste)
                    {
                        bestPage  = static_cast<int>(p);
                        bestShelf = static_cast<int>(s);
                        bestWaste = waste;
                    }
                }
            }
        }

        if (bestPage < 0)
        {
            for (size_t p = 0; p < pages.size() && bestPage < 0; ++p)
            {
                if (pages[p].nextY + footH <= _pageSize && footW + _padding <= _pageSize)
                {
                    bestPage = static_cast<int>(p);
                }
            }
            if (bestPage < 0)
            {
                pages.push_back({ {}, _padding });
                bestPage = static_cast<int>(pages.size()) - 1;
            }

            Page& page = pages[static_cast<size_t>(bestPage)];
            page.shelves.push_back({ page.nextY, footH, _padding });
            page.nextY += footH;
            bestShelf = static_cast<int>(page.shelves.size()) - 1;
        }

        Shelf& shelf             = pages[static_cast<size_t>(bestPage)].shelves[static_cast<size_t>(bestShelf)];
        result.placements[index] = { bestPage, shelf.nextX, shelf.y, true };
        shelf.nextX += footW;
    }

    result.pageCount = static_cast<int>(pages.size());
    return result;
}

}   // namespace slug::baker
