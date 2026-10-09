#pragma once
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <span>
#include <vector>

namespace slug::baker
{

// ===========================================
//  Shelf 아틀라스 패커
// ===========================================

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
    bool bPlaced = false;   // 페이지보다 커서 못 넣은 경우 false
};

struct PackResult
{
    std::vector<PackPlacement> placements;   // 입력 순서와 동일
    int                        pageCount = 0;
};

// 높이 내림차순으로 정렬한 뒤 가장 낭비가 적은 shelf 에 넣는다 (best fit).
// 페이지 가장자리와 사각형 사이, 사각형끼리 최소 _padding 픽셀의 간격을 보장한다.
// 크기가 0 인 사각형은 배치하지 않고 (page 0, 0, 0, bPlaced = true) 로 둔다.
[[nodiscard]] inline PackResult PackShelf(
    const std::span<const PackSize> _sizes,
    const int                       _pageSize,
    const int                       _padding)
{
    struct Shelf
    {
        int y    = 0;
        int h    = 0;
        int nextX = 0;
    };
    struct Page
    {
        std::vector<Shelf> shelves;
        int                nextY = 0;
    };

    PackResult result;
    result.placements.assign(_sizes.size(), {});

    std::vector<size_t> order(_sizes.size());
    std::iota(order.begin(), order.end(), size_t { 0 });
    std::stable_sort(order.begin(), order.end(), [&](const size_t _a, const size_t _b)
    {
        if (_sizes[_a].h != _sizes[_b].h)
        {
            return _sizes[_a].h > _sizes[_b].h;
        }
        return _sizes[_a].w > _sizes[_b].w;
    });

    std::vector<Page> pages;
    for (const size_t index: order)
    {
        const PackSize& size = _sizes[index];
        if (size.w <= 0 || size.h <= 0)
        {
            result.placements[index] = { 0, 0, 0, true };
            continue;
        }

        // 가장자리 여백(_padding) + 사각형 + 뒤쪽 간격(_padding)
        const int footW = size.w + _padding;
        const int footH = size.h + _padding;
        if (footW + _padding > _pageSize || footH + _padding > _pageSize)
        {
            continue;   // bPlaced = false
        }

        int    bestPage  = -1;
        int    bestShelf = -1;
        long   bestWaste = 0;
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
            // 새 shelf: 기존 페이지 중 높이가 남는 곳 -> 없으면 새 페이지
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
