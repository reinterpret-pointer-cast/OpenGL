// Creates window, opengl context and renders a rectangle

#define _INCLUDE_TOKEN(p0, p1) <p0/p1>

#define FAN_INCLUDE_PATH C:/libs/fan/include
#define fan_debug 0
#include _INCLUDE_TOKEN(FAN_INCLUDE_PATH, fan/types/types.h)

#include _FAN_PATH(graphics/graphics.h)

#define loco_window
#define loco_context

#define loco_yuv420p
#include _FAN_PATH(graphics/loco.h)

constexpr uint32_t count = 1;

struct pile_t {

  static constexpr fan::vec2 ortho_x = fan::vec2(-1, 1);
  static constexpr fan::vec2 ortho_y = fan::vec2(-1, 1);

  void open() {
    loco.open(loco_t::properties_t());
    fan::graphics::open_matrices(
      loco.get_context(),
      &matrices,
      loco.get_window()->get_size(),
      ortho_x,
      ortho_y
    );
    loco.get_window()->add_resize_callback(this, [](fan::window_t*, const fan::vec2i& size, void* userptr) {
      pile_t* pile = (pile_t*)userptr;

      pile->viewport.set_viewport(pile->loco.get_context(), 0, size);
      });
    viewport.open(loco.get_context(), 0, loco.get_window()->get_size());
  }

  loco_t loco;
  fan::opengl::matrices_t matrices;
  fan::opengl::viewport_t viewport;
  fan::opengl::cid_t cids[count];
};

int main() {

  pile_t* pile = new pile_t;
  pile->open();

  loco_t::yuv420p_t::properties_t p;

  p.size = fan::vec2(1, 1);
  //p.block_properties.
  p.matrices = &pile->matrices;
  p.viewport = &pile->viewport;

  constexpr fan::vec2ui image_size = fan::vec2ui(1920, 1050);

  std::string str;
  fan::io::file::read("output.yuv", &str);

  p.load_yuv(pile->loco.get_context(), (uint8_t*)str.data(), image_size);

  p.position = fan::vec2(0, 0);
  p.position.z = 0;
  pile->loco.yuv420p.push_back(&pile->cids[0], p);

  //fan::print(y);
  pile->loco.set_vsync(false);
  while(pile->loco.window_open(pile->loco.process_frame([]{}))) {

    pile->loco.get_fps();
  }

  return 0;
}