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
#ifndef __APERIO_SVS_ENCODING_H_
#define __APERIO_SVS_ENCODING_H_
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <vips/vips.h>
#include <vips/vips8>

// Set of supported svs Aperio metadata.
typedef struct {
  std::optional<double> mpp;  // microns per pixel.
  std::optional<int> app_mag;  // apparent magnification.
} SvsMetadata;

// Encodes a generic vips in .svs format.
// libvips already supports pyramidal formats, but
// is impossible to customize its behavior.
// `in` is a vips RGB image (the svs encoding will be RGB.)
// `svs_out_filepath` is the output file location.
// `scalings` is a vector of downsample magnitudes. For instance,
// for a value of 4.0, the resulting image width and height
// will be multiplied by 1 / 4.0 to compute the resulting layer size.
// `metadata` are any additional supported metadata that you want to include.
bool vips2svs_encoder(const vips::VImage &in, const char *svs_out_filepath,
                      const std::vector<double> &scalings,
                      SvsMetadata svs_metadata);
#endif // __APERIO_SVS_ENCODING_H_
