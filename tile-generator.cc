// Copyright 2021 Ellogon BV.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <cassert>
#include <cstring>
#include <memory>
#include <sstream>
#include <type_traits>
#include <vips/vips.h>
#include <vips/vips8>
#include <cstring>

#include "utils.h"
#include "tile-generator.h"

using namespace vips;

VipsImageTileGenerator::VipsImageTileGenerator(
  const VImage &source, unsigned tile_width, unsigned tile_height)
  : source_(source), tile_width_(tile_width), tile_height_(tile_height),
    num_tiles_width_(partition(source.width(), tile_width)),
    num_total_tiles_(num_tiles_width_ * partition(source.height(), tile_height)) {}

std::optional<Tile> VipsImageTileGenerator::Next(
  const std::optional<Tile> &prev, bool *end) const {
  assert(!*end);

  const unsigned tile_index = (prev) ? (*prev).index + 1 : 0;
  if (tile_index + 1 > num_total_tiles_) {
    *end = true;
    return {};
  }

  const VipsImageTileGenerator &ref = *this;
  return ref[tile_index];
}

const bool VipsImageTileGenerator::ExtractTile(int x, int y, Buffer *out) const {
  UniqueVipsRegion region(vips_region_new(source_.get_image()));

  const unsigned bands = source_.bands();
  const unsigned x_end = x + tile_width_;
  const unsigned y_end = y + tile_height_;
  const unsigned width = source_.width();
  const unsigned height = source_.height();

  int region_valid_width = (x_end > width) ? width - x : tile_width_;
  int region_valid_height = (y_end > height) ? height - y : tile_height_;

  VipsRect r = { x, y, region_valid_width, region_valid_height };

  if (vips_region_prepare(region.get(), &r))
    return false;

  const size_t tile_line_size = tile_width_ * bands;
  const size_t tile_size = tile_line_size * tile_height_;

  size_t region_line_size = VIPS_REGION_SIZEOF_LINE(region.get());
  size_t region_stride = VIPS_REGION_LSKIP(region.get());

  uint8_t *c = VIPS_REGION_ADDR_TOPLEFT(region.get());
  uint8_t *data = new uint8_t[tile_size]{};

  for (int i = 0; i < region_valid_height; ++i) {
    const size_t dst_offset = tile_line_size * i;
    const size_t src_offset = region_stride * i;
    memcpy(data + dst_offset, c + src_offset, region_line_size);
  }

  out->data.reset(data);
  out->size = tile_size;
  return true;
}

const std::optional<Tile> VipsImageTileGenerator::operator[](unsigned int i) const {
  const unsigned row = i / (num_tiles_width_);
  const unsigned column = i % num_tiles_width_;
  const int y = row * tile_height_;
  const int x = column * tile_width_;

  Tile tile{{}, i};
  if (!ExtractTile(x, y, &tile.buffer)) {
    fprintf(stderr, "Unable to extract tile.");
    return {};
  }
  return tile;
}
