struct responsive_text_t {

  static constexpr typename loco_t::shape_type_t::_t shape_type = loco_t::shape_type_t::responsive_text;

  struct align_e {
    uint8_t alignment = center;
    static constexpr uint8_t left = 0;
    static constexpr uint8_t center = 1;
    //static constexpr uint8_t right = 2;
  };

  using lvi_t = loco_t::letter_t::vi_t;
  using lri_t = loco_t::letter_t::ri_t;

  struct properties_t : lvi_t, lri_t {
    using type_t = responsive_text_t;

    align_e alignment;

    loco_text_properties_t
  };

  #define BLL_set_CPP_nrsic 0
  #define BLL_set_CPP_ConstructDestruct
  #define BLL_set_CPP_Node_ConstructDestruct
  #define BLL_set_AreWeInsideStruct 1
  #define BLL_set_BaseLibrary 1
  #define BLL_set_prefix letter_list
  #define BLL_set_type_node uint32_t
  #define BLL_set_NodeData \
    loco_t::shape_t shape; \
    fan::font::character_info_nr_t internal_id; \
    fan::vec2 position;
  #define BLL_set_Link 1
  #include _FAN_PATH(BLL/BLL.h)
  letter_list_t letter_list;

  #define BLL_set_CPP_nrsic 0
  #define BLL_set_CPP_CopyAtPointerChange
  #define BLL_set_CPP_ConstructDestruct
  #define BLL_set_CPP_Node_ConstructDestruct
  #define BLL_set_AreWeInsideStruct 1
  #define BLL_set_BaseLibrary 1
  #define BLL_set_prefix line_list
  #define BLL_set_type_node uint32_t
  #define BLL_set_NodeData \
    f32_t total_width = 0;
  #include _FAN_PATH(BLL/BLL.h)
  line_list_t line_list;

  #define BLL_set_CPP_nrsic 0
  #define BLL_set_CPP_CopyAtPointerChange
  #define BLL_set_CPP_ConstructDestruct
  #define BLL_set_CPP_Node_ConstructDestruct
  #define BLL_set_AreWeInsideStruct 1
  #define BLL_set_BaseLibrary 1
  #define BLL_set_prefix tlist
  #define BLL_set_Link 0
  #define BLL_set_type_node uint32_t
  #define BLL_set_NodeData \
    letter_list_NodeReference_t LetterStartNR{true}; \
    letter_list_NodeReference_t LetterEndNR; \
    line_list_NodeReference_t LineStartNR; /* TODO need reinit at reset */ \
    uint32_t LineCount = 1; /* TODO need reinit at reset */ \
    loco_t::camera_t* camera = 0; \
    loco_t::viewport_t* viewport = 0; \
    fan::vec3 position; \
    f32_t font_size; \
    fan::color color; \
    fan::color outline_color; \
    f32_t outline_size; \
    align_e alignment; \
    fan::vec2 size = 0; \
    fan::vec2 max_sizes = 0; \
    line_list_NodeReference_t max_x_sized_line{true}; /* TODO does it really need sic? */
  #include _FAN_PATH(BLL/BLL.h)
  tlist_t tlist;

  responsive_text_t() {

  }

  void push_back(loco_t::cid_nt_t& id, const properties_t& properties) {

    tlist_NodeReference_t instance_id = tlist.NewNode();

    id->shape_type = loco_t::shape_type_t::responsive_text;
    *id.gdp4() = instance_id.NRI;

    {
      auto& instance = tlist[instance_id];
      instance.camera = properties.camera;
      instance.viewport = properties.viewport;
      instance.position = properties.position;
      instance.font_size = 1; // properties.font_size
      instance.color = properties.color;
      instance.outline_color = properties.outline_color;
      instance.outline_size = properties.outline_size;
      instance.size = properties.size;
      instance.LineStartNR = line_list.NewNodeLast();
    }

    // todo use aligment
    for (uintptr_t i = 0; i != properties.text.size();) {
      uint8_t letter_size = properties.text.utf8_size(i);
      uint32_t utf8_letter = properties.text.get_utf8_character(i, letter_size);
      auto found = gloco->font.info.characters.find(utf8_letter);
      if (found == gloco->font.info.characters.end()) {
        fan::throw_error("invalid utf8 letter");
      }
      internal_append_letter(instance_id, found->second, true);
      /*

            append_letter(id, properties.text[i]);
      uint8_t letter_size = properties.text.utf8_size(0);
      i += letter_size;
      */

      i += letter_size;
    }
  }

