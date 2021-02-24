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
#include <cmath>
#include <iostream>
#include <ostream>
#include <tiff.h>
#include <tiffio.h>
#include <vips/vips.h>
#include <vips/vips8>
#include <map>
#include <sstream>

#include "utils.h"
#include "spinners.h"
#include "tile-generator.h"
#include "aperio-svs-encoding.h"

#define TILE_SIZE 256

#ifdef WITH_SPINNER
static spinners::Spinner *spinner = nullptr;

struct sigaction old_action_int;
struct sigaction old_action_abrt;

static inline struct sigaction* get_sigaction(int signo) {
  switch (signo) {
    case SIGINT:
      return &old_action_int;
    case SIGABRT:
      return &old_action_abrt;
  }
  return nullptr;
}

static void receive_signal(int signo) {
  if (spinner)
    spinner->Stop();
  struct sigaction *old = get_sigaction(signo);
  sigaction(signo, old, nullptr);
  kill(0, signo);
}

static void arm_signal_handler() {
  struct sigaction sa = {};
  sa.sa_handler = receive_signal;
  sigaction(SIGINT, &sa, &old_action_int);   // Ctrl-C
  sigaction(SIGABRT, &sa, &old_action_abrt);   // abort()
}

#endif

enum class PageType { kTiled, kStriped };

enum class AperioDescriptionType { kThumbnailLayer, kNativeLayer, kSubLayer };

static inline int plateau(int idx, float k = 6.0) {
  return std::round((1 - std::exp(-k * idx)) * 100);
}

using Metadata = std::map<std::string, std::string>;

static const char *aperio_header = "Aperio Image Library v11.1.9";

static void init_tiff_page(TIFF *out, uint32_t width, uint32_t height,
                    uint16 page, uint32_t subfile_type) {
  TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);
  TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);
  TIFFSetField(out, TIFFTAG_IMAGEDEPTH, 1);
  TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 3);
  TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(out, TIFFTAG_YCBCRSUBSAMPLING, 2, 2);
  TIFFSetField(out, TIFFTAG_SUBFILETYPE, subfile_type);
}

static void aperio_describe_layer(AperioDescriptionType layer_type,
                                  const VImage &layer, const uint32_t native_width,
                                  const uint32_t native_height,
                                  const std::optional<unsigned> tile_size,
                                  const std::optional<int> jpeg_quality,
                                  const std::optional<Metadata> metadata, TIFF *out) {
  std::ostringstream ss;
  ss << aperio_header << std::endl;
  ss << native_width << "x" << native_height;
  if (layer_type == AperioDescriptionType::kNativeLayer ||
      layer_type == AperioDescriptionType::kSubLayer)
    ss << " (" << *tile_size << "x" << *tile_size << ")";

  if (layer_type == AperioDescriptionType::kThumbnailLayer ||
      layer_type == AperioDescriptionType::kSubLayer)
    ss << " -> " << layer.width() << "x" << layer.height();

  if (layer_type == AperioDescriptionType::kNativeLayer ||
      layer_type == AperioDescriptionType::kSubLayer) {
    ss << " JPEG/RGB ";
    if (jpeg_quality)
      ss << "Q=" << *jpeg_quality;
  } else
    ss << " - ";

  if (layer_type == AperioDescriptionType::kNativeLayer ||
      layer_type == AperioDescriptionType::kThumbnailLayer) {
    ss << ";Mirax Digital Slide";
    for (auto pair : *metadata) {
      ss << "|";
      ss << pair.first << " = " << pair.second;
    }
  }

  TIFFSetField(out, TIFFTAG_IMAGEDESCRIPTION, ss.str().c_str());
}

