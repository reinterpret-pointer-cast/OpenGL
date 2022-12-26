struct button_t {

  static constexpr f32_t inactive = 1.0;
  static constexpr f32_t hover = 1.2;
  static constexpr f32_t press = 1.4;

  struct vi_t {
    fan::vec3 position = 0;
    f32_t angle = 0;
    fan::vec2 size = 0;
    fan::vec2 rotation_point = 0;
    fan::color color = fan::colors::white;
    fan::color outline_color;
    fan::vec3 rotation_vector = fan::vec3(0, 0, 1);
    f32_t outline_size;
  };

  static constexpr uint32_t max_instance_size = fan::min(256, 4096 / (sizeof(vi_t) / 4));

  struct bm_properties_t {
    using parsed_masterpiece_t = fan::masterpiece_t<
      loco_t::matrices_list_NodeReference_t,
      fan::graphics::viewport_list_NodeReference_t
    >;

    struct key_t : parsed_masterpiece_t {}key;
  };

  struct cid_t;

  struct ri_t : bm_properties_t {
    cid_t* cid;
    uint8_t selected = 0;
    fan::graphics::theme_list_NodeReference_t theme;
    uint32_t text_id;
    loco_t::vfi_t::shape_id_t vfi_id;
    uint64_t udata;
  };

  #define make_key_value(type, name) \
    type& name = *key.get_value<decltype(key)::get_index_with_type<type>()>();

  struct properties_t : vi_t, ri_t {

    make_key_value(loco_t::matrices_list_NodeReference_t, matrices);
    make_key_value(fan::graphics::viewport_list_NodeReference_t, viewport);

    fan::wstring text;
    f32_t font_size = 0.1;

    loco_t::vfi_t::iflags_t vfi_flags;

    bool disable_highlight = false;

    loco_t::mouse_button_cb_t mouse_button_cb = [](const loco_t::mouse_button_data_t&) -> int { return 0; };
    loco_t::mouse_move_cb_t mouse_move_cb = [](const loco_t::mouse_move_data_t&) -> int { return 0; };
    loco_t::keyboard_cb_t keyboard_cb = [](const loco_t::keyboard_data_t&) -> int { return 0; };

    properties_t() = default;
    properties_t(const vi_t& i) : vi_t(i) {}
    properties_t(const ri_t& p) : ri_t(p) {}
  };

  #undef make_key_value

  void push_back(fan::graphics::cid_t* cid, properties_t& p) {
    loco_t* loco = get_loco();

    #if defined(loco_vulkan)
      auto& matrices = loco->matrices_list[p.matrices];
      if (matrices.matrices_index.button == (decltype(matrices.matrices_index.button))-1) {
        matrices.matrices_index.button = m_matrices_index++;
        m_shader.set_matrices(loco, matrices.matrices_id, matrices.matrices_index.button);
      }
    #endif

    auto theme = loco->button.get_theme(p.theme);
    loco_t::text_t::properties_t tp;
    tp.color = theme->button.text_color;
    tp.font_size = p.font_size;
    tp.position = p.position;
    tp.text = p.text;
    tp.position.z += p.position.z + 1;
    tp.viewport = p.viewport;
    tp.matrices = p.matrices;

    sb_push_back(cid, p);

    sb_get_ri(cid).text_id = loco->text.push_back(tp);

    set_theme(cid, theme, inactive);

    loco_t::vfi_t::properties_t vfip;
    vfip.shape_type = loco_t::vfi_t::shape_t::rectangle;
    vfip.shape.rectangle.matrices = p.matrices;
    vfip.shape.rectangle.viewport = p.viewport;
    vfip.shape.rectangle.position = p.position;
    vfip.shape.rectangle.size = p.size;
    vfip.flags = p.vfi_flags;
    if (!p.disable_highlight) {
      vfip.mouse_move_cb = [this, cb = p.mouse_move_cb, udata = p.udata, cid_ = cid](const loco_t::vfi_t::mouse_move_data_t& mm_d) -> int {
        loco_t* loco = OFFSETLESS(mm_d.vfi, loco_t, vfi_var_name);
        loco_t::mouse_move_data_t mmd = mm_d;
        if (mm_d.flag->ignore_move_focus_check == false && !loco->button.sb_get_ri(cid_).selected) {
          if (mm_d.mouse_stage == loco_t::vfi_t::mouse_stage_e::inside) {
            loco->button.set_theme(cid_, loco->button.get_theme(cid_), hover);
          }
          else {
            loco->button.set_theme(cid_, loco->button.get_theme(cid_), inactive);
          }
        }
        mmd.cid = cid_;
        cb(mmd);
        return 0;
      };
      vfip.mouse_button_cb = [this, cb = p.mouse_button_cb, udata = p.udata, cid_ = cid](const loco_t::vfi_t::mouse_button_data_t& ii_d) -> int {
        loco_t* loco = OFFSETLESS(ii_d.vfi, loco_t, vfi_var_name);
        if (ii_d.flag->ignore_move_focus_check == false && !loco->button.sb_get_ri(cid_).selected) {
          if (ii_d.button == fan::button_left && ii_d.button_state == fan::button_state::press) {
            loco->button.set_theme(cid_, loco->button.get_theme(cid_), press);
            ii_d.flag->ignore_move_focus_check = true;
            loco->vfi.set_focus_keyboard(loco->vfi.get_focus_mouse());
          }
        }
        else if (!loco->button.sb_get_ri(cid_).selected) {
          if (ii_d.button == fan::button_left && ii_d.button_state == fan::button_state::release) {
            if (ii_d.mouse_stage == loco_t::vfi_t::mouse_stage_e::inside) {
              loco->button.set_theme(cid_, loco->button.get_theme(cid_), hover);
            }
            else {
              loco->button.set_theme(cid_, loco->button.get_theme(cid_), inactive);
            }
            ii_d.flag->ignore_move_focus_check = false;
          }
        }

        loco_t::mouse_button_data_t mid = ii_d;
        mid.cid = cid_;
        cb(mid);

        return 0;
      };
      vfip.keyboard_cb = [cb = p.keyboard_cb, udata = p.udata, cid_ = cid](const loco_t::vfi_t::keyboard_data_t& kd) -> int {
        loco_t* loco = OFFSETLESS(kd.vfi, loco_t, vfi_var_name);
        loco_t::keyboard_data_t kd_ = kd;
        kd_.cid = cid_;
        cb(kd_);
        return 0;
      };
    }

    sb_get_ri(cid).vfi_id = loco->vfi.push_shape(vfip);
  }
  void erase(fan::graphics::cid_t* cid) {
    loco_t* loco = get_loco();
    auto& ri = sb_get_ri(cid);
    loco->text.erase(ri.text_id);
    loco->vfi.erase(ri.vfi_id);
    sb_erase(cid);
  }

  auto& get_ri(fan::graphics::cid_t* cid) {
    return sb_get_ri(cid);
  }

  void draw() {
    sb_draw();
  }

  #if defined(loco_opengl)
    #define sb_shader_vertex_path _FAN_PATH(graphics/glsl/opengl/2D/objects/button.vs)
  #define sb_shader_fragment_path _FAN_PATH(graphics/glsl/opengl/2D/objects/button.fs)
  #elif defined(loco_vulkan)
    #define vulkan_buffer_count 4
    #define sb_shader_vertex_path _FAN_PATH_QUOTE(graphics/glsl/vulkan/2D/objects/button.vert.spv)
    #define sb_shader_fragment_path _FAN_PATH_QUOTE(graphics/glsl/vulkan/2D/objects/button.frag.spv)
  #endif
  #define vk_sb_ssbo
  #define vk_sb_vp
  #include _FAN_PATH(graphics/shape_builder.h)

  button_t() {
    sb_open();
  }
  ~button_t() {
    sb_close();
  }

  fan_2d::graphics::gui::theme_t* get_theme(fan::graphics::theme_list_NodeReference_t nr) {
    loco_t* loco = get_loco();
    return loco->get_context()->theme_list[nr].theme_id;
  }
  fan_2d::graphics::gui::theme_t* get_theme(fan::graphics::cid_t* cid) {
    return get_theme(get_ri(cid).theme);
  }
  void set_theme(fan::graphics::cid_t* cid, fan_2d::graphics::gui::theme_t* theme, f32_t intensity) {
    loco_t* loco = get_loco();
    fan_2d::graphics::gui::theme_t t = *theme;
    t = t * intensity;
    
    set(cid, &vi_t::color, t.button.color);
    set(cid, &vi_t::outline_color, t.button.outline_color);
    set(cid, &vi_t::outline_size, t.button.outline_size);
    auto& ri = get_ri(cid);
    ri.theme = theme;
    loco->text.set(ri.text_id, 
      &loco_t::letter_t::vi_t::outline_color, t.button.text_outline_color);
    loco->text.set(ri.text_id, 
      &loco_t::letter_t::vi_t::outline_size, t.button.text_outline_size);
  }

  template <typename T>
  auto get_button(fan::graphics::cid_t* cid, auto T::* member) {
    loco_t* loco = get_loco();
    return loco->button.get(cid, member);
  }
  template <typename T, typename T2>
  void set_button(fan::graphics::cid_t* cid, auto T::*member, const T2& value) {
    loco_t* loco = get_loco();
    loco->button.set(cid, member, value);
  }

  template <typename T>
  T get_text_renderer(fan::graphics::cid_t* cid, auto T::* member) {
    loco_t* loco = get_loco();
    auto block = sb_get_block(cid);
    return loco->text.get(block->p[cid->instance_id].text_id, member);
  }
  template <typename T, typename T2>
  void set_text_renderer(fan::graphics::cid_t* cid, auto T::*member, const T2& value) {
    loco_t* loco = get_loco();
    auto block = sb_get_block(cid);
    loco->text.set(block->p[cid->instance_id].text_id, member, value);
  }

  void set_position(fan::graphics::cid_t* cid, const fan::vec3& position) {
    loco_t* loco = get_loco();
    auto& ri = get_ri(cid);
    loco->text.set_position(&ri.text_id, position + fan::vec3(0, 0, 0.5));
    set_button(cid, &vi_t::position, position);
    loco->vfi.set_rectangle(
      ri.vfi_id,
      &loco_t::vfi_t::set_rectangle_t::position,
      position
    );
  }
  void set_size(fan::graphics::cid_t* cid, const fan::vec3& size) {
    loco_t* loco = get_loco();
    auto& ri = get_ri(cid);
    set_button(cid, &vi_t::size, size);
    loco->vfi.set_rectangle(
      ri.vfi_id,
      &loco_t::vfi_t::set_rectangle_t::size,
      size
    );
  }

  //void set_matrices(fan::graphics::cid_t* cid, loco_t::matrices_list_NodeReference_t n) {
  //  sb_set_key<bm_properties_t::key_t::get_index_with_type<decltype(n)>()>(cid, n);
  //  loco_t* loco = get_loco();
  //  auto block = sb_get_block(cid);
  //  loco->text.set_matrices(block->p[cid->instance_id].text_id, n);
  //}

  //fan::graphics::viewport_t* get_viewport(fan::graphics::cid_t* cid) {
  //  loco_t* loco = get_loco();
  //  auto block = sb_get_block(cid);
  //  return loco->get_context()->viewport_list[*block->p[cid->instance_id].key.get_value<1>()].viewport_id;
  //}
  /*void set_viewport(fan::graphics::cid_t* cid, fan::graphics::viewport_list_NodeReference_t n) {
    loco_t* loco = get_loco();
    sb_set_key<instance_properties_t::key_t::get_index_with_type<decltype(n)>()>(cid, n);
    auto block = sb_get_block(cid);
    loco->text.set_viewport(block->p[cid->instance_id].text_id, n);
  }*/

  void set_theme(fan::graphics::cid_t* cid, f32_t state) {
    loco_t* loco = get_loco();
    loco->button.set_theme(cid, loco->button.get_theme(cid), state);
  }

    // gets udata from current focus
  /*uint64_t get_id_udata(loco_t::vfi_t::shape_id_t id) {
    loco_t* loco = get_loco();
    auto udata = loco->vfi.get_id_udata(id);
    fan::opengl::cid_t* cid = (fan::opengl::cid_t*)udata;
    auto block = sb_get_block(cid);
    return block->p[cid->instance_id].udata;
  }*/

  void set_selected(fan::graphics::cid_t* cid, bool flag) {
    auto& ri = get_ri(cid);
    ri.selected = flag;
  }

  fan::wstring get_text(fan::graphics::cid_t* cid) {
    loco_t* loco = get_loco();
    auto& ri = get_ri(cid);
    return loco->text.get_properties(ri.text_id).text;
  }
  void set_text(fan::graphics::cid_t* cid, const fan::wstring& text) {
    loco_t* loco = get_loco();
    auto& ri = get_ri(cid);
    loco->text.set_text(&ri.text_id, text);
  }

  #if defined(loco_vulkan)
  uint32_t m_matrices_index = 0;
  #endif
};