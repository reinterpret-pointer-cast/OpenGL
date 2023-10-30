#include fan_pch

#define stage_loader_path .
#include _FAN_PATH(graphics/gui/stage_maker/loader.h)

// make custom stage
lstd_defstruct(custom_t)
#include _FAN_PATH(graphics/gui/stage_maker/preset.h)

static constexpr auto stage_name = "";
fan::graphics::rectangle_t r;
void open(void* sod) {
  r = {{
    .position = 400,
    .size = 100,
    .color = fan::colors::red
  }};
}

void close() {

}

void window_resize() {

}

void update() {

}

};

int main() {
  loco_t loco;

  fan::vec2 window_size = loco.get_window()->get_size();
  loco.default_camera->camera.set_ortho(
    fan::vec2(0, window_size.x),
    fan::vec2(0, window_size.y)
  );

  stage_loader_t stage_loader;

  loco_t::texturepack_t tp;
  tp.open_compiled("TexturePack");
  stage_loader.open(&tp);

  stage_loader_t::stage_open_properties_t op;

  stage_loader_t::nr_t it_custom = stage_loader_t::open_stage<custom_t>(op);
  stage_loader_t::nr_t it_stage0 = stage_loader_t::open_stage<stage_loader_t::stage::stage0_t>(op);

  loco.loop([] {

  });
}