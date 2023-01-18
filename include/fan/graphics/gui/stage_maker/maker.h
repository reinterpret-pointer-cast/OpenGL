static void open_file_gui(const fan::string& path) {
#if defined(fan_platform_windows)
	// thanks microsoft
	fan::string dir_file_path = path;
	dir_file_path.replace_all("/", R"(\\)");

	ShellExecute(
		0,
		"open",
		dir_file_path.c_str(),
		0,
		0,
		SW_SHOWNORMAL
	);
#endif
}

struct stage_maker_t {

	#define use_key_lambda(key, state) \
		if (mb.mouse_stage != loco_t::vfi_t::mouse_stage_e::inside) { \
			return 0; \
		} \
		if (mb.button != key) { \
			return 0; \
					} \
		if (mb.button_state != state) { \
			return 0; \
		}

	static constexpr const char* stage_folder_name = "stages";
	static auto get_file_fullpath(const fan::string& stage_name) {
		return fan::string(stage_folder_name) + "/" +
			stage_name + ".h";
	};

	static constexpr const char* stage_instance_tempalte_str = R"(void open(auto* loco) {
  
}

void close(auto* loco){
		
}

void window_resize_callback(auto* loco){
		
}

void update(auto* loco){
	
}
)";

	static constexpr f32_t gui_size = 0.05;

	loco_t* get_loco() {
		return (loco_t*)((uint8_t*)OFFSETLESS(this, pile_t, stage_maker_var_name) + offsetof(pile_t, loco_var_name));
	}

	struct gui_store_t {
		uint16_t type;
		union shape_t {
			struct button_t{
				fan::vec2 position;
				fan::vec2 size;
				fan::string text;
				f32_t font_size;
				fan_2d::graphics::gui::theme_t theme;
			};
		}shape;
	};

	std::vector<gui_store_t> gui_store;

	struct stage_t {
		
		static constexpr uint8_t stage_options = 0;
		static constexpr uint8_t stage_instance = 1;

		struct stage_e {
			static constexpr uint8_t main = 0;
			static constexpr uint8_t gui = 1;
			static constexpr uint8_t function = 2;
		};

		stage_maker_t* get_stage_maker() {
			return OFFSETLESS(this, stage_maker_t, stage);
		}
	}stage;

	void push_stage_main(const loco_t::menu_maker_button_t::properties_t& p) {
		auto& loco = *get_loco();
		loco.menu_maker_button.push_back(stage.get_stage_maker()->instances[stage_t::stage_instance].menu_id, p);
	}
	void reopen_stage(uint8_t s) {
		auto loco = get_loco();

		auto nr = stage.get_stage_maker()->instances[s].menu_id;
		auto& instances = loco->menu_maker_button.instances[nr].base.instances;

		auto it = instances.GetNodeFirst();
		while (it != instances.dst) {
			if (loco->menu_maker_button.is_visually_valid(nr, it)) {
				it = it.Next(&instances);
				continue;
			}
			loco->menu_maker_button.push_initialized(nr, it);
			it = it.Next(&instances);
		}
	}
	void close_stage(uint8_t stage) {
		auto& loco = *get_loco();
		loco.menu_maker_button.erase_menu_soft(instances[stage].menu_id);
	}

	void open_stage_function() {

	}

	void reopen_main() {
		reopen_stage(stage_t::stage_options);
		reopen_stage(stage_t::stage_instance);
	}

	void open_stage(uint8_t stage_) {
		if (current_stage == stage_) {
			return;
		}
		current_stage = stage_;

		auto loco = get_loco();

		switch (current_stage) {
		case stage_t::stage_e::main: {

			loco_t::menu_maker_button_t::open_properties_t op;
			op.theme = &theme;
			op.matrices = &matrices;
			op.viewport = &viewport;
			op.gui_size = gui_size * 3;
			op.position = fan::vec2(op.gui_size * (5.0 / 3), -1.0 + op.gui_size);
			instances[stage_t::stage_instance].menu_id = loco->menu_maker_button.push_menu(op);
			break;
		}
		case stage_t::stage_e::gui: {
			in_gui_editor_id = loco->menu_maker_button.get_selected_id(instances[stage_t::stage_instance].menu_id);
			fgm.open_from_stage_maker(get_selected_name(
				OFFSETLESS(loco, pile_t, loco_var_name), 
				instances[stage_t::stage_instance].menu_id,
				in_gui_editor_id
			));
			break;
		}
		case stage_t::stage_e::function: {
			open_stage_function();
			break;
		}
		}
	}

	auto write_stage(pile_t* pile) {
		auto file_name = get_file_fullpath("stage");

		fan::io::file::write(file_name,
			stage_h_str,
			std::ios_base::binary
		);
	};

	auto append_stage_to_file(pile_t* pile, const fan::string& stage_name) {
		if (stage_h_str.find(stage_name) != fan::string::npos) {
			return;
		}

		static constexpr const char* find_end_str("\n};");
		auto struct_stage_end = stage_h_str.find_last_of(find_end_str);

		if (struct_stage_end == fan::string::npos) {
			fan::throw_error("corrupted stage.h");
		}

    auto append_struct = fmt::format(R"(
  struct {}_t : stage_common_t_t<{}_t> {{
    #include "{}"
  }};
  )", 
    stage_name.c_str(), 
    stage_name.c_str(),
    get_file_fullpath(stage_name).c_str());
		stage_h_str.insert(struct_stage_end, append_struct.c_str());

		//static constexpr fan::string_view find_vector("};\n};");
		//auto struct_vector_end = stage_h_str.find(find_vector);
		//if (struct_vector_end == fan::string::npos) {
		//	fan::throw_error("corrupted stage.h");
		//}

		//// problem: adds ',' to end
		//fan::string append_vector = fan::string("   &") + stage_name + ",\n  ";
		//stage_h_str.insert(struct_vector_end, append_vector);
	};

	fan::wstring get_selected_name(
		pile_t* pile, 
		loco_t::menu_maker_button_t::instance_NodeReference_t nr,
		loco_t::menu_maker_button_t::base_type_t::instance_NodeReference_t id
	) {
		auto t = pile->loco.menu_maker_button.instances[nr].base.instances[
			id
		].text;
		return t;
	}

	fan::wstring get_selected_name_last(pile_t* pile) {
		auto nr = pile->loco.menu_maker_button.instances.GetNodeLast();
		return get_selected_name(
			pile,
			nr,
			pile->loco.menu_maker_button.instances[nr].base.instances.GetNodeLast()
		);
	}

	auto write_stage_instance(pile_t* pile) {
		auto stage_name = get_selected_name_last(pile);
		auto file_name = get_file_fullpath(stage_name);
		fan::io::file::write(file_name, stage_instance_tempalte_str, std::ios_base::binary);

		append_stage_to_file(pile, stage_name);
		write_stage(pile);
	};

	void open_erase_button(pile_t* pile) {
		loco_t::menu_maker_button_t::properties_t p;
		p.text = L"Erase";
		p.theme = &erase_theme;
		p.mouse_button_cb = [this](const loco_t::mouse_button_data_t& mb) -> int {

			use_key_lambda(fan::mouse_left, fan::mouse_state::release);
			
			pile_t* pile = OFFSETLESS(OFFSETLESS(mb.vfi, loco_t, vfi), pile_t, loco_var_name);

			pile->loco.menu_maker_button.erase_and_update(
				instances[stage_t::stage_instance].menu_id,
				pile->loco.menu_maker_button.get_selected_id(instances[stage_t::stage_instance].menu_id)
			);
			pile->loco.menu_maker_button.erase_button_soft(instances[stage_t::stage_options].menu_id, erase_button_id);
			pile->loco.menu_maker_button.set_selected(instances[stage_t::stage_options].menu_id, nullptr);
			pile->loco.menu_maker_button.set_selected(instances[stage_t::stage_instance].menu_id, nullptr);

			return 1;
		};
		auto& current_y = pile->loco.menu_maker_button.get_offset(instances[stage_t::stage_options].menu_id).y;
		auto old_y = current_y;
		current_y = 1.9;
		erase_button_id = pile->loco.menu_maker_button.push_back(instances[stage_t::stage_options].menu_id, p);
		current_y = old_y;
	}

	void open_without_init() {
		auto* loco = get_loco();

		loco_t::menu_maker_button_t::open_properties_t op;
		op.theme = &theme;
		op.matrices = &matrices;
		op.viewport = &viewport;

		op.gui_size = gui_size;
		op.position = fan::vec2(-1.0 + op.gui_size * 5, -1.0 + op.gui_size * 1);
		instances[0].menu_id = loco->menu_maker_button.push_menu(op);

		loco_t::menu_maker_button_t::properties_t p;
		p.text = L"Create New Stage";
		p.mouse_button_cb = [this, loco](const loco_t::mouse_button_data_t& mb) -> int {

			use_key_lambda(fan::mouse_left, fan::mouse_state::release);

			pile_t* pile = OFFSETLESS(OFFSETLESS(mb.vfi, loco_t, vfi), pile_t, loco_var_name);
			pile->stage_maker.open_stage(stage_maker_t::stage_t::stage_e::main);
			instance_t* instance = &pile->stage_maker.instances[stage_maker_t::stage_t::stage_instance];

			static auto create_stage = [this, loco]() {
				loco_t::menu_maker_button_t::properties_t p;
				static uint32_t x = 0;
				p.text = fan::wstring(L"stage") + fan::to_wstring(x++);
				p.mouse_button_cb = [this](const loco_t::mouse_button_data_t& mb) -> int {

					use_key_lambda(fan::mouse_left, fan::mouse_state::release);

					pile_t* pile = OFFSETLESS(OFFSETLESS(mb.vfi, loco_t, vfi), pile_t, loco_var_name);
					fan::graphics::cid_t* cid = mb.cid;
					if (mb.mouse_stage == loco_t::vfi_t::mouse_stage_e::inside) {
						if (pile->loco.menu_maker_button.is_visually_valid(
							instances[stage_t::stage_options].menu_id,
							pile->stage_maker.erase_button_id
						)) {
							return 0;
						}
						pile->loco.menu_maker_button.push_initialized(
							instances[stage_t::stage_options].menu_id,
							pile->stage_maker.erase_button_id
						);
					}
					return 0;
				};
				pile_t* pile = OFFSETLESS(loco, pile_t, loco_var_name);
				pile->stage_maker.push_stage_main(p);
			};

			create_stage();
			write_stage_instance(pile);
			return 0;
		};

		loco->menu_maker_button.push_back(instances[stage_t::stage_options].menu_id, p);
		p.text = L"Gui stage";
		p.mouse_button_cb = [](const loco_t::mouse_button_data_t& mb) {

			use_key_lambda(fan::mouse_left, fan::mouse_state::release);

			pile_t* pile = OFFSETLESS(OFFSETLESS(mb.vfi, loco_t, vfi), pile_t, loco_var_name);
			auto nr = pile->stage_maker.instances[stage_t::stage_instance].menu_id;
			auto id = pile->loco.menu_maker_button.get_selected_id(nr);
			if (pile->loco.menu_maker_button.instances[nr].base.instances.inri(id)) {
				return 0;
			}

			pile->stage_maker.close_stage(stage_t::stage_options);
			pile->stage_maker.close_stage(stage_t::stage_instance);
			pile->stage_maker.open_stage(stage_t::stage_e::gui);

			return 1;
		};
		loco->menu_maker_button.push_back(instances[stage_t::stage_options].menu_id, p);
		p.text = L"Function stage";

		p.mouse_button_cb = [](const loco_t::mouse_button_data_t& mb) -> int {

			use_key_lambda(fan::mouse_left, fan::mouse_state::release);

			pile_t* pile = OFFSETLESS(OFFSETLESS(mb.vfi, loco_t, vfi), pile_t, loco_var_name);

			open_file_gui(
				get_file_fullpath(
					pile->loco.menu_maker_button.get_selected_text(pile->stage_maker.instances[stage_maker_t::stage_t::stage_instance].menu_id)
				)
			);
			return 0;
		};
		loco->menu_maker_button.push_back(instances[stage_t::stage_options].menu_id, p);
	}

	void open(const char* texturepack_name) {
		
  stage_h_str = R"(protected:
  #define BLL_set_CPP_ConstructDestruct
  #define BLL_set_CPP_Node_ConstructDestruct
  #define BLL_set_BaseLibrary 1
  #define BLL_set_prefix stage_list
  #define BLL_set_type_node uint16_t
  #define BLL_set_NodeDataType void *
  #define BLL_set_Link 1
  #define BLL_set_AreWeInsideStruct 1
  #include _FAN_PATH(BLL/BLL.h)
public:

using nr_t = stage_list_NodeReference_t;

struct stage_open_properties_t {
	loco_t::matrices_list_NodeReference_t matrices;
	fan::graphics::viewport_list_NodeReference_t viewport;
	fan::graphics::theme_list_NodeReference_t theme;
};

template <typename T = __empty_struct>
struct stage_common_t_t {

  stage_common_t_t(auto* loader, auto* loco, const stage_open_properties_t& properties) {
    T* stage = (T*)this;
    loader->load_fgm(loco, (T*)this, properties, stage->stage_name);
    stage->open(loco);
  }
  void close(auto* loco) {
    T* stage = (T*)this;
    stage->close(loco);
  }

  nr_t stage_id;

protected:
  #define BLL_set_CPP_ConstructDestruct
  #define BLL_set_CPP_Node_ConstructDestruct
  #define BLL_set_BaseLibrary 1
  #define BLL_set_prefix cid_list
  #define BLL_set_type_node uint16_t
  #define BLL_set_NodeDataType fan::graphics::cid_t
  #define BLL_set_Link 1
  #define BLL_set_StoreFormat 1
  #define BLL_set_AreWeInsideStruct 1
  #define BLL_set_StoreFormat1_ElementPerBlock 0x100
  #include _FAN_PATH(BLL/BLL.h)
public:

  cid_list_t cid_list;
};

using stage_common_t = stage_common_t_t<>;

struct stage {
  inline static stage_list_t stage_list;
};

)";

		auto loco = get_loco();

		fgm.open(texturepack_name);
		instances.resize(3);

		current_stage = fan::uninitialized;

		theme = fan_2d::graphics::gui::themes::gray();
		theme.open(loco->get_context());

		erase_theme = fan_2d::graphics::gui::themes::deep_red();
		erase_theme.open(loco->get_context());

		fan::vec2 window_size = loco->get_window()->get_size();
		fan::vec2 ratio = window_size / window_size.max();
		loco->open_matrices(
			&matrices,
			fan::vec2(-1, 1) * ratio.x,
			fan::vec2(-1, 1) * ratio.y
		);

		pile_t* pile = OFFSETLESS(loco, pile_t, loco_var_name);

		viewport.open(loco->get_context());
		viewport.set(loco->get_context(), 0, window_size, window_size);

		open_without_init();
		open_stage(stage_t::stage_e::main);
		open_erase_button(pile);
		pile->loco.menu_maker_button.erase_button_soft(instances[stage_t::stage_options].menu_id, erase_button_id);
	}
	void close() {
		auto& loco = *get_loco();
		theme.close(loco.get_context());
	}

	void reopen_from_fgm() {
		auto loco = get_loco();
		//open_without_init();
		reopen_main();
		loco->menu_maker_button.set_selected(
			instances[stage_t::stage_instance].menu_id,
			in_gui_editor_id
		);
		current_stage = stage_t::stage_e::main;
		//open_erase_button(OFFSETLESS(loco, pile_t, loco_var_name));
	}

	struct instance_t {
		loco_t::menu_maker_button_t::nr_t menu_id;
	};
	std::vector<instance_t> instances;

	uint8_t current_stage;

	loco_t::matrices_t matrices;
	fan::graphics::viewport_t viewport;
	fan_2d::graphics::gui::theme_t theme;
	fan_2d::graphics::gui::theme_t erase_theme;
	fan::string stage_h_str;

	loco_t::menu_maker_button_t::base_type_t::instance_NodeReference_t in_gui_editor_id;
	loco_t::menu_maker_button_t::base_type_t::instance_NodeReference_t erase_button_id;

	#include "fgm.h"
};

#undef use_key_lambda