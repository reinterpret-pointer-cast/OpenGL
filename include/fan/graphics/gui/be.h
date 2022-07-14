#pragma once

#include _FAN_PATH(graphics/gui/key_event.h)
#include _FAN_PATH(physics/collision/circle.h)

namespace fan_2d {
  namespace graphics {
    namespace gui {

      enum class mouse_stage {
        outside,
        inside,
        outside_drag,
        inside_drag // when dragged from other element and released inside other element
      };

      struct be_t {

        //using user_global_data_t = T_user_global_data;

        typedef void(*on_input_cb)(fan::opengl::context_t* context, be_t*, uint32_t element_id, uint16_t key, fan::key_state key_state, fan_2d::graphics::gui::mouse_stage mouse_stage, void* userptr);
        typedef void(*on_mouse_move_cb)(fan::opengl::context_t* context, be_t*, uint32_t element_id, mouse_stage mouse_stage, void* userptr);

        struct hitbox_type_t {
          static constexpr uint8_t rectangle = 0;
          static constexpr uint8_t circle = 1;
        };

        struct properties_t {

          using type_t = be_t;

          on_input_cb on_input_function;
          on_mouse_move_cb on_mouse_event_function;

          void* userptr;
          uint32_t element_id;

          uint32_t depth = 0;

          uint8_t hitbox_type;
          union {
            struct{
              fan::vec2 position;
              fan::vec2 size;
            }hitbox_rectangle;
            struct{
              fan::vec2 position;
              f32_t radius;
            }hitbox_circle;
          };
        };

        struct key_info_t {
          be_t* be;
          uint32_t index;
          uint16_t key;
          fan::key_state key_state;
          fan_2d::graphics::gui::mouse_stage mouse_stage;
        };

        void open() {
          m_button_data.open();
          coordinate_offset = 0;
          m_old_mouse_stage = fan::uninitialized;
          m_do_we_hold_button = 0;
          m_focused_button_id = fan::uninitialized;
        }
        void close() {
          m_button_data.close();
        }

        auto inside(uint32_t i, const fan::vec2& p) {

          switch(m_button_data[i].properties.hitbox_type) {
            case hitbox_type_t::rectangle: {
              return fan_2d::collision::rectangle::point_inside_no_rotation(
                p, 
                m_button_data[i].properties.hitbox_rectangle.position - m_button_data[i].properties.hitbox_rectangle.size, 
                m_button_data[i].properties.hitbox_rectangle.position + m_button_data[i].properties.hitbox_rectangle.size
              );
            }
            case hitbox_type_t::circle: {
              return fan_2d::collision::circle::point_inside(
                p, 
                m_button_data[i].properties.hitbox_circle.position,
                m_button_data[i].properties.hitbox_circle.radius
              );
            }
          }
        };

        void feed_mouse_move(fan::opengl::context_t* context, const fan::vec2& mouse_position, uint32_t depth) {
          if (m_do_we_hold_button == 1) {
            return;
          }
          if (m_focused_button_id != fan::uninitialized) {
            if (inside(m_focused_button_id, coordinate_offset + mouse_position)) {
              return;
            }
          }


          uint32_t i = m_button_data.rbegin();
          while (i != m_button_data.rend()) {
            if (m_button_data[i].properties.depth != depth) {
              continue;
            }
            if (inside(i, coordinate_offset + mouse_position)) {
              if (m_focused_button_id != fan::uninitialized) {
                m_button_data[i].properties.on_mouse_event_function(context, this, m_button_data[i].properties.element_id, mouse_stage::outside, m_button_data[i].properties.userptr);
              }
              m_focused_button_id = i;
              m_button_data[i].properties.on_mouse_event_function(context, this, m_button_data[m_focused_button_id].properties.element_id, mouse_stage::inside, m_button_data[i].properties.userptr);
              return;
            }
            i = m_button_data.rnext(i);
          }
          if (m_focused_button_id != fan::uninitialized) {
            m_button_data[m_focused_button_id].properties.on_mouse_event_function(context, this, m_button_data[m_focused_button_id].properties.element_id, mouse_stage::outside, m_button_data[m_focused_button_id].properties.userptr);
            m_focused_button_id = fan::uninitialized;
          }
        }

