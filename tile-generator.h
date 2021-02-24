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
#ifndef __TILE_GENERATOR_H_
#define __TILE_GENERATOR_H_
#include <cstddef>
#include <vips/vips.h>
#include <vips/vips8>

#include "utils.h"

using namespace vips;

typedef struct {
  Buffer buffer;
  unsigned index;
} Tile;

// Lazy loads each tile.
class VipsImageTileGenerator : public Generator<Tile> {
public:

  VipsImageTileGenerator(const VImage &source, unsigned tile_width, unsigned tile_height);
  virtual std::optional<Tile> Next(
    const std::optional<Tile> &prev, bool *end) const final;

  const std::optional<Tile> operator[] (unsigned i) const;

private:
  const bool ExtractTile(int x, int y, Buffer *out) const;

  const VImage &source_;
  const unsigned tile_width_;
  const unsigned tile_height_;

  const unsigned num_tiles_width_;
  const unsigned num_total_tiles_;
};
#endif // __TILE_GENERATOR_H_
