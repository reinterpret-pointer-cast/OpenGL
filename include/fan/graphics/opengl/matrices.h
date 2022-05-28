#pragma once

#include _FAN_PATH(bll.h)
#include _FAN_PATH(graphics/opengl/gl_core.h)

namespace fan {
	namespace opengl {

		struct matrices_t {

      typedef void(*inform_cb_t)(matrices_t*, void* updateptr, void* userptr);

      struct inform_data_t {
        inform_cb_t cb;
        void* userptr;
      };

      void open() {
        m_view = fan::mat4(1);
        camera_position = 0;
        m_inform_data_list.open();
      }
      void close() {
        m_inform_data_list.close();
      }

      fan::vec3 get_camera_position() const {
        return camera_position;
      }
      void set_camera_position(void* updateptr, const fan::vec3& cp) {
        camera_position = cp;

        m_view[3][0] = 0;
        m_view[3][1] = 0;
        m_view[3][2] = 0;
        m_view = m_view.translate(camera_position);
        fan::vec3 position = m_view.get_translation();
        constexpr fan::vec3 front(0, 0, 1);

        m_view = fan::math::look_at_left<fan::mat4>(position, position + front, fan::camera::world_up);

        uint32_t it = m_inform_data_list.begin();
        while (it != m_inform_data_list.end()) {
          m_inform_data_list.start_safe_next(it);

          m_inform_data_list[it].cb(this, updateptr, m_inform_data_list[it].userptr);

          it = m_inform_data_list.end_safe_next();
        }
      }

			void set_ortho(void* updateptr, const fan::vec2& x, const fan::vec2& y) {
        m_projection = fan::math::ortho<fan::mat4>(
          x.x,
          x.y,
          y.y,
          y.x,
          0.1,
          100.0
        );

        m_view[3][0] = 0;
        m_view[3][1] = 0;
        m_view[3][2] = 0;
        m_view = m_view.translate(camera_position);
        fan::vec3 position = m_view.get_translation();
        constexpr fan::vec3 front(0, 0, 1);

        m_view = fan::math::look_at_left<fan::mat4>(position, position + front, fan::camera::world_up);

        uint32_t it = m_inform_data_list.begin();

        while (it != m_inform_data_list.end()) {
          m_inform_data_list.start_safe_next(it);

          m_inform_data_list[it].cb(this, updateptr, m_inform_data_list[it].userptr);

          it = m_inform_data_list.end_safe_next();
        }
      }

      uint32_t push_inform(inform_cb_t cb, void* userptr) {
        inform_data_t data;
        data.cb = cb;
        data.userptr = userptr;
        return m_inform_data_list.push_back(data);
      }
      void erase_inform(uint32_t id) {
        m_inform_data_list.erase(id);
      }

      bll_t<inform_data_t> m_inform_data_list;

			fan::mat4 m_projection;
      // temporary
      fan::mat4 m_view;

      fan::vec3 camera_position;
		};

	}
}