  void erase(loco_t::cid_nt_t& id) {
    //auto internal_id = *(tlist_NodeReference_t*)id.gdp4();
    //__abort();
    //tlist.unlrec(internal_id);
    auto internal_id = *(tlist_NodeReference_t*)id.gdp4();
    auto& instance = tlist[internal_id];
    free_letters(internal_id);

    {
      auto lnr = instance.LineStartNR;
      for(auto i = instance.LineCount; i--;){
        auto nlnr = lnr.Next(&line_list);
        line_list.unlrec(lnr);
        lnr = nlnr;
      }
    }

    tlist.Recycle(internal_id);
  }

  bool append_letter(loco_t::cid_nt_t& id, wchar_t wc, bool force = false) {
    fan::string utf8(&wc, &wc + 1);
    uint8_t letter_size = utf8.utf8_size(0);
    uint32_t utf8_letter = utf8.get_utf8_character(0, letter_size);
    auto found = gloco->font.info.characters.find(utf8_letter);
    if (found == gloco->font.info.characters.end()) {
      fan::throw_error("invalid utf8 letter");
    }
    auto instance_id = *(tlist_NodeReference_t*)id.gdp4();
    return internal_append_letter(instance_id, found->second, force);
  }

  // also calculates total_width
  bool get_letter_position(tlist_NodeReference_t instance_id, fan::font::character_info_nr_t char_internal_id, fan::vec3& position, bool force, auto working_letter_nr, bool is_first) {
    auto& instance = tlist[instance_id];

    auto new_character_info = gloco->font.info.get_letter_info(char_internal_id, gloco->font.info.size);

    static auto calculate_letter_position = [&](const auto& character_info, const fan::vec3& p) {
      return fan::vec3(
        p.x + character_info.metrics.size.x / 2,
        p.y + (gloco->font.info.size  - character_info.metrics.size.y) / 2 - character_info.metrics.offset.y,
        0
      );
      };

    if (is_first) {
      position = 0;
      position = calculate_letter_position(new_character_info, position);
      position.z = instance.position.z;
    }
    else {
      auto& old_letter = letter_list[working_letter_nr];
      auto old_character_info = gloco->font.info.get_letter_info(old_letter.internal_id, gloco->font.info.size );
      fan::vec3 o_pos = old_letter.position;
      position = o_pos;
      position -= calculate_letter_position(old_character_info, 0);
      position = calculate_letter_position(new_character_info, position);
      position.x += old_character_info.metrics.advance;
      position.z = instance.position.z;
    }
    if ((position.x + new_character_info.metrics.size.x / 2) * instance.font_size > instance.size.x * 2 && force == false) {
      return false;
    }
    line_list[instance.LineStartNR].total_width = position.x + new_character_info.metrics.size.x / 2;
    return true;
  }

  bool internal_append_letter(tlist_NodeReference_t instance_id, fan::font::character_info_nr_t char_internal_id, bool force = false) {

    auto& instance = tlist[instance_id];

    typename loco_t::letter_t::properties_t p;
    p.color = instance.color;
    p.font_size = instance.font_size;
    p.camera = instance.camera;
    p.viewport = instance.viewport;
    p.outline_color = instance.outline_color;
    p.outline_size = instance.outline_size;
 
    p.letter_id = gloco->font.info.character_info_list[char_internal_id].utf8_character;

    if (!get_letter_position(instance_id, char_internal_id, p.position, force, instance.LetterEndNR, instance.LetterStartNR.iic())) {
      return false;
    }


    {
      letter_list_NodeReference_t letter_nr;
      if (instance.LetterStartNR.iic()) {
        letter_nr = letter_list.NewNodeLast();
        instance.LetterStartNR = letter_nr;
        instance.LetterEndNR = letter_nr;
      }
      else {
        letter_nr = letter_list.NewNode();
        letter_list.linkNext(instance.LetterEndNR, letter_nr);
        instance.LetterEndNR = letter_nr;
      }
      auto& letter = letter_list[letter_nr];
      letter.position = p.position;
      letter.internal_id = char_internal_id;

      p.position *= instance.font_size;
      p.position += instance.position - fan::vec3(instance.size.x, 0, 0);
      letter.shape = p;
    }

    if (line_list[instance.LineStartNR].total_width > instance.max_sizes.x) {
      instance.max_sizes.x = line_list[instance.LineStartNR].total_width;
      instance.max_x_sized_line = instance.LineStartNR;
      update_characters_with_max_size(instance_id);
    }

    return true;
  }


