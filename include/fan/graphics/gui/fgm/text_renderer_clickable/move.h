case builder_draw_type_t::text_renderer_clickable: {
  pile->builder.trc.set_position(
    &pile->window,
    &pile->context,
    pile->editor.selected_type_index,
    position + pile->editor.click_position
  );

  pile->editor.update_resize_rectangles(pile);

  break;
}