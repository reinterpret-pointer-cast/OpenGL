#pragma once

#include <fan/graphics/webp.h>
#include <fan/graphics/stb.h>

namespace fan {
  namespace image {

    struct image_info_t {
      void* data;
      fan::vec2i size;
      int channels;
      uint8_t type; // webp, stb
    };

    static bool load(const std::string& file, image_info_t* image_info) {
      bool ret;
      if (fan::webp::validate_webp(file)) {
        ret = fan::webp::load(file, (fan::webp::image_info_t*)image_info);
        image_info->type = 0;
      }
      else {
        ret = fan::stb::load(file, (fan::stb::image_info_t*)image_info);
        image_info->type = 1;
      }
#if fan_debug >= fan_debug_low
      if (ret) {
        fan::print_warning("failed to load image data from path:" + file);
      }
#endif
      return ret;
    }
    static void free(image_info_t* image_info) {
      if (image_info->type == 0) { // webp
        fan::webp::free_image(image_info->data);
      }
      else if (image_info->type == 1) { // webp
        fan::stb::free_image(image_info->data);
      }
    }

  }
}