#pragma once

#include <fan/graphics/opengl/gl_core.h>

namespace fan {
	namespace opengl {

		struct viewport_t {

			viewport_t() = default;

			struct properties_t {
				fan::vec2 position;
				fan::vec2 size;
			};

			void open(fan::opengl::context_t* context) {

			}

			void close(fan::opengl::context_t* context) {
				context->disable_draw(m_context_step_id);
				m_context_step_id = fan::uninitialized;
			}

			void set(fan::opengl::context_t* context, const properties_t& properties) {
				m_position = properties.position;
				m_size = properties.size;
			}

			void enable(fan::opengl::context_t* context) {
				m_context_step_id = context->enable_draw(this, [](fan::opengl::context_t* c, void* d) { ((decltype(this))d)->step(c); });
			}
			void disable(fan::opengl::context_t* context) {
			#if fan_debug >= fan_debug_low
				if (m_context_step_id == fan::uninitialized) {
					fan::throw_error("trying to disable unenabled draw call");
				}
			#endif
				context->disable_draw(m_context_step_id);
				m_context_step_id = fan::uninitialized;
			}

			// pushed to window draw queue
			void step(fan::opengl::context_t* context) {
				context->opengl.call(context->opengl.glViewport, m_position.x, m_position.y, m_size.x, m_size.y);
			}

			fan::vec2 m_position;
			fan::vec2 m_size;

			uint32_t m_context_step_id;
		};

	}
}