  bool reset_position_size(tlist_NodeReference_t instance_id, bool force = false) {

    auto& instance = tlist[instance_id];

    auto nr = instance.LetterStartNR;
    if(nr.iic()){
      return false; // is it false
    }
    fan::vec3 position = 0;
    do{
      fan::font::character_info_nr_t char_internal_id = letter_list[nr].internal_id;

      if (!get_letter_position(instance_id, char_internal_id, position, force, nr.Prev(&letter_list), nr == instance.LetterStartNR)) {
        return false;
      }
      position *= instance.font_size;
      position += instance.position - fan::vec3(instance.size.x, 0, 0);
      letter_list[nr].shape.set_position(position);
      auto new_character_info = gloco->font.info.get_letter_info(char_internal_id, gloco->font.info.size);
      letter_list[nr].shape.set_size(new_character_info.metrics.size / 2 * instance.font_size);

      if (nr == instance.LetterEndNR) {
        break;
      }
      nr = nr.Next(&letter_list);
    }while(1);
    return true;
  }

  void update_max_sizes(tlist_NodeReference_t instance_id) {
    auto& instance = tlist[instance_id];

    instance.max_sizes = fan::vec2(0, instance.size.y);

    auto nr = instance.LineStartNR;
    for (uint32_t i = instance.LineCount; i--;) {
      auto& line = line_list[nr];
      if (line.total_width > instance.max_sizes.x) {
        instance.max_x_sized_line = nr;
      }
      instance.max_sizes.x = std::max(instance.max_sizes.x, line.total_width);
      nr = nr.Next(&line_list);
    }

  }

  void update_characters_with_max_size(tlist_NodeReference_t instance_id) {
    auto& instance = tlist[instance_id];

    f32_t scaler = instance.size.x * 2 / instance.max_sizes.x;
    //fan::print("salsa", scaler, instance.size.y * 2 / gloco->font.info.height);
    scaler = std::min(scaler, instance.size.y * 2 / gloco->font.info.height);
    instance.font_size = scaler;
    reset_position_size(instance_id, true);
  }

  void set_size(loco_t::cid_nt_t& id, const fan::vec2& new_size) {
    auto internal_id = *(tlist_NodeReference_t*)id.gdp4();
    auto& instance = tlist[internal_id];

    instance.size = new_size;

    update_max_sizes(internal_id);
    update_characters_with_max_size(internal_id);
  }

  void set_outline_color(loco_t::cid_nt_t& id, const fan::color& color) {
    auto internal_id = *(tlist_NodeReference_t*)id.gdp4();
    auto& instance = tlist[internal_id];
    auto nr = instance.LetterStartNR;
    if(nr.iic()){
      return;
    }
    do {
      letter_list[nr].shape.set_outline_color(color);
      if (nr == instance.LetterEndNR) {
        break;
      }
      nr = nr.Next(&letter_list);
    } while (1);
  }
  void set_outline_size(loco_t::cid_nt_t& id, f32_t size) {
    auto internal_id = *(tlist_NodeReference_t*)id.gdp4();
    auto& instance = tlist[internal_id];
    auto nr = instance.LetterStartNR;
    if(nr.iic()){
      return;
    }
    do {
      letter_list[nr].shape.set_outline_size(size);
      if (nr == instance.LetterEndNR) {
        break;
      }
      nr = nr.Next(&letter_list);
    } while (1);
  }
  void free_letters(tlist_NodeReference_t internal_id){
    auto& instance = tlist[internal_id];

    {
      auto nr = instance.LetterStartNR;
      if(nr.iic()){
        return;
      }
      do {
        auto nnr = nr.Next(&letter_list);
        letter_list.unlrec(nr);
        if (nr == instance.LetterEndNR) {
          break;
        }
        nr = nnr;
      }while(1);
    }
  }
  void set_text(loco_t::cid_nt_t& id, const fan::string& text) {
    auto internal_id = *(tlist_NodeReference_t*)id.gdp4();
    auto& instance = tlist[internal_id];
    free_letters(internal_id);

    instance.max_sizes = 0;
    instance.LetterStartNR.sic();

    for (uintptr_t i = 0; i != text.size();) {
      uint8_t letter_size = text.utf8_size(i);
      uint32_t utf8_letter = text.get_utf8_character(i, letter_size);
      auto found = gloco->font.info.characters.find(utf8_letter);
      if (found == gloco->font.info.characters.end()) {
        fan::throw_error("invalid utf8 letter");
      }
      internal_append_letter(internal_id, found->second);

      i += letter_size;
    }
  }

  auto& get_instance(loco_t::cid_nt_t& id) {
    auto internal_id = *(tlist_NodeReference_t*)id.gdp4();
    return tlist[internal_id];
  }

