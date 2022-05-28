case builder_draw_type_t::text_renderer_clickable: {
  switch (key_state) {
    case fan::key_state::press: {
      pile->editor.flags |= flags_t::moving;
      pile->editor.click_position = pile->window.get_mouse_position();

      decltype(pile->builder.trc)::properties_t trc_p;
      trc_p.position = pile->window.get_mouse_position();
      // fetch same size as gui
      trc_p.hitbox_size = pile->editor.builder_types.get_size(&pile->window, &pile->context, 0);
      trc_p.hitbox_position = trc_p.position;
      trc_p.text = "Clickable text";
      trc_p.font_size = constants::gui_size;
      trc_p.text_color = fan::colors::white;
     
      depth_t depth;
      depth.depth = pile->editor.depth_index++;
      depth.type = builder_draw_type_t::text_renderer_clickable;
      depth.index = pile->builder.trc.size(&pile->window, &pile->context);
      pile->editor.depth_map.push_back(depth);

      pile->builder.trc.push_back(&pile->window, &pile->context, trc_p);

      pile->editor.builder_draw_type = editor_t::builder_draw_type_t::text_renderer_clickable;
      pile->editor.builder_draw_type_index = pile->builder.trc.size(&pile->window, &pile->context) - 1;
      pile->editor.selected_type = editor_t::builder_draw_type_t::text_renderer_clickable;
      pile->editor.selected_type_index = pile->editor.builder_draw_type_index;

      break;
    }
    case fan::key_state::release: {
      pile->editor.flags &= ~flags_t::moving;

      switch (pile->editor.builder_draw_type) {
        case builder_draw_type_t::text_renderer_clickable: {

          // if object is not within builder_viewport we will delete it
          if (!pile->editor.is_inside_builder_viewport(
            pile,
            pile->builder.trc.get_hitbox_position(
            &pile->window,
            &pile->context,
            pile->editor.builder_draw_type_index
            ) +
            pile->builder.trc.get_hitbox_size(
            &pile->window,
            &pile->context,
            pile->editor.builder_draw_type_index
            )
            ))
          {
            pile->editor.close_build_properties(pile);
            pile->builder.trc.erase(
              &pile->window,
              &pile->context,
              pile->editor.builder_draw_type_index
            );
            pile->editor.depth_map.erase(pile->editor.depth_map.size() - 1);
          }
          else {
            pile->editor.close_build_properties(pile);
            click_collision_t click_collision;
            click_collision.builder_draw_type = pile->editor.builder_draw_type;
            click_collision.builder_draw_type_index = pile->editor.builder_draw_type_index;
            pile->editor.open_build_properties(pile, click_collision);
          }
          break;
        }
      }
      break;
    }
  }
  break;
}