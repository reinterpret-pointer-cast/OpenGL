#include fan_pch

#include _FAN_PATH(graphics/gui/tilemap_editor/loader.h)

struct player_t {
  static constexpr fan::vec2 speed{200, 200};

  player_t() {
    visual = fan::graphics::sprite_t{{
      .position = fan::vec3(0, 100, 10),
      .size = 32,
      .blending = true
    }};
  }
  void update() {
    f32_t dt = gloco->get_delta_time();
    if (gloco->window.key_pressed(fan::key_d)) {
      velocity.x = speed.x;
    }
    else if (gloco->window.key_pressed(fan::key_a)) {
      velocity.x = -speed.x;
    }
    else {
      velocity.x = 0;
    }

    if (gloco->window.key_pressed(fan::key_w)) {
      velocity.y = -speed.y;
    }
    else if (gloco->window.key_pressed(fan::key_s)) {
      velocity.y = speed.y;
    }
    else {
      velocity.y = 0;
    }

    visual.set_velocity(velocity);
    visual.set_position(visual.get_collider_position());
  }
  fan::vec2 velocity = 0;
  fan::graphics::collider_dynamic_t visual;
};

f32_t zoom = 2;
void init_zoom() {
  auto& window = *gloco->get_window();
  auto update_ortho = []{
    fan::vec2 s = gloco->window.get_size();
    gloco->default_camera->camera.set_ortho(
      fan::vec2(-s.x, s.x) / zoom,
      fan::vec2(-s.y, s.y) / zoom
    );;
  };

  update_ortho();

  window.add_buttons_callback([&](const auto&d) {
    if (d.button == fan::mouse_scroll_up) {
      zoom *= 1.2;
    }
    else if (d.button == fan::mouse_scroll_down) {
      zoom /= 1.2;
    }
    update_ortho();
  });
}

int main() {
  loco_t loco;
  loco_t::texturepack_t tp;
  tp.open_compiled("texture_packs/tilemap.ftp");

  fte_loader_t loader;
  loader.open(&tp);

  auto compiled_map = loader.compile("tilemaps/collision_test.fte");

  fte_loader_t::properties_t p;

  p.position = fan::vec3(0, 0, 0);
  auto map_id0_t = loader.add(&compiled_map, p);

  init_zoom();

  player_t player;


  loco.loop([&] {
    gloco->get_fps();
    player.update();
    gloco->default_camera->camera.set_position(player.visual.get_position());
  });
}