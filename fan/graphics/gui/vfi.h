// very flexible input

#include<functional>

struct vfi_t {

  static constexpr typename loco_t::shape_type_t shape_type = loco_t::shape_type_t::hitbox;

  vfi_t() {
    focus.mouse.invalidate();
    focus.keyboard.invalidate();
    focus.text.invalidate();
  }
  ~vfi_t() {
  }

  typedef uint16_t shape_type_t;

  struct shape_t {
    static constexpr shape_type_t always = 0;
    static constexpr shape_type_t rectangle = 1;
  };
  struct shape_properties_always_t {
    f32_t z;
  };
  struct shape_data_always_t {

  };
  struct shape_properties_rectangle_t {
    fan::vec3 position;
    fan::vec2 size;
    fan::graphics::viewport_list_NodeReference_t viewport;
    loco_t::camera_list_NodeReference_t camera;
  };
  struct shape_data_rectangle_t {
    fan::vec2 position;
    fan::vec2 size;
    fan::graphics::viewport_list_NodeReference_t viewport;
    loco_t::camera_list_NodeReference_t camera;
    shape_data_rectangle_t& operator=(const shape_properties_rectangle_t& p) {
      position = *(fan::vec2*)&p.position;
      size = p.size;
      viewport = p.viewport;
      camera = p.camera;
      return *this;
    }
  };

  struct iflags_t {
    iflags_t() : ignore_button(0) {

    }
    uint32_t ignore_button : 1;
    //static constexpr shape_type_t always_check_top_focus = 1 << 0;
  };

  typedef uint16_t shape_id_integer_t;

  struct focus_method_mouse_flag {
    focus_method_mouse_flag() : ignore_move_focus_check(0) {}
    uint32_t ignore_move_focus_check : 1;
  };

  enum class mouse_stage_e {
    outside,
    inside
  };

  struct mouse_move_data_t {
    vfi_t* vfi;
    fan::vec2 position;
    mouse_stage_e mouse_stage;
    focus_method_mouse_flag* flag;
  };

  struct mouse_button_data_t {
    vfi_t* vfi;
    fan::vec2 position;
    uint16_t button;
    fan::mouse_state button_state;
    mouse_stage_e mouse_stage;
    focus_method_mouse_flag* flag;
  };

  struct keyboard_data_t {
    vfi_t* vfi;
    uint16_t key;
    fan::keyboard_state keyboard_state;
  };

  struct text_data_t {
    vfi_t* vfi;
    uint32_t key;
  };

  using mouse_move_cb_t = fan::function_t<int(const mouse_move_data_t&)>;
  using mouse_button_cb_t = fan::function_t<int(const mouse_button_data_t&)>;
  using keyboard_cb_t = fan::function_t<int(const keyboard_data_t&)>;
  using text_cb_t = fan::function_t<int(const text_data_t&)>;

  struct common_shape_data_t {

    mouse_button_cb_t mouse_button_cb = [](const mouse_button_data_t&) -> int { return 0; };
    mouse_move_cb_t mouse_move_cb = [](const mouse_move_data_t&) -> int { return 0; };
    keyboard_cb_t keyboard_cb = [](const keyboard_data_t&) -> int { return 0; };
    text_cb_t text_cb = [](const text_data_t&) -> int { return 0; };
    f32_t depth;
    union{
      struct always_t {
        always_t() = default;
        using base_t = shape_data_always_t;
        always_t(shape_data_always_t& b) {
          memcpy(data, &b, sizeof(base_t));
        }
        operator shape_data_always_t&() {
          return *(shape_data_always_t*)data;
        }
        operator shape_data_always_t&() const {
          return *(shape_data_always_t*)data;
        }
        auto* operator->() {
          return &operator shape_data_always_t&();
        }
        auto* operator->() const {
          return &operator shape_data_always_t&();
        }
        uint8_t data[sizeof(shape_data_always_t)];
      }always;
      struct rectangle_t {
        rectangle_t() = default;
        using base_t = shape_data_rectangle_t;
        rectangle_t(shape_data_rectangle_t& b) {
          memcpy(data, &b, sizeof(shape_data_rectangle_t));
        }
        operator shape_data_rectangle_t&() {
          return *(shape_data_rectangle_t*)data;
        }
        operator shape_data_rectangle_t&() const {
          return *(shape_data_rectangle_t*)data;
        }
        auto* operator->() {
          return &operator shape_data_rectangle_t&();
        }
        auto* operator->() const {
          return &operator shape_data_rectangle_t&();
        }
        uint8_t data[sizeof(shape_data_rectangle_t)];
      }rectangle;
    }shape;
  }; 

