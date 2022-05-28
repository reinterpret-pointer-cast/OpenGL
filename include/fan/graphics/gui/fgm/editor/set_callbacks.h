builder_types.m_button_event.set_on_input(pile,
  [](
  fan::window_t* window,
  fan::opengl::context_t* context,
  uint32_t index,
  uint16_t key,
  fan::key_state key_state,
  fan_2d::graphics::gui::mouse_stage mouse_stage,
  void* user_ptr
) {

  if (key != fan::mouse_left) {
    return;
  }
  if (mouse_stage != fan_2d::graphics::gui::mouse_stage::inside) {
    return;
  }

  pile_t* pile = (pile_t*)user_ptr;

  if (!pile->editor.is_inside_types_viewport(pile, window->get_mouse_position())) {
    return;
  }

  switch (index) {

    #include _FAN_PATH(graphics/gui/fgm/rectangle_sized_text_button/create.h)
    #include _FAN_PATH(graphics/gui/fgm/text_renderer_clickable/create.h)
  }
});

pile->window.add_mouse_move_callback(pile,
  [](fan::window_t* window, const fan::vec2i& position, void* user_ptr) {
  pile_t* pile = (pile_t*)user_ptr;

  if (!(pile->editor.flags & flags_t::ignore_moving)) {

    if (!(pile->editor.flags & flags_t::moving)) {
      return;
    }

    switch (pile->editor.selected_type) {
      #include _FAN_PATH(graphics/gui/fgm/rectangle_sized_text_button/move.h)
      #include _FAN_PATH(graphics/gui/fgm/text_renderer_clickable/move.h)
    }
  }
  if (pile->editor.flags & flags_t::resizing) {
    switch (pile->editor.selected_type) {
      #include _FAN_PATH(graphics/gui/fgm/rectangle_sized_text_button/resize.h)
      #include _FAN_PATH(graphics/gui/fgm/text_renderer_clickable/resize.h)
    }
  }

});

pile->window.add_keys_callback(pile,
  [](fan::window_t* window, uint16_t key, fan::key_state key_state, void* user_ptr)
{
  pile_t* pile = (pile_t*)user_ptr;

  switch (key) {
    #include _FAN_PATH(graphics/gui/fgm/input/mouse_left.h)
    #include _FAN_PATH(graphics/gui/fgm/input/mouse_scroll_up.h)
    #include _FAN_PATH(graphics/gui/fgm/input/mouse_scroll_down.h)
    #include _FAN_PATH(graphics/gui/fgm/input/key_delete.h)
  }
});

pile->editor.resize_rectangles.m_button_event.set_on_input(pile,
  [](
  fan::window_t* window,
  fan::opengl::context_t* context,
  uint32_t index,
  uint16_t key,
  fan::key_state key_state,
  fan_2d::graphics::gui::mouse_stage mouse_stage,
  void* user_ptr
) {

  if (key != fan::mouse_left) {
    return;
  }

  pile_t* pile = (pile_t*)user_ptr;

  if (key_state == fan::key_state::release) {
    pile->editor.flags &= ~flags_t::ignore_properties_close;
    pile->editor.flags &= ~flags_t::ignore_moving;
    pile->editor.flags &= ~flags_t::resizing;
  }

  if (mouse_stage != fan_2d::graphics::gui::mouse_stage::inside) {
    return;
  }

  if (!pile->editor.is_inside_builder_viewport(pile, window->get_mouse_position())) {
    return;
  }
  
  pile->editor.resize_stage = index;
  switch (pile->editor.selected_type) {
    case builder_draw_type_t::rectangle_text_button_sized: {
      pile->editor.click_position = pile->builder.rtbs.get_position(&pile->window, &pile->context, pile->editor.selected_type_index);
      break;
    }
    case builder_draw_type_t::text_renderer_clickable: {
      pile->editor.click_position = pile->builder.trc.get_position(&pile->window, &pile->context, pile->editor.selected_type_index);
      break;
    }
  }
  //pile->editor.click_position = window->get_mouse_position();
  pile->editor.flags |= flags_t::ignore_properties_close;
  pile->editor.flags |= flags_t::ignore_moving;
  pile->editor.flags |= flags_t::resizing;

});