  fan::string get_text(loco_t::cid_nt_t& id) {
    auto internal_id = *(tlist_NodeReference_t*)id.gdp4();
    auto& instance = tlist[internal_id];

    fan::string r;
    auto nr = instance.LetterStartNR;
    if(nr.iic()){
      return "";
    }
    do {
      r += gloco->font.info.character_info_list[letter_list[nr].internal_id].utf8_character;
      if (nr == instance.LetterEndNR) {
        break;
      }
      nr = nr.Next(&letter_list);
    } while (1);

    return r;
  }
  fan::vec3 get_text_left_position(loco_t::cid_nt_t& id) {
    auto internal_id = *(tlist_NodeReference_t*)id.gdp4();
    auto& instance = tlist[internal_id];
    return instance.position - fan::vec3(instance.size, 0);
  }

  // maybe internal variables need copy? such as letterstartnr
  properties_t get_properties(loco_t::cid_nt_t& id) {
    auto internal_id = *(tlist_NodeReference_t*)id.gdp4();
    auto& instance = tlist[internal_id];
    properties_t p;
    p.camera = instance.camera;
    //p.theme =  gloco->get_context()->theme_list[*p.key.get_values<loco_t::t>()].matrices_id;
    p.viewport = instance.viewport;
    p.position = instance.position;
    p.color = instance.color;
    p.text = get_text(id);
    p.font_size = instance.font_size;
    p.outline_color = instance.outline_color;
    p.outline_size = instance.outline_size;
    p.alignment = instance.alignment;
    p.size = instance.size;
    return p;
  }

  //bool does_letter_fit(uint32_t wc, bool force = false) {
  //  f32_t scaler = get_scaler();
  //  fan::vec2 size = get_size() * 2;
  //  f32_t font_size = gloco->font.info.size * scaler;
  //  f32_t line0_width = gloco->text.get_text_size(m_text_lines[line_index].get_text(),
  //    font_size).x;
  //  f32_t line1_width = gloco->font.info.get_letter_info(wc, font_size).metrics.size.x;
  //  f32_t total = line0_width + line1_width;
  //  if (total <= size.x) {
  //    return true;
  //  }
  //  if (line_index + 1 < uint32_t(1.0 / current_font_size)) {
  //    ++line_index;
  //    return true;
  //  }

  //  if (force == true) {
  //    return true;
  //  }

  //  return false;
  //}

  //bool does_text_fit(const fan::string& text, bool force = false) {
  //  f32_t scaler = get_scaler();
  //  fan::vec2 size = get_size() * 2;
  //  f32_t font_size = gloco->font.info.size * scaler;
  //  f32_t line0_width = gloco->text.get_text_size(m_text_lines[line_index].get_text(), font_size).x;
  //  f32_t line1_width = gloco->text.get_text_size(text, font_size).x;
  //  f32_t total = line0_width + line1_width;
  //  if (total <= size.x) {
  //    return true;
  //  }
  //  if (line_index + 1 < uint32_t(1.0 / current_font_size)) {
  //    ++line_index;
  //    return true;
  //  }

  //  if (force == true) {
  //    return true;
  //  }

  //  return false;
  //}

  //f32_t get_scaler() {
  //  fan::vec2 size = get_size();
  //  f32_t scaler = current_font_size * size.y * 2 / gloco->font.info.height;
  //  f32_t biggest = 0;
  //  for (uint32_t i = 0; i < uint32_t(1.0 / current_font_size); ++i) {
  //    f32_t new_font_size = gloco->font.info.size * scaler;
  //    f32_t text_width = gloco->text.get_text_size(m_text_lines[i].get_text(), new_font_size).x / 2;
  //    biggest = std::max(biggest, text_width);
  //  }
  //  scaler *= std::min(size.x / biggest, 1.f);
  //  return scaler;
  //}

  //void calculate_font_size() {
  //  f32_t scaler = get_scaler();
  //  for (uint32_t i = 0; i < uint32_t(1.0 / current_font_size); ++i) {
  //    m_text_lines[i].set_font_size(gloco->font.info.size * scaler);
  //  }
  //}

  //void set_position(const fan::vec3& v) {
  //  m_position = v;

  //  calculate_text_positions();
  //}
  //fan::vec2 get_size() const {
  //  return m_size;
  //}
  //void set_size(const fan::vec2& size) {
  //  m_size = size;
  //  calculate_font_size();
  //  calculate_text_positions();
  //}

  //void push_letter_force(loco_t::text_t::tlist_NodeReference_t nr, f32_t left, f32_t& advance, uint32_t wc) {
  //  does_letter_fit(wc, true);
  //  // might need properties no idea
  //  gloco->text.append_letter(m_text_lines[line_index], wc, nr, left, advance);
  //}
};