#pragma once

struct format {

  struct shape_type_t {
    using _t = uint16_t;
    static constexpr _t button = 0;
    static constexpr _t sprite = 1;
  };

  struct shape_button_t {
    fan::vec3 position;
    fan::vec2 size;
    f32_t font_size;
    fan_2d::graphics::gui::theme_t theme;
  };
  struct shape_sprite_t {
    fan::vec3 position;
    fan::vec2 size;
    uint64_t hash_path;
  };
};