  struct common_shape_properties_t {
    using type_t = vfi_t;

    shape_type_t shape_type;
    mouse_button_cb_t mouse_button_cb = [](const mouse_button_data_t&) -> int { return 0; };
    mouse_move_cb_t mouse_move_cb = [](const mouse_move_data_t&) -> int { return 0; };
    keyboard_cb_t keyboard_cb = [](const keyboard_data_t&) -> int { return 0; };
    text_cb_t text_cb = [](const text_data_t&) -> int { return 0; };
    struct {
      struct always_t {
        always_t() = default;
        using base_t = shape_properties_always_t;
        always_t(shape_properties_always_t& b) {
          memcpy(data, &b, sizeof(base_t));
        }
        operator shape_properties_always_t&() {
          return *(shape_properties_always_t*)data;
        }
        operator shape_properties_always_t&() const {
          return *(shape_properties_always_t*)data;
        }
        auto* operator->() {
          return &operator shape_properties_always_t&();
        }
        auto* operator->() const {
          return &operator shape_properties_always_t&();
        }
        uint8_t data[sizeof(shape_properties_always_t)];
      }always;
      struct rectangle_t {
        rectangle_t() = default;
        using base_t = shape_properties_rectangle_t;
        rectangle_t(shape_properties_rectangle_t& b) {
          memcpy(data, &b, sizeof(shape_properties_rectangle_t));
        }
        operator shape_properties_rectangle_t&() {
          return *(shape_properties_rectangle_t*)data;
        }
        operator shape_properties_rectangle_t&() const {
          return *(shape_properties_rectangle_t*)data;
        }
        auto* operator->() const {
          return &operator shape_properties_rectangle_t&();
        }
        auto* operator->() {
          return &operator shape_properties_rectangle_t&();
        }
        uint8_t data[sizeof(shape_properties_rectangle_t)];
      }rectangle;
    }shape;
    iflags_t flags;
    bool ignore_init_move = false;
  }; 

  using set_shape_t = decltype(common_shape_data_t::shape);
  using set_rectangle_t = decltype(set_shape_t::rectangle)::base_t;

  #include "vfi_shape_list_settings.h"
  #include _FAN_PATH(BLL/BLL.h)

  using shape_list_nr_t = shape_list_NodeReference_t;

  using shape_id_t = loco_t::cid_nt_t;

  struct shape_id_wrap_t {
    shape_id_wrap_t() = default;
    shape_id_wrap_t(loco_t::cid_nt_t* nt) : n(nt){

    }
    bool operator==(shape_id_wrap_t id) {
      return ((shape_list_nr_t)*this).NRI == (id.operator shape_list_nr_t&()).NRI;
    }
    operator shape_list_nr_t () const {
      return *(shape_list_nr_t*)n->gdp4();
    }
    operator shape_list_nr_t&() {
      return *(shape_list_nr_t*)n->gdp4();
    }
    operator shape_id_t& () {
      return *(shape_id_t*)n;
    }
    bool is_invalid() {
      return n == nullptr || shape_list_inric(operator shape_list_nr_t&());
    }
    shape_id_wrap_t& operator=(const shape_list_nr_t& nr) {
      operator shape_list_nr_t () = nr;
      return *this;
    }
    shape_id_wrap_t& operator=(const shape_id_t& id) {
      operator shape_list_nr_t () = shape_id_wrap_t((shape_id_t*)&id).operator shape_list_nr_t();
      return *this;
    }
    void invalidate() {
      if (is_invalid()) {
        return;
      }
      gloco->shapes.vfi.shape_list.unlrec(operator shape_list_nr_t&());
      operator shape_list_nr_t&() = gloco->shapes.vfi.shape_list.gnric();
    }
    loco_t::cid_nt_t* n = nullptr;
  };

  struct {
    shape_list_nr_t mouse;
    shape_list_nr_t keyboard;
    shape_list_nr_t text;

    struct {
      struct {
        fan::vec2 position;
        focus_method_mouse_flag flags;
      }mouse;
      struct {

      }keyboard;
      struct {

      }text;
    }method;
  }focus;

