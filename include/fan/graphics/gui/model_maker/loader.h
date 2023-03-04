#include <variant>

struct model_loader_t {

  struct private_ {
    #define fgm_build_model_maker
    #include _FAN_PATH(graphics/gui/fgm/common.h)
    #undef fgm_build_model_maker
  };

  struct iterator_t {
    fan_masterpiece_make(
      (private_::stage_maker_shape_format::shape_sprite_t)sprite,
      (private_::stage_maker_shape_format::shape_mark_t)mark
    );
  };

  using sprite_t = private_::stage_maker_shape_format::shape_sprite_t;
  using mark_t = private_::stage_maker_shape_format::shape_mark_t;

  void load(loco_t::texturepack_t* tp, const fan::string& path, auto lambda) {
    fan::string f;

    if (!fan::io::file::exists(path)) {
      return;
    }
    fan::io::file::read(path, &f);

    if (f.empty()) {
      return;
    }

    iterator_t iterator;

    uint64_t offset = 0;
    // read header
    uint32_t header = fan::read_data<uint32_t>(f, offset);
    iterator.iterate_masterpiece([&](auto& d) {
      // read the type
      auto type = fan::read_data<loco_t::shape_type_t::_t>(f, offset);
      uint32_t instance_count = fan::read_data<uint32_t>(f, offset);
      for (uint32_t i = 0; i < instance_count; ++i) {
        d.iterate_masterpiece([&](auto& o) {
          o = fan::read_data<std::remove_reference_t<decltype(o)>>(f, offset);
        });
        lambda(d);
      }
    });
  }
};

struct cm_t {
  struct instance_t {
    std::variant<
      model_loader_t::mark_t,
      model_loader_t::sprite_t
    > type;
  };

  template <typename T>
  struct id_t {
    using type_t = T;
    type_t internal_;
    std::shared_ptr<loco_t::cid_t> cid;
  };

  struct model_t {
    std::vector<id_t<
      std::variant<
      loco_t::sprite_t*
      #if defined(loco_rectangle)
      ,loco_t::rectangle_t*
      #endif
      #if defined(loco_button)
      , loco_t::button_t*
      #endif
      >
    >> cids;
    std::unordered_map<std::string, instance_t> instances;

    f32_t angle = 0;
  };

  std::unordered_map<uint32_t, model_t> groups;

  void import_from(const char* path, loco_t::texturepack_t* tp) {
    model_loader_t loader;
    loco_t::texturepack_t::ti_t ti;
    loader.load(tp, path, [&](const auto& data) {
      groups[data.group_id].instances[data.id].type = data;
    });
  }

  fan::vec3 position = 0;
  f32_t angle = 0;
}; 

struct model_list_t {

  using model_id_t = uint64_t;
  std::unordered_map<model_id_t, cm_t*> model_list;


  model_id_t push_model(loco_t::texturepack_t* tp, cm_t* cms) {
    model_id_t model_id = (model_id_t)cms;
    model_list[model_id] = cms;

    iterate(model_id, [&]<typename T>(auto group_id, auto shape_id, const T& properties) {
      if constexpr (std::is_same_v<T, model_loader_t::sprite_t>) {
        loco_t::sprite_t::properties_t p;
        p.camera = &pile->camera;
        p.viewport = &pile->viewport;
        p.position = properties.position;
        p.size = properties.size;
        loco_t::texturepack_t::ti_t ti;
        if (ti.qti(tp, properties.texturepack_name)) {
          fan::throw_error("invalid textureapack name", properties.texturepack_name);
        }
        p.load_tp(&ti);
        model_list[model_id]->position = p.position;
        push_shape(model_id, group_id, p);
      }
    });

    return model_id;
  }
  void erase(model_id_t id, uint32_t group_id) {
    auto& cids = model_list[id]->groups[group_id].cids;
    for (auto& i : cids) {
      loco_var.erase_shape(i.cid.get());
    }
    model_list[id]->groups.erase(group_id);
  }

  void erase(model_id_t id) {
    auto& groups = model_list[id]->groups;
    for (auto it = groups.begin(); it != groups.end(); ) {
      auto& cids = it->second.cids;
      for (auto& i : cids) {
        loco_var.erase_shape(i.cid.get());
      }
      it = groups.erase(it);
    }
  }

