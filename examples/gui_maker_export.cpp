#define _INCLUDE_TOKEN(p0, p1) <p0/p1>

#define FAN_INCLUDE_PATH C:/libs/fan/include
#define fan_debug 1
#include _INCLUDE_TOKEN(FAN_INCLUDE_PATH, fan/types/types.h)

#define fan_windows_subsystem fan_windows_subsystem_windows

#include _FAN_PATH(graphics/graphics.h)

#define loco_window
#define loco_context

#define loco_line
#define loco_button
#define loco_var_name loco
#include _FAN_PATH(graphics/loco.h)

struct pile_t {

  void open() {
    loco.open(loco_t::properties_t());
    fgm.open();
    /*loco.get_window()->add_resize_callback(this, [](fan::window_t* window, const fan::vec2i& size, void* userptr) {
      fan::vec2 window_size = window->get_size();
      fan::vec2 ratio = window_size / window_size.max();
      std::swap(ratio.x, ratio.y);
      pile_t* pile = (pile_t*)userptr;
      pile->matrices.set_ortho(
        ortho_x * ratio.x, 
        ortho_y * ratio.y
      );
    });*/
  }

  loco_t loco_var_name;
  #define fgm_var_name fgm
  #include _FAN_PATH(graphics/gui/fgm/fgm.h)
  fgm_t fgm_var_name;
};

int main() {
  pile_t* pile = new pile_t;

  pile->open();

  //pile->loco.set_vsync(false);
  //pile->loco.get_window()->set_max_fps(5);

  while(pile->loco.window_open(pile->loco.process_frame([]{}))) {
    pile->loco.get_fps();
  }

  return 0;
}