        void feed_mouse_input(fan::opengl::context_t* context, uint16_t button, fan::key_state state, const fan::vec2& mouse_position, uint32_t depth) {
          if (m_do_we_hold_button == 0) {
            if (state == fan::key_state::press) {
              if (m_focused_button_id != fan::uninitialized) {
                m_do_we_hold_button = 1;
                m_button_data[m_focused_button_id].properties.on_input_function(context, this, m_button_data[m_focused_button_id].properties.element_id, button, fan::key_state::press, mouse_stage::inside, m_button_data[m_focused_button_id].properties.userptr);
              }
              else {
                uint32_t i = m_button_data.rbegin();
                while (i != m_button_data.rend()) {
                  if (m_button_data[i].properties.depth != depth) {
                    continue;
                  }
                  if (inside(i, coordinate_offset + mouse_position)) {
                    m_button_data[i].properties.on_input_function(context, this, m_button_data[i].properties.element_id, button, state, mouse_stage::outside, m_button_data[i].properties.userptr);
                  }
                  i = m_button_data.rnext(i);
                }
                return; // clicked at space
              }
            }
            else {
              return;
            }
          }
          else {
            if (state == fan::key_state::press) {
              return; // double press
            }
            else {
              if (m_button_data[m_focused_button_id].properties.depth != depth) {
                return;
              }
              if (inside(m_focused_button_id, coordinate_offset + mouse_position)) {
                pointer_remove_flag = 1;
                m_button_data[m_focused_button_id].properties.on_input_function(context, this, m_button_data[m_focused_button_id].properties.element_id, button, fan::key_state::release, mouse_stage::inside, m_button_data[m_focused_button_id].properties.userptr);
                if (pointer_remove_flag == 0) {
                  return;
                  //rtb is deleted
                }
              }
              else {
                uint32_t i = m_button_data.rbegin();
                while (i != m_button_data.rend()) {
                  if (inside(i, coordinate_offset + mouse_position)) {
                    m_button_data[i].properties.on_input_function(context, this, m_button_data[i].properties.element_id, button, fan::key_state::release, mouse_stage::inside_drag, m_button_data[i].properties.userptr);
                    m_focused_button_id = i;
                    break;
                  }
                  i = m_button_data.rnext(i);
                }

                pointer_remove_flag = 1;
                m_button_data[m_focused_button_id].properties.on_input_function(context, this, m_button_data[m_focused_button_id].properties.element_id, button, fan::key_state::release, mouse_stage::outside, m_button_data[m_focused_button_id].properties.userptr);
                if (pointer_remove_flag == 0) {
                  return;
                }

                pointer_remove_flag = 0;
              }
              m_do_we_hold_button = 0;
            }
          }
        }

        uint32_t push_back(const properties_t& p) {
          button_data_t b;
          b.properties = p;
          return m_button_data.push_back(b);
        }
        void erase(uint32_t i, fan::opengl::cid_t* cid) {
          uint32_t src = m_button_data.size() - 1;
          uint32_t dst = i;
          m_button_data[i] = m_button_data[m_button_data.size() - 1];
          cid->id = i; // maybe
          //m_button_data.pop_back();
        }

        // used for camera position
        fan::vec2 get_coordinate_offset() const {
          return coordinate_offset;
        }
        void set_coordinate_offset(const fan::vec2& offset) {
          coordinate_offset = offset;
        }

        uint32_t size() const {
          return m_button_data.size();
        }

        void write_in(FILE* f) {
          m_button_data.write_in(f);
        }
        void write_out(FILE* f) {
          m_button_data.write_out(f);
        }

    //  protected:

        inline static thread_local bool pointer_remove_flag;

        uint8_t m_old_mouse_stage;
        bool m_do_we_hold_button;
        uint32_t m_focused_button_id;

        struct button_data_t {
          properties_t properties;
        };

        bll_t<button_data_t> m_button_data;

        fan::vec2 coordinate_offset;
      };
    }
  }
}