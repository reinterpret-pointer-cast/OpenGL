case builder_draw_type_t::rectangle_text_button_sized: {
  pile->builder.rtbs.set_position(
    &pile->window,
    &pile->context,
    pile->editor.selected_type_index,
    position + pile->editor.click_position
  );

  pile->editor.update_resize_rectangles(pile);

  break;
}