static void write_page(const VImage &in, const unsigned tile_size,
                       const std::optional<int> jpeg_quality,
                       PageType page_type, TIFF *out) {
  const uint32_t width = in.width();
  const uint32_t height = in.height();

#ifdef WITH_SPINNER
  spinner->SetText("Generating layer with size (" + std::to_string(width) + ", " + std::to_string(height) + ")");
#endif

  const int tile_width = (page_type == PageType::kStriped) ? width : tile_size;

  VImage cached = in.tilecache(
      VImage::option()
      ->set("tile_width", tile_width)
      ->set("tile_height", static_cast<int>(tile_size))
      ->set("persistent", false));

  init_tiff_page(out, width, height, 0, 0);

  // Set jpeg compression
  TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_JPEG);
  if (jpeg_quality)
    TIFFSetField(out, TIFFTAG_JPEGQUALITY, *jpeg_quality);

  if (page_type == PageType::kTiled) {
    TIFFSetField(out, TIFFTAG_TILEWIDTH, tile_size);
    TIFFSetField(out, TIFFTAG_TILELENGTH, tile_size);
  } else
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, tile_size);

  VipsImageTileGenerator tiles(cached, tile_width, tile_size);
  for (const std::optional<Tile> &tile : tiles) {
    assert(tile);
    const Buffer &buffer = (*tile).buffer;
    const unsigned index = (*tile).index;
    if (page_type == PageType::kTiled)
      TIFFWriteEncodedTile(out, index, buffer.data.get(), buffer.size);
    else
      TIFFWriteEncodedStrip(out, index, buffer.data.get(), buffer.size);
  }
  TIFFWriteDirectory(out);
}

// In case we need to implement some specific conversions.
static bool SvsMetadata2StringsMap(const SvsMetadata &data, Metadata *out) {
  if (data.app_mag)
    out->insert({"AppMag", std::to_string(*data.app_mag)});

  if (data.mpp)
    out->insert({"MPP", std::to_string(*data.mpp)});

  return true;
}

bool vips2svs_encoder(const VImage &in, const char *svs_out_filepath,
                      const std::vector<double> &scalings, SvsMetadata svs_metadata) {
  // Create our svs file
  errno = 0;
  TIFF* tiff = TIFFOpen(svs_out_filepath, "w");
  if (!tiff) {
    perror("TIFFOpen()");
    return false;
  }

  // native layer, subsampling layers and a thumbnail
  const int kNativeJpegQuality = plateau(1);

  const uint32_t native_width = in.width();
  const uint32_t native_height = in.height();

  Metadata metadata = {};
  if (!SvsMetadata2StringsMap(svs_metadata, &metadata)) {
    fprintf(stderr, "Could not encode metadata.");
    return false;
  }

#ifdef WITH_SPINNER
  spinner = new spinners::Spinner();
  arm_signal_handler();
  spinner->Start();
#endif

  // Generate first tiff directory.
  aperio_describe_layer(AperioDescriptionType::kNativeLayer,
                        in, native_width, native_height, TILE_SIZE,
                        kNativeJpegQuality, metadata, tiff);
  write_page(in, TILE_SIZE, kNativeJpegQuality, PageType::kTiled, tiff);

  // Generate the thumbnail.
  {
    const double scale_factor = (native_height > native_width)
      ? 768.0 / native_height
      : 1024.0 / native_width;
    VImage thumbnail = in.resize(scale_factor);
    aperio_describe_layer(AperioDescriptionType::kThumbnailLayer,
                          thumbnail, native_width, native_height, {},
                          {}, metadata, tiff);

    write_page(thumbnail, 16, {}, PageType::kStriped, tiff);
  }

  for (size_t i = 0; i < scalings.size(); ++i) {
    VImage new_layer = in.resize(1 / scalings[i]);
    const int jpeg_quality = plateau(i + 2);
    aperio_describe_layer(AperioDescriptionType::kSubLayer,
                          new_layer, native_width, native_height, TILE_SIZE,
                          jpeg_quality, {}, tiff);

    write_page(new_layer, TILE_SIZE, jpeg_quality, PageType::kTiled, tiff);
  }

#ifdef WITH_SPINNER
  spinner->Stop();
#endif

  // Close the file
  TIFFClose(tiff);
  return true;
}