  loco_t::cid_t* push_shape(model_id_t model_id, uint32_t group_id, const auto& properties) {
    auto& cids = model_list[model_id]->groups[group_id].cids;
    typename std::remove_reference_t<decltype(cids)>::value_type p;
    p.internal_ = (std::remove_const_t<std::remove_reference_t<decltype(properties)>>::type_t*)0;
    p.cid = std::make_shared<loco_t::cid_t>();
    cids.emplace_back(p);
    loco_var.push_shape(cids.back().cid.get(), properties);
    return cids.back().cid.get();
  }

  void iterate(model_id_t model_id, auto lambda) {
    for (auto& it2 : model_list[model_id]->groups) {
      for (auto& i : it2.second.instances) {
        std::visit([&](auto&& o) {
          lambda(it2.first, i.first, o);
          }, i.second.type);
      }
    }
  }

  void iterate(model_id_t model_id, uint32_t group_id, auto lambda) {
    auto it = model_list[model_id]->groups.find(group_id);
    if (it == model_list[model_id]->groups.end()) {
      fan::throw_error("model iterate - invalid group_id");
    }
    for (auto& i : it->second.instances) {
      std::visit([&](auto&& o) {
        lambda(i.first, o);
        }, i.second.type);
    }
  }

  void iterate_cids(model_id_t model_id, auto lambda, fan::function_t<void(cm_t::model_t&)> group_lambda = [](cm_t::model_t&){}) {
    for (auto& it : model_list[model_id]->groups) {
      iterate_cids(model_id, it.first, lambda, group_lambda);
    }
  }

  void iterate_cids(model_id_t model_id, uint32_t group_id, auto lambda, fan::function_t<void(cm_t::model_t&)> group_lambda = [](cm_t::model_t&) {}) {
    auto& group = model_list[model_id]->groups[group_id];
    for (auto j : model_list[model_id]->groups[group_id].cids) {
      std::visit([&](auto&& o) {
        using shape_t = std::remove_pointer_t<std::remove_reference_t<decltype(o)>>;
        lambda.template operator() < shape_t > (pile->loco.get_shape<shape_t>(), j, group);
      }, j.internal_);
    }
    group_lambda(group);
  }

  void set_position(model_id_t model_id, const fan::vec3& position) {
    iterate_cids(model_id, [&]<typename shape_t>(auto* shape, auto& object, auto& model_info) {
      auto offset = position - model_list[model_id]->position;
      auto current = shape->get(object.cid.get(), &shape_t::vi_t::position);
      shape->set(object.cid.get(), &shape_t::vi_t::position, current + offset);
      shape->set(object.cid.get(), &shape_t::vi_t::rotation_point, current - position);
    });
    model_list[model_id]->position = position;
  }

  void set_angle(model_id_t model_id, f32_t angle) {
    iterate_cids(model_id, 
      // iterate per object in group x
      [&]<typename shape_t>(auto* shape, auto& object, auto& model_info) {
      auto offset = angle - model_info.angle;
      auto current = shape->get(object.cid.get(), &shape_t::vi_t::angle);
      shape->set(object.cid.get(), &shape_t::vi_t::angle, current + offset);
    },
    // iterate group
    [&](auto& model_info) {
      model_info.angle = angle;
    }
    );
  }
  void set_angle(model_id_t model_id, uint32_t group_id, f32_t angle) {
    iterate_cids(model_id,
      group_id,
      // iterate per object in group x
      [&]<typename shape_t>(auto * shape, auto & object, auto & model_info) {
      auto offset = angle - model_info.angle;
      auto current = shape->get(object.cid.get(), &shape_t::vi_t::angle);
      shape->set(object.cid.get(), &shape_t::vi_t::angle, current + offset);
    },
      // iterate group
      [&](auto& model_info) {
      model_info.angle = angle;
    }
    );
  }

  //void set_angle(model_id_t model_id, f32_t angle) {
  //  for (auto& it : model_list[model_id]->groups) {
  //    for (auto j : it.second.cids) {
  //      std::visit([&](auto&& o) {
  //        using shape_t = std::remove_pointer_t<std::remove_reference_t<decltype(o)>>;
  //      pile->loco.get_shape<shape_t>()->set(j.cid.get(), &shape_t::vi_t::angle, angle);
  //        }, j.internal_);
  //    }
  //  }
  //}
};

#undef loco_var