// Creates window, opengl context and renders a rectangle

#define _INCLUDE_TOKEN(p0, p1) <p0/p1>

#ifndef FAN_INCLUDE_PATH
#define FAN_INCLUDE_PATH C:/libs/fan/include
#endif
#define fan_debug 0
#include _INCLUDE_TOKEN(FAN_INCLUDE_PATH, fan/types/types.h)

//#define loco_vulkan

#define loco_window
#define loco_context

//#define loco_rectangle
#define loco_sprite_sheet
#include _FAN_PATH(graphics/loco.h)

struct pile_t {

  static constexpr fan::vec2 ortho_x = fan::vec2(-1, 1);
  static constexpr fan::vec2 ortho_y = fan::vec2(-1, 1);

  pile_t() {
    fan::vec2 window_size = loco.get_window()->get_size();
    loco.open_matrices(
      &matrices,
      ortho_x,
      ortho_y
    );
    loco.get_window()->add_resize_callback([&](const fan::window_t::resize_cb_data_t& d) {
      fan::vec2 window_size = d.size;
    fan::vec2 ratio = window_size / window_size.max();
    std::swap(ratio.x, ratio.y);
    //matrices.set_ortho(
    //  ortho_x * ratio.x, 
    //  ortho_y * ratio.y
    //);
    viewport.set(loco.get_context(), 0, d.size, d.size);
      });
    viewport.open(loco.get_context());
    viewport.set(loco.get_context(), 0, window_size, window_size);
  }

  loco_t loco;
  loco_t::matrices_t matrices;
  fan::graphics::viewport_t viewport;
  fan::graphics::cid_t cid[(unsigned long long)1];
};

int main() {

  fan::time::clock c;
  c.start();

  pile_t* pile = new pile_t;

  loco_t::sprite_sheet_t::properties_t p;

  p.position = fan::vec2(0, 0);
  p.size = fan::vec2(0.13, 0.08375) * 2;
  p.matrices = &pile->matrices;
  p.viewport = &pile->viewport;

  loco_t::image_t textures[6];
  textures[0].load(&pile->loco, "frames/frame0.webp");
  textures[1].load(&pile->loco, "frames/frame1.webp");
  textures[2].load(&pile->loco, "frames/frame2.webp");
  textures[3].load(&pile->loco, "frames/frame3.webp");
  textures[4].load(&pile->loco, "frames/frame4.webp");
  textures[5].load(&pile->loco, "frames/frame5.webp");

  loco_t::sheet_t sheet;
  for (uint32_t i = 0; i < std::size(textures); ++i) {
    sheet.push_back(&textures[i]);
  }
  sheet.animation_speed = 1e+8;
  p.sheet = &sheet;
  p.position = fan::random::vec2(0, 0);
  auto nr = pile->loco.sprite_sheet.push_back(p);

  pile->loco.sprite_sheet.start(nr);
  //Sleep(300);
  //pile->loco.sprite_sheet.start(nr2);
  //Sleep(10);
  //pile->loco.sprite_sheet.start(nr3);
  //Sleep(10);
  //pile->loco.sprite_sheet.start(nr4);
  //pile->loco.set_vsync(false);

  pile->loco.loop([&] {
    pile->loco.get_fps();
  });

  return 0;
}