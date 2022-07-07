#pragma once

#include _FAN_PATH(graphics/graphics.h)

namespace fan_2d {
  namespace graphics {

    template <typename T_user_global_data, typename T_user_instance_data>
    struct letter_t {

      using user_global_data_t = T_user_global_data;
      using user_instance_data_t = T_user_instance_data;

      using move_cb_t = void(*)(letter_t*, uint32_t src, uint32_t dst, user_instance_data_t*);

      struct properties_t {
        f32_t font_size = 16;
        fan::vec2 position = 0;
        fan::color color = fan::colors::white;
        uint16_t letter_id;
        user_instance_data_t data;
      };

      struct instance_t {
        fan::vec2 position;
        fan::vec2 size;
        fan::color color;
        fan::vec2 tc_position;
        fan::vec2 tc_size;
      };

      static constexpr uint32_t max_instance_size = 256;

      struct id_t{
        id_t(uint32_t id) {
          block = id / max_instance_size;
          instance = id % max_instance_size;
        }
        uint32_t block;
        uint32_t instance;
      };

      void open(fan::opengl::context_t* context, fan_2d::graphics::font_t* font_, move_cb_t move_cb_, const user_global_data_t& gd) {

        m_shader.open(context);

        m_shader.set_vertex(
          context,
#include _FAN_PATH(graphics/glsl/opengl/2D/objects/letter.vs)
        );

        m_shader.set_fragment(
          context,
#include _FAN_PATH(graphics/glsl/opengl/2D/objects/letter.fs)
        );

        m_shader.compile(context);

        blocks.open();

        m_draw_node_reference = fan::uninitialized;

        font = font_;
        user_global_data = gd;
        move_cb = move_cb_;
      }
      void close(fan::opengl::context_t* context) {
        m_shader.close(context);
        for (uint32_t i = 0; i < blocks.size(); i++) {
          blocks[i].uniform_buffer.close(context);
        }
        blocks.close();
      }

      uint32_t push_back(fan::opengl::context_t* context, const properties_t& p) {
        instance_t it;
        //   it.offset = 0;
        it.position = p.position;
        it.color = p.color;

        fan::font::single_info_t si = font->info.get_letter_info(p.letter_id, p.font_size);

        it.tc_position = si.glyph.position / font->image.size;
        it.tc_size.x = si.glyph.size.x / font->image.size.x;
        it.tc_size.y = si.glyph.size.y / font->image.size.y;

        it.size = si.metrics.size / 2;

        uint32_t i = 0;

        for (; i < blocks.size(); i++) {
          if (blocks[i].uniform_buffer.size() != max_instance_size) {
            break;
          }
        }

        if (i == blocks.size()) {
          blocks.push_back({});
          blocks[i].uniform_buffer.open(context);
          blocks[i].uniform_buffer.bind_uniform_block(context, m_shader.id, "instance_t");
        }

        blocks[i].uniform_buffer.push_ram_instance(context, it);
        blocks[i].user_instance_data[blocks[i].uniform_buffer.size() - 1] = p.data;

        uint32_t src = blocks[i].uniform_buffer.size() % max_instance_size;

        blocks[i].uniform_buffer.common.edit(
          context,
          src - 1,
          std::min(src, max_instance_size)
        );

        return i * max_instance_size + (blocks[i].uniform_buffer.size() - 1);
      }
      void erase(fan::opengl::context_t* context, uint32_t id) {
        
        uint32_t block_id = id / max_instance_size;
        uint32_t instance_id = id % max_instance_size;

        if (block_id == blocks.size() - 1 && instance_id == blocks.ge()->uniform_buffer.size() - 1) {
          blocks[block_id].uniform_buffer.common.m_size -= blocks[block_id].uniform_buffer.common.buffer_bytes_size;
          if (blocks[block_id].uniform_buffer.size() == 0) {
            blocks[block_id].uniform_buffer.close(context);
            blocks.m_size -= 1;
          }
          return;
        }

        uint32_t last_block_id = blocks.size() - 1;
        uint32_t last_instance_id = blocks[last_block_id].uniform_buffer.size() - 1;

        instance_t* data = blocks[block_id].uniform_buffer.get_instance(context, last_instance_id);

        blocks[block_id].uniform_buffer.edit_ram_instance(
          context,
          instance_id,
          data,
          0,
          sizeof(instance_t)
        );

        blocks[block_id].uniform_buffer.common.edit(
          context,
          instance_id,
          instance_id + 1
        );

        blocks[last_block_id].uniform_buffer.common.m_size -= blocks[block_id].uniform_buffer.common.buffer_bytes_size;

        blocks[block_id].user_instance_data[instance_id] = blocks[last_block_id].user_instance_data[last_instance_id];

        if (blocks[last_block_id].uniform_buffer.size() == 0) {
          blocks[last_block_id].uniform_buffer.close(context);
          blocks.m_size -= 1;
        }


        move_cb(
          this,
          last_instance_id + last_block_id * max_instance_size,
          id,
          &blocks[block_id].user_instance_data[instance_id]
        );
      }