  shape_list_t shape_list;

  using properties_t = common_shape_properties_t;

  void push_back(shape_id_t& in, const properties_t& p) {
    shape_id_wrap_t id(&in);
    auto nr = shape_list.NewNodeLast();
    auto& instance = shape_list[nr];
    instance.shape_type = p.shape_type;
    instance.flags = p.flags;
    instance.shape_data.mouse_move_cb = p.mouse_move_cb;
    instance.shape_data.mouse_button_cb = p.mouse_button_cb;
    instance.shape_data.keyboard_cb = p.keyboard_cb;
    instance.shape_data.text_cb = p.text_cb;
    switch(p.shape_type) {
      case shape_t::always:{
        instance.shape_data.depth = p.shape.always->z;
        break;
      }
      case shape_t::rectangle:{
        instance.shape_data.depth = p.shape.rectangle->position.z;
        instance.shape_data.shape.rectangle->camera = p.shape.rectangle->camera;
        instance.shape_data.shape.rectangle->viewport = p.shape.rectangle->viewport;
        instance.shape_data.shape.rectangle->position = *(fan::vec2*)&p.shape.rectangle->position;
        instance.shape_data.shape.rectangle->size = p.shape.rectangle->size;
        break;
      }
    }

    fan::vec2 mouse_position = gloco->get_mouse_position();
    fan::vec2 tp = transform(mouse_position, p.shape_type, &instance.shape_data);
    if (focus.mouse.is_invalid()) {
      if (!p.ignore_init_move && inside(p.shape_type, &instance.shape_data, tp) == mouse_stage_e::inside) {
        feed_mouse_move(mouse_position);
      }
    }
    in.init();
    in->shape_type = (std::underlying_type_t<loco_t::shape_type_t>)loco_t::shape_type_t::hitbox;
    *in.gdp4() = nr.NRI;
  }
  void erase(shape_id_t& in) {
    shape_id_wrap_t id = &in;
    bool fm = focus.mouse == id;
    if (fm) {
      focus.mouse.invalidate();
    }
    if (focus.keyboard == id) {
      focus.keyboard.invalidate();
    }
    if (focus.text == id) {
      focus.text.invalidate();
    }
    shape_list.unlrec(id);
    if (fm) {
      feed_mouse_move(gloco->get_mouse_position());
    }
  }
  template <typename T>
  void set_always(shape_id_t in, auto T::*member, auto value) {
    shape_id_wrap_t id(&in);
    shape_list[id].shape_data.shape.always->*member = value;
  }
  template <typename T>
  void set_rectangle(shape_id_t in, auto T::*member, auto value) {
    shape_id_wrap_t id(&in);
    ((T*)&shape_list[id].shape_data.shape.rectangle)->*member = value;
  }

  void set_common_data(shape_id_t in, auto common_shape_data_t::*member, auto value) {
    shape_id_wrap_t id(&in);
    shape_list[id].shape_data.*member = value;
  }

  static fan::vec2 transform_position(const fan::vec2& p, fan::graphics::viewport_t* viewport, loco_t::camera_t* camera) {
      
    fan::vec2 viewport_position = viewport->get_position(); 
    fan::vec2 viewport_size = viewport->get_size();

    f32_t l = camera->coordinates.left;
    f32_t r = camera->coordinates.right;
    f32_t t = camera->coordinates.up;
    f32_t b = camera->coordinates.down;

    fan::vec2 tp = p - viewport_position;
    fan::vec2 d = viewport_size;
    tp /= d;
    tp = fan::vec2(r * tp.x - l * tp.x + l, b * tp.y - t * tp.y + t);
    return tp;
  }

  mouse_stage_e inside(shape_type_t shape_type, common_shape_data_t* data, const fan::vec2& p) {
    switch(shape_type) {
      case shape_t::always: {
        return mouse_stage_e::inside;
      }
      case shape_t::rectangle: {
        fan::vec2 size = data->shape.rectangle->size;
        bool in = fan_2d::collision::rectangle::point_inside_no_rotation(
          p,
          data->shape.rectangle->position,
          size
        );
       /* if (!loco->get_context().viewport_list[data->shape.rectangle.viewport].viewport_id->inside_wir(p)) {
          return mouse_stage_e::outside;
        }*/
        return in ? mouse_stage_e::inside : mouse_stage_e::outside;
      }
      default: {
        fan::throw_error("invalid shape_type");
        return mouse_stage_e::outside;
      }
    }
  };

