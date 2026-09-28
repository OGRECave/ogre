/* Copyright 2026 Google LLC
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
      http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/*
 * Exercise Ogre's ASTC, PKM, KTX, and PVR image decoders. image_fuzz pins the
 * extension to PNG, so these registered codecs need explicit dispatch.
 */

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "OgreDataStream.h"
#include "OgreException.h"
#include "OgreImage.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

namespace {

constexpr size_t kMaxInputSize = 1U << 20;
constexpr uint32_t kMaxDimension = 4096;
Ogre::Root *g_root = nullptr;

uint32_t read_le32(const uint8_t *data) {
  return static_cast<uint32_t>(data[0]) |
         (static_cast<uint32_t>(data[1]) << 8) |
         (static_cast<uint32_t>(data[2]) << 16) |
         (static_cast<uint32_t>(data[3]) << 24);
}

uint32_t read_le24(const uint8_t *data) {
  return static_cast<uint32_t>(data[0]) |
         (static_cast<uint32_t>(data[1]) << 8) |
         (static_cast<uint32_t>(data[2]) << 16);
}

bool dimensions_are_bounded(uint32_t width, uint32_t height, uint32_t depth) {
  return width > 0 && height > 0 && depth > 0 && width <= kMaxDimension &&
         height <= kMaxDimension && depth <= 256 &&
         static_cast<uint64_t>(width) * height * depth <= (1U << 22);
}

bool image_set_is_bounded(uint32_t width, uint32_t height, uint32_t depth,
           uint32_t faces, uint32_t surfaces,
           uint32_t mip_levels) {
  const uint64_t texels = static_cast<uint64_t>(width) * height * depth *
           faces * surfaces * mip_levels;
  // Ogre::Image supports only 1 or 6 faces; PVR decoding does not handle arrays.
  return (faces == 1 || faces == 6) && surfaces == 1 &&
    mip_levels > 0 && mip_levels <= 16 && texels <= (1U << 22);
}

bool plausible_astc(const uint8_t *data, size_t size) {
  if (size < 16 || read_le32(data) != 0x5ca1ab13)
    return false;

  const uint8_t block_x = data[4];
  const uint8_t block_y = data[5];
  const uint8_t block_z = data[6];
  const bool valid_2d_block =
      (block_x == 4 && block_y == 4) ||
      (block_x == 5 && (block_y == 4 || block_y == 5)) ||
      (block_x == 6 && (block_y == 5 || block_y == 6)) ||
      (block_x == 8 && (block_y == 5 || block_y == 6 || block_y == 8)) ||
      (block_x == 10 && (block_y == 5 || block_y == 6 || block_y == 8 ||
                         block_y == 10)) ||
      (block_x == 12 && (block_y == 10 || block_y == 12));
  const uint32_t width = read_le24(data + 7);
  const uint32_t height = read_le24(data + 10);
  const uint32_t depth = read_le24(data + 13);

  return valid_2d_block && block_z != 0 && block_z <= 6 &&
         dimensions_are_bounded(width, height, depth);
}

bool plausible_pkm(const uint8_t *data, size_t size) {
  if (size < 16 || memcmp(data, "PKM ", 4) != 0)
    return false;

  const uint32_t width = (static_cast<uint32_t>(data[12]) << 8) | data[13];
  const uint32_t height = (static_cast<uint32_t>(data[14]) << 8) | data[15];
  return dimensions_are_bounded(width, height, 1);
}

bool plausible_ktx(const uint8_t *data, size_t size) {
  static constexpr uint8_t kKtxId[12] = {
      0xab, 0x4b, 0x54, 0x58, 0x20, 0x31,
      0x31, 0xbb, 0x0d, 0x0a, 0x1a, 0x0a};
  if (size < 64 || memcmp(data, kKtxId, sizeof(kKtxId)) != 0)
    return false;

  const uint32_t width = read_le32(data + 36);
  const uint32_t height = read_le32(data + 40);
  const uint32_t depth = std::max(1U, read_le32(data + 44));
  const uint32_t faces = read_le32(data + 52);
  const uint32_t mip_levels = std::max(1U, read_le32(data + 56));
  const uint32_t metadata_size = read_le32(data + 60);

    return dimensions_are_bounded(width, height, depth) &&
      image_set_is_bounded(width, height, depth, faces, 1, mip_levels) &&
      metadata_size <= size - 64;
}

bool plausible_pvr(const uint8_t *data, size_t size) {
  if (size < 52)
    return false;

  if (read_le32(data + 44) == 0x21525650) {
    const uint32_t width = read_le32(data + 8);
    const uint32_t height = read_le32(data + 4);
        const uint32_t mip_count = read_le32(data + 12);
        return dimensions_are_bounded(width, height, 1) && mip_count < 16 &&
          image_set_is_bounded(width, height, 1, 1, 1, mip_count + 1);
  }

  if (read_le32(data) == 0x03525650) {
    const uint32_t height = read_le32(data + 24);
    const uint32_t width = read_le32(data + 28);
    const uint32_t depth = std::max(1U, read_le32(data + 32));
    const uint32_t surfaces = read_le32(data + 36);
    const uint32_t faces = read_le32(data + 40);
    const uint32_t mip_levels = read_le32(data + 44);
    const uint32_t metadata_size = read_le32(data + 48);
        return dimensions_are_bounded(width, height, depth) &&
          image_set_is_bounded(width, height, depth, faces, surfaces,
                mip_levels) &&
          metadata_size <= size - 52;
  }

  return false;
}

void initialize() {
  if (g_root)
    return;

  auto *log_manager = new Ogre::LogManager();
  log_manager->createLog("texture_codec_fuzz.log", true, false, true);
  log_manager->setMinLogLevel(Ogre::LML_CRITICAL);
  g_root = new Ogre::Root("", "", "");
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size < 2 || size > kMaxInputSize)
    return 0;

  initialize();
  const uint8_t *image_data = data + 1;
  const size_t image_size = size - 1;
  const char *extension = nullptr;
  bool plausible = false;

  switch (data[0] % 4) {
  case 0:
    extension = "astc";
    plausible = plausible_astc(image_data, image_size);
    break;
  case 1:
    extension = "pkm";
    plausible = plausible_pkm(image_data, image_size);
    break;
  case 2:
    extension = "ktx";
    plausible = plausible_ktx(image_data, image_size);
    break;
  default:
    extension = "pvr";
    plausible = plausible_pvr(image_data, image_size);
    break;
  }

  if (!plausible)
    return 0;

  try {
    Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream(
        const_cast<uint8_t *>(image_data), image_size, false, true));
    Ogre::Image image;
    image.load(stream, extension);
    if (image.getData() && image.getSize()) {
      volatile uint8_t sink = image.getData()[0];
      (void)sink;
    }
  } catch (const Ogre::Exception &) {
  }

  return 0;
}