      void enable_draw(fan::opengl::context_t* context) {
        this->draw(context);

#if fan_debug >= fan_debug_low
        if (m_draw_node_reference != fan::uninitialized) {
          fan::throw_error("trying to call enable_draw twice");
        }
#endif

        m_draw_node_reference = context->enable_draw(this, [](fan::opengl::context_t* c, void* d) { ((decltype(this))d)->draw(c); });
      }
      void disable_draw(fan::opengl::context_t* context) {
#if fan_debug >= fan_debug_low
        if (m_draw_node_reference == fan::uninitialized) {
          fan::throw_error("trying to disable unenabled draw call");
        }
#endif
        context->disable_draw(m_draw_node_reference);
      }

      void draw(fan::opengl::context_t* context) {
        m_shader.use(context);

        m_shader.set_int(context, "texture_sampler", 0);
        context->opengl.glActiveTexture(fan::opengl::GL_TEXTURE0);
        context->opengl.glBindTexture(fan::opengl::GL_TEXTURE_2D, font->image.texture);

        for (uint32_t i = 0; i < blocks.size(); i++) {
          blocks[i].uniform_buffer.bind_buffer_range(context, blocks[i].uniform_buffer.size());

          blocks[i].uniform_buffer.draw(
            context,
            0,
            blocks[i].uniform_buffer.size() * 6
          );
        }
      }

      void bind_matrices(fan::opengl::context_t* context, fan::opengl::matrices_t* matrices) {
        m_shader.bind_matrices(context, matrices);
      }

      template <typename T>
      T get(fan::opengl::context_t* context, const id_t& id, T instance_t::*member) {
        uint32_t block_id = id / max_instance_size;
        uint32_t instance_id = id % max_instance_size;
        return blocks[block_id].uniform_buffer.get_instance(context, instance_id)->*member;
      }
      template <typename T>
      void set(fan::opengl::context_t* context, const id_t& id, T instance_t::*member, const T& value) {
        uint32_t block_id = id / max_instance_size;
        uint32_t instance_id = id % max_instance_size;
        blocks[block_id].uniform_buffer.edit_ram_instance(context, instance_id, &value, fan::ofof<instance_t, T>(member), sizeof(T));
      }

      void set_user_instance_data(fan::opengl::context_t* context, const id_t& id, const user_instance_data_t& user_instance_data) {
        blocks[id.block].user_instance_data[id.instance] = user_instance_data;
      }

      fan::shader_t m_shader;
      struct block_t {
        fan::opengl::core::uniform_block_t<instance_t, max_instance_size> uniform_buffer;
        user_instance_data_t user_instance_data[max_instance_size];
      };
      uint32_t m_draw_node_reference;

      fan::hector_t<block_t> blocks;

      fan_2d::graphics::font_t* font;
      user_global_data_t user_global_data;
      move_cb_t move_cb;
    };
  }
}