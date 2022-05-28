pile->builder.trc.erase(
            &pile->window,
            &pile->context,
            pile->editor.selected_type_index
);
for (uint32_t i = 0; i < pile->editor.depth_map.size(); i++) {
  if (pile->editor.depth_map[i].type == pile->editor.selected_type &&
      pile->editor.depth_map[i].index == pile->editor.selected_type_index) {
    pile->editor.depth_map.erase(i);

    for (uint32_t j = i; j < pile->editor.depth_map.size(); j++) {
      if (pile->editor.selected_type != pile->editor.depth_map[j].type) {
        continue;
      }
      pile->editor.depth_map[j].index--;
    }

    pile->editor.close_build_properties(pile);
    break;
  }
}