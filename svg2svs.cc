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
#include <cerrno>
#include <cstdio>
#include <cmath>
#include <climits>
#include <iostream>
#include <sstream>
#include <string>
#include <filesystem>
#include <algorithm>
#include <set>
#include <sstream>
#include <getopt.h>
#include <tiff.h>
#include <tiffio.h>
#include <vips/vips.h>
#include <vips/vips8>
#include <cassert>

#include "aperio-svs-encoding.h"

using namespace vips;

static int usage(const char *prog, const char *msg = nullptr) {
  if (msg && strlen(msg))
    fprintf(stderr, "\033[1m\033[31m%s\033[0m\n\n", msg);
  fprintf(stderr, "Usage: %s [options] <input-svg-filename> <output-svs-filename>\n"
          "Options:\n", prog);
  fprintf(stderr,
          "  -b, --base-width <width>                    : Width of the base of the pyramid. (Default 16000)\n"
          "  -l, --layers-factors <factor> [<factor>,...]: Downsampling factors for each layer of the pyramid. (Default 4,16,64)\n"
          "  -h, --help                                  : Display this help text and exit.\n");
  return (msg) ? 1 : 0;
}

static struct option long_options[] = {
  { "help", no_argument, 0, 'h' },
  { "base-width", required_argument, 0, 'b'},
  { "layers-factors", required_argument, 0, 'l'},
  { 0, 0, 0, 0 },
};

static bool parse_and_set_layers_factors(char *const layers_factors, std::vector<double> *out) {
  out->clear();
  char *pos = layers_factors;
  while (*pos != '\0') {
    errno = 0;
    const double factor = strtod(pos, &pos);
    if (factor == 0.0 || errno > 0) {
      return false;
    }
    if (*pos == ',')
      pos++;
    out->push_back(factor);
  }
  return true;
}

int main(int argc, char *argv[]) {
  unsigned long base_width = 16000;
  std::vector<double> layers_factors{{ 4.0, 16.0, 64.0 }};
  std::string input_svg;
  std::string output_svs;

  if (argc < 3)
    return usage(argv[0], "Wrong number of positional arguments.");

  int opt;
  while ((opt = getopt_long(argc - 2, argv, "hb:l:",
                            long_options, nullptr)) != -1) {
    switch (opt) {
    case 'h':
      return usage(argv[0]);
    case 'b':
      char *pos;
      base_width = strtoul(optarg, &pos, 10);
      if (*pos != '\0' || base_width == 0)
        return usage(argv[0], "Invalid width.");
      break;
    case 'l':
      if (!parse_and_set_layers_factors(optarg, &layers_factors))
        return usage(argv[0], "Invalid factors.");
      break;
    case '?':
    case ':':
    default:
      return usage(argv[0], "");
    }
  }

  // Sort layers factors.
  std::sort(layers_factors.begin(), layers_factors.end());

  input_svg = std::string(argv[optind]);
  output_svs = std::string(argv[optind + 1]);

  if (VIPS_INIT(argv[0]))
    vips_error_exit(nullptr);

  // We interpret the whole svg canvas as 100.
  const unsigned kNumSubDivisions = 10 * 10;

  // Query the svg size
  const double default_resolution_width = VImage::svgload(
    input_svg.c_str(), VImage::option()->set("unlimited", true)).width();
  const double dpi = base_width * 72 / default_resolution_width;
  VImage in = VImage::svgload(
    input_svg.c_str(), VImage::option()
    ->set("dpi", dpi)
    ->set("unlimited", true));

  if (in.has_alpha())
    in = in.extract_band(0, VImage::option()->set("n", 3));

#if 0
  assert(in.interpretation() == VIPS_INTERPRETATION_sRGB);
  assert(in.bands() == 3);
  assert(in.width() == BASE_WIDTH);
#endif

  SvsMetadata svs_metadata = {};
  svs_metadata.mpp = 10.0 * kNumSubDivisions / base_width;
  svs_metadata.app_mag = 40;

  if (!vips2svs_encoder(in, output_svs.c_str(), layers_factors, svs_metadata))
    fprintf(stderr, "Error while generating svs pyramid file.\n");

  vips_shutdown();
  return 0;
}
