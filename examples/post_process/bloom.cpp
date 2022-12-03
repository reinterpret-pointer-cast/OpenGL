// Creates window, opengl context and renders a rectangle

#define _INCLUDE_TOKEN(p0, p1) <p0/p1>

#define FAN_INCLUDE_PATH C:/libs/fan/include
#define fan_debug 0
#include _INCLUDE_TOKEN(FAN_INCLUDE_PATH, fan/types/types.h)

#define loco_window
#define loco_context

#define loco_vulkan


#define loco_sprite
#define loco_post_process
#include _FAN_PATH(graphics/loco.h)

constexpr uint32_t count = 1;

struct pile_t {

	static constexpr fan::vec2 ortho_x = fan::vec2(-1, 1);
	static constexpr fan::vec2 ortho_y = fan::vec2(-1, 1);

	void open() {
		loco.open_matrices(
			&matrices,
			ortho_x,
			ortho_y
		);
		/*  loco.get_window()->add_resize_callback(this, [](fan::window_t* window, const fan::vec2i& size, void* userptr) {
				fan::vec2 window_size = window->get_size();
				fan::vec2 ratio = window_size / window_size.max();
				std::swap(ratio.x, ratio.y);
				pile_t* pile = (pile_t*)userptr;
				pile->matrices.set_ortho(
					ortho_x * ratio.x,
					ortho_y * ratio.y,
					1
				);
			});
			loco.get_window()->add_resize_callback(this, [](fan::window_t*, const fan::vec2i& size, void* userptr) {
				pile_t* pile = (pile_t*)userptr;

				pile->viewport.set_viewport(pile->loco.get_context(), 0, size, pile->loco.get_window()->get_size());
			});*/
		viewport.open(loco.get_context());
		viewport.set(loco.get_context(), 0, loco.get_window()->get_size(), loco.get_window()->get_size());
	}

	loco_t loco;
	loco_t::matrices_t matrices;
	fan::graphics::viewport_t viewport;
	fan::graphics::cid_t cids[count];
};

int main() {

	pile_t* pile = new pile_t;
	pile->open();

	loco_t::sprite_t::properties_t p;

	//p.block_properties.
	p.matrices = &pile->matrices;
	p.viewport = &pile->viewport;

	loco_t::image_t image;
	loco_t::image_t::load_properties_t lp;
	lp.filter = fan::opengl::GL_LINEAR;
	image.load(&pile->loco, "images/planet.webp");
	p.image = &image;
	p.size = fan::cast<f32_t>(image.size) / pile->loco.get_window()->get_size();
	p.position = 0;
	p.position.z = 0;
	pile->loco.sprite.push_back(&pile->cids[0], p);

	//pile->loco.post_process.push(&pile->viewport, &pile->matrices);

	pile->loco.set_vsync(false);

	pile->loco.get_window()->add_buttons_callback([&](const fan::window_t::mouse_buttons_cb_data_t& d) {

		if (d.state != fan::mouse_state::press) {
			return;
		}

		switch (d.button) {
		case fan::mouse_scroll_up: {
			pile->loco.post_process.bloomamount += 0.01;
			break;
		}
		case fan::mouse_scroll_down: {
			pile->loco.post_process.bloomamount -= 0.01;
			break;
		}
		}
	});

	pile->loco.loop([&] {
		pile->loco.get_fps();
	});

	return 0;
}