  fan::vec2 transform(const fan::vec2& v, shape_type_t shape_type, common_shape_data_t* shape_data) {
    switch (shape_type) {
      case shape_t::always: {
        return v;
      }
      case shape_t::rectangle: {
        return transform_position(
          v,
          gloco->get_context().viewport_list[shape_data->shape.rectangle->viewport].viewport_id,
          gloco->camera_list[shape_data->shape.rectangle->camera].camera_id
        ) + fan::vec2(*(fan::vec2*)&gloco->camera_list[shape_data->shape.rectangle->camera].camera_id->camera_position);
        break;
      }
      default: {
        fan::throw_error("invalid shape type");
        return {};
      }
    }
  }

  void init_focus_mouse_flag() {
    focus.method.mouse.flags.ignore_move_focus_check = false;
  }

  // might fail because shape_type isnt set
  shape_id_t get_focus_mouse() {
    shape_id_t s;
    s.init();
    s->shape_type = (std::underlying_type_t<loco_t::shape_type_t>)loco_t::shape_type_t::hitbox;
    *s.gdp4() = focus.mouse.NRI;
    return s;
  }
  void set_focus_mouse(shape_id_t id) {
    focus.mouse = shape_id_wrap_t(&id).operator shape_list_nr_t();
    init_focus_mouse_flag();
  }
  void set_focus_mouse(shape_list_nr_t id) {
    focus.mouse = id;
    init_focus_mouse_flag();
  }
  shape_id_t get_focus_keyboard() {
    shape_id_t s;
    s.init();
    s->shape_type = (std::underlying_type_t<loco_t::shape_type_t>)loco_t::shape_type_t::hitbox;
    *s.gdp4() = focus.keyboard.NRI;
    return s;
  }
  void set_focus_keyboard(shape_id_t id) {
    focus.keyboard = shape_id_wrap_t(&id).operator shape_list_nr_t();
  }
  void set_focus_keyboard(shape_list_nr_t id) {
    focus.keyboard = id;
    init_focus_mouse_flag();
  }
  shape_id_t get_focus_text() {
    shape_id_t s;
    s.init();
    s->shape_type = (std::underlying_type_t<loco_t::shape_type_t>)loco_t::shape_type_t::hitbox;
    *s.gdp4() = focus.text.NRI;
    return s;
  }
  void set_focus_text(shape_id_t id) {
    focus.text = shape_id_wrap_t(&id).operator shape_list_nr_t();
  }

  void invalidate_focus_mouse() {
    focus.mouse.invalidate();
    focus.method.mouse.flags.ignore_move_focus_check = false;
  }
  void invalidate_focus_keyboard() {
    focus.keyboard.invalidate();
  }
  void invalidate_focus_text() {
    focus.text.invalidate();
  }

  void feed_mouse_move(const fan::vec2& position) {
    focus.method.mouse.position = position;
    mouse_move_data_t mouse_move_data;
    mouse_move_data.vfi = this;
    mouse_move_data.flag = &focus.method.mouse.flags;
    if (!focus.mouse.is_invalid()) {
      auto& data = shape_list[focus.mouse];
      fan::vec2 tp = transform(position, data.shape_type, &data.shape_data);
      mouse_move_data.mouse_stage = inside(data.shape_type, &data.shape_data, tp);
      mouse_move_data.position = tp;
      auto bcbfm = focus.mouse;
      data.shape_data.mouse_move_cb(mouse_move_data);
      if (bcbfm != focus.mouse) {
        data = shape_list[focus.mouse];
        tp = transform(position, data.shape_type, &data.shape_data);
        mouse_move_data.mouse_stage = inside(data.shape_type, &data.shape_data, tp);
      }
      if (focus.method.mouse.flags.ignore_move_focus_check == true) {
        return;
      }
    }

    f32_t closest_z = -1;
    shape_list_NodeReference_t closest_z_nr;

    auto it = shape_list.GetNodeFirst();
    while(it != shape_list.dst) {
      auto& data = shape_list[it];
      fan::vec2 tp = transform(position, data.shape_type, &data.shape_data);
      mouse_move_data.mouse_stage = inside(data.shape_type, &data.shape_data, tp);
      if (mouse_move_data.mouse_stage == mouse_stage_e::inside) {
        if (data.shape_data.depth > closest_z) {
          closest_z = data.shape_data.depth;
          closest_z_nr = it;
        }
      }
      it = it.Next(&shape_list);
    }
    if (closest_z != -1) {
      auto* data = &shape_list[closest_z_nr];
      fan::vec2 tp = transform(position, data->shape_type, &data->shape_data);
      mouse_move_data.position = tp;
      mouse_move_data.mouse_stage = inside(data->shape_type, &data->shape_data, tp);
      set_focus_mouse(closest_z_nr); // can be wrong
      data->shape_data.mouse_move_cb(mouse_move_data);
      return;
    }
    focus.mouse.invalidate();
    return;
  }

