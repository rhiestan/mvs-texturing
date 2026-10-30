/*
 * Copyright (C) 2015, Nils Moehrle
 * TU Darmstadt - Graphics, Capture and Massively Parallel Computing
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD 3-Clause license. See the LICENSE.txt file for details.
 */

#ifndef TEX_PIXELSET_HEADER
#define TEX_PIXELSET_HEADER

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

/**
 * Set of pixel coordinates of a width x height image. The pixels are kept in
 * a list in insertion order and a bitmap is used for the membership test.
 * This replaces std::set<std::pair<int, int> >, which allocates a node per
 * pixel; note that the iteration order is the insertion order, not sorted.
 */
class PixelSet {
    public:
        typedef std::vector<std::pair<int, int> > Pixels;
        typedef Pixels::const_iterator iterator;

        PixelSet(int width, int height)
            : width(width), flags(static_cast<std::size_t>(width) * height, 0) {}

        void insert(int x, int y) {
            std::uint8_t & flag = flags[static_cast<std::size_t>(y) * width + x];
            if (flag) return;
            flag = 1;
            pixels.emplace_back(x, y);
        }

        void clear(void) {
            for (std::pair<int, int> const & p : pixels) {
                flags[static_cast<std::size_t>(p.second) * width + p.first] = 0;
            }
            pixels.clear();
        }

        iterator begin(void) const {return pixels.begin();}
        iterator end(void) const {return pixels.end();}

    private:
        int width;
        std::vector<std::uint8_t> flags;
        Pixels pixels;
};

#endif /* TEX_PIXELSET_HEADER */
