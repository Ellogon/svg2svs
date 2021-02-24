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
#ifndef __UTILS_H_
#define __UTILS_H_
#include <memory>
#include <iostream>
#include <optional>
#include <type_traits>
#include <vips/vips.h>
#include <vips/vips8>

typedef struct {
  std::unique_ptr<uint8_t[]> data;
  size_t size;
} Buffer;

template<typename T>
struct GDeleter {
  void operator()(T* ptr) const { g_object_unref(ptr); }
};

template<class T>
using UniqueVipsPointer = std::unique_ptr<T, GDeleter<T>>;

typedef UniqueVipsPointer<VipsRegion> UniqueVipsRegion;

template<typename T>
class Generator {
public:
  Generator() {}
  virtual std::optional<T> Next(const std::optional<T> &prev, bool *end) const = 0;

  class const_iterator {
  public:
    const_iterator(const Generator &container, bool end = false)
      : container_(container),
        end_(end),
        value_((!end_) ? container.Next({}, &end_) : std::optional<T>({}))
        {}

    const std::optional<T> &operator* () const { return value_; }

    void operator++ () {
      if (!end_)
        value_ = container_.Next(value_, &end_);
    }

    bool operator!= (const const_iterator& other) {
      return other.IsEnd() != end_;
    }

  private:
    const Generator<T> &container_;
    bool end_;
    std::optional<T> value_;
    bool IsEnd() const { return end_; }
  };

  const_iterator begin() const { return const_iterator(*this); }
  const_iterator end() const { return const_iterator(*this, true); }
};

inline unsigned partition(const unsigned total_length, const unsigned part) {
  return std::floor((total_length + part - 1) / part);
}
#endif // __UTILS_H_