  void feed_mouse_button(uint16_t button, fan::mouse_state state) {
    mouse_button_data_t mouse_button_data;
    mouse_button_data.vfi = this;
    if (focus.mouse.is_invalid()) {
      return;
    }

    mouse_button_data.button = button;
    mouse_button_data.button_state = state;

    auto* data = &shape_list[focus.mouse];

    mouse_button_data.position = transform(focus.method.mouse.position, data->shape_type, &data->shape_data);
    mouse_button_data.mouse_stage = inside(data->shape_type, &data->shape_data, mouse_button_data.position);
    mouse_button_data.flag = &focus.method.mouse.flags;
    auto bcbfm = focus.mouse;

    data->shape_data.mouse_button_cb(mouse_button_data);

    if (bcbfm != focus.mouse) {
      if (focus.mouse.is_invalid()) {
        return;
      }
      data = &shape_list[focus.mouse];
      mouse_button_data.position = transform(focus.method.mouse.position, data->shape_type, &data->shape_data);
      mouse_button_data.mouse_stage = inside(data->shape_type, &data->shape_data, mouse_button_data.position);
    }

    if (mouse_button_data.mouse_stage == mouse_stage_e::outside) {
      if (focus.method.mouse.flags.ignore_move_focus_check == false) {
        focus.mouse.invalidate();
        feed_mouse_move(focus.method.mouse.position);
      }
    }
  }
  void feed_keyboard(uint16_t key, fan::keyboard_state keyboard_state) {
    keyboard_data_t keyboard_data;
    keyboard_data.vfi = this;
    if (focus.keyboard.is_invalid()) {
      return;
    }

    keyboard_data.key = key;
    keyboard_data.keyboard_state = keyboard_state;

    shape_list[focus.keyboard].shape_data.keyboard_cb(keyboard_data);
  }
  void feed_text(uint32_t key) {
    text_data_t text_data;
    text_data.vfi = this;
    if (focus.text.is_invalid()) {
      return;
    }

    text_data.key = key;

    shape_list[focus.text].shape_data.text_cb(text_data);
  }

  fan::vec3 get_position(shape_id_t& in) {
    shape_id_wrap_t id(&in);
    auto& shape = shape_list[id];
    switch (shape.shape_type) {
      case vfi_t::shape_t::rectangle: {
        return fan::vec3(
          shape.shape_data.shape.rectangle->position, 
          shape.shape_data.depth
          );
      }
    }
    fan::throw_error("invalid get_position for id");
  }
  void set_position(shape_id_t& in, const fan::vec3& position) {
    shape_id_wrap_t id(&in);
    auto& shape = shape_list[id];
    switch (shape.shape_type) {
      case vfi_t::shape_t::rectangle: {
        shape.shape_data.shape.rectangle->position = *(fan::vec2*)&position;
        shape.shape_data.depth = position.z;
        return;
      }
    }
    fan::throw_error("invalid set_position for id");
  }

  fan::vec2 get_size(shape_id_t& in) {
    shape_id_wrap_t id(&in);
    auto& shape = shape_list[id];
    switch (shape.shape_type) {
      case vfi_t::shape_t::rectangle: {
        return shape.shape_data.shape.rectangle->size;
      }
    }
    fan::throw_error("invalid get_position for id");
  }
  void set_size(shape_id_t& in, const fan::vec2& size) {
    shape_id_wrap_t id(&in);
    auto& shape = shape_list[id];
    switch (shape.shape_type) {
      case vfi_t::shape_t::rectangle: {
        shape.shape_data.shape.rectangle->size = size;
        return;
      }
    }
    fan::throw_error("invalid set_position for id");
  }
};