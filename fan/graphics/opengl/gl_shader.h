struct shader_t {

  shader_list_NodeData_t& get_shader() const {
    return gloco->shader_list[shader_reference];
  }
  shader_list_NodeData_t& get_shader() {
    return gloco->shader_list[shader_reference];
  }

  shader_list_NodeReference_t shader_reference;

  void open() {
    shader_reference = gloco->shader_list.NewNode();
    auto& shader = get_shader();
    shader.id = fan::uninitialized;
    shader.shader = this;

    shader.vertex = fan::uninitialized;
    shader.fragment = fan::uninitialized;
  }

  void close() {
    auto& context = gloco->get_context();
    this->remove();
    gloco->shader_list.Recycle(shader_reference);
  }

  shader_t() = default;

  shader_t(shader_t&& shader) {
    shader_reference = shader.shader_reference;
    gloco->shader_list[shader_reference].shader = this;
    shader.shader_reference.sic();
  }
  shader_t(const shader_t& shader) {
    open();
    gloco->shader_list[shader_reference].on_activate = gloco->shader_list[shader.shader_reference].on_activate;
  }

  shader_t& operator=(const shader_t& s) {
    if (this != &s) {
      open();
      gloco->shader_list[shader_reference].on_activate = gloco->shader_list[s.shader_reference].on_activate;
    }
    return *this;
  }
  shader_t& operator=(shader_t&& s) {
    if (this != &s) {
      if (!shader_reference.iic()) {
        close();
      }
      shader_reference = s.shader_reference;
      gloco->shader_list[shader_reference].shader = this;
      gloco->shader_list[shader_reference].on_activate = gloco->shader_list[s.shader_reference].on_activate;
      s.shader_reference.sic();
    }
    return *this;
  }
  ~shader_t() {
    if (shader_reference.iic()) {
      return;
    }
    close();
  }

  void use() const {
    auto& context = gloco->get_context();
    const auto& shader = get_shader();
    if (shader.id == context.current_program) {
      return;
    }
    context.opengl.call(context.opengl.glUseProgram, shader.id);
    context.current_program = shader.id;
  }

  void remove() {
    auto& context = gloco->get_context();
    auto& shader = get_shader();
    fan_validate_buffer(shader.id, {
        context.opengl.call(context.opengl.glValidateProgram, shader.id);
        int status = 0;
        context.opengl.call(context.opengl.glGetProgramiv, shader.id, fan::opengl::GL_VALIDATE_STATUS, &status);
        if (status) {
            context.opengl.call(context.opengl.glDeleteProgram, shader.id);
        }
        shader.id = fan::uninitialized;
      });
  }

  void set_vertex(char* vertex_ptr, fan::opengl::GLint length) {
    auto& context = gloco->get_context();
    auto& shader = get_shader();

    if (shader.vertex != fan::uninitialized) {
      context.opengl.call(context.opengl.glDeleteShader, shader.vertex);
    }

    shader.vertex = context.opengl.call(context.opengl.glCreateShader, fan::opengl::GL_VERTEX_SHADER);

    context.opengl.call(context.opengl.glShaderSource, shader.vertex, 1, &vertex_ptr, &length);

    context.opengl.call(context.opengl.glCompileShader, shader.vertex);

    checkCompileErrors(context, shader.vertex, "VERTEX");
  }

  void set_vertex(const fan::string& vertex_code) {
    auto& context = gloco->get_context();
    auto& shader = get_shader();

    if (shader.vertex != fan::uninitialized) {
      context.opengl.call(context.opengl.glDeleteShader, shader.vertex);
    }

    shader.vertex = context.opengl.call(context.opengl.glCreateShader, fan::opengl::GL_VERTEX_SHADER);

    char* ptr = (char*)vertex_code.c_str();
    fan::opengl::GLint length = vertex_code.size();

    context.opengl.call(context.opengl.glShaderSource, shader.vertex, 1, &ptr, &length);
    context.opengl.call(context.opengl.glCompileShader, shader.vertex);

    checkCompileErrors(context, shader.vertex, "VERTEX");
  }

  void set_fragment(char* fragment_ptr, fan::opengl::GLint length) {
    auto& context = gloco->get_context();
    auto& shader = get_shader();

    if (shader.fragment != -1) {
      context.opengl.glDeleteShader(shader.fragment);
    }

    shader.fragment = context.opengl.glCreateShader(fan::opengl::GL_FRAGMENT_SHADER);
    context.opengl.glShaderSource(shader.fragment, 1, &fragment_ptr, &length);

    context.opengl.glCompileShader(shader.fragment);
    checkCompileErrors(context, shader.fragment, "FRAGMENT");
  }

  void set_fragment(const fan::string& fragment_code) {
    auto& context = gloco->get_context();
    auto& shader = get_shader();

    if (shader.fragment != -1) {
      context.opengl.call(context.opengl.glDeleteShader, shader.fragment);
    }

    shader.fragment = context.opengl.call(context.opengl.glCreateShader, fan::opengl::GL_FRAGMENT_SHADER);

    char* ptr = (char*)fragment_code.c_str();
    fan::opengl::GLint length = fragment_code.size();

    context.opengl.call(context.opengl.glShaderSource, shader.fragment, 1, &ptr, &length);

    context.opengl.call(context.opengl.glCompileShader, shader.fragment);
    checkCompileErrors(context, shader.fragment, "FRAGMENT");
  }

  bool compile() {
    auto& context = gloco->get_context();
    auto& shader = get_shader();

    auto temp_id = context.opengl.call(context.opengl.glCreateProgram);
    if (shader.vertex != -1) {
      context.opengl.call(context.opengl.glAttachShader, temp_id, shader.vertex);
    }
    if (shader.fragment != -1) {
      context.opengl.call(context.opengl.glAttachShader, temp_id, shader.fragment);
    }

    context.opengl.call(context.opengl.glLinkProgram, temp_id);
    bool ret = checkCompileErrors(context, temp_id, "PROGRAM");

    if (shader.vertex != -1) {
      context.opengl.call(context.opengl.glDeleteShader, shader.vertex);
      shader.vertex = -1;
    }
    if (shader.fragment != -1) {
      context.opengl.call(context.opengl.glDeleteShader, shader.fragment);
      shader.fragment = -1;
    }

    if (ret == false) {
      return ret;
    }

    if (shader.id != -1) {
      context.opengl.call(context.opengl.glDeleteProgram, shader.id);
    }
    shader.id = temp_id;

    shader.projection_view[0] = context.opengl.call(context.opengl.glGetUniformLocation, shader.id, "projection");
    shader.projection_view[1] = context.opengl.call(context.opengl.glGetUniformLocation, shader.id, "view");
    return ret;
  }


  static constexpr auto validate_error_message = [](const auto str) {
    return "failed to set value for:" + str + " check if variable is used in file so that its not optimized away";
  };

  void set_bool(const fan::string& name, bool value) const {
    use();
    auto& context = gloco->get_context();
    set_int(name, value);
  }

  void set_int(const fan::string& name, int value) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    #if fan_debug >= fan_debug_insanity
    fan_validate_value(location, validate_error_message(name));
    #endif
    context.opengl.call(context.opengl.glUniform1i, location, value);
  }

  void set_uint(const fan::string& name, uint32_t value) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    #if fan_debug >= fan_debug_insanity
    fan_validate_value(location, validate_error_message(name));
    #endif
    context.opengl.call(context.opengl.glUniform1ui, location, value);
  }

  void set_int_array(const fan::string& name, int* values, int size) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    #if fan_debug >= fan_debug_insanity
    fan_validate_value(location, validate_error_message(name));
    #endif
    context.opengl.call(context.opengl.glUniform1iv, location, size, values);
}

  void set_uint_array(const fan::string& name, uint32_t* values, int size) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    #if fan_debug >= fan_debug_insanity
    fan_validate_value(location, validate_error_message(name));
    #endif
    context.opengl.call(context.opengl.glUniform1uiv, location, size, values);
  }

  void set_float_array(const fan::string& name, f32_t* values, int size) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    #if fan_debug >= fan_debug_insanity
    fan_validate_value(location, validate_error_message(name));
    #endif
    context.opengl.call(context.opengl.glUniform1fv, location, size, values);
  }

  void set_float(const fan::string& name, fan::vec2::value_type value) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    #if fan_debug >= fan_debug_insanity
    fan_validate_value(location, validate_error_message(name));
    #endif
    if constexpr (std::is_same<fan::vec2::value_type, f32_t>::value) {
      context.opengl.call(context.opengl.glUniform1f, location, value);
    }
    else {
      context.opengl.call(context.opengl.glUniform1d, location, value);
    }
}

  void set_vec2(const fan::string& name, const fan::vec2& value) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    #if fan_debug >= fan_debug_insanity
    fan_validate_value(location, validate_error_message(name));
    #endif
    if constexpr (std::is_same<fan::vec2::value_type, f32_t>::value) {
      context.opengl.call(context.opengl.glUniform2fv, location, 1, (f32_t*)&value.x);
    }
    else {
      context.opengl.call(context.opengl.glUniform2dv, location, 1, (f64_t*)&value.x);
    }
  }


  void set_vec2_array(const fan::string& name, std::vector<fan::vec2>& values) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    #if fan_debug >= fan_debug_insanity
    fan_validate_value(location, validate_error_message(name));
    #endif
    if constexpr (std::is_same<fan::vec2::value_type, f32_t>::value) {
      context.opengl.call(context.opengl.glUniform2fv, location, values.size(), (f32_t*)&(*values.data())[0]);
    }
    else {
      context.opengl.call(context.opengl.glUniform2dv, location, values.size(), (f64_t*)&(*values.data())[0]);
    }
  }


  void set_vec2(const fan::string& name, f32_t x, f32_t y) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    #if fan_debug >= fan_debug_insanity
    fan_validate_value(location, validate_error_message(name));
    #endif
    if constexpr (std::is_same<fan::vec2::value_type, f32_t>::value) {
      context.opengl.call(context.opengl.glUniform2f, location, x, y);
  }
    else {
      context.opengl.call(context.opengl.glUniform2d, location, x, y);
    }
}

  void set_vec3(const fan::string& name, const fan::vec3& value) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    #if fan_debug >= fan_debug_insanity
    fan_validate_value(location, validate_error_message(name));
    #endif
    if constexpr (std::is_same<fan::vec3::value_type, float>::value) {
      context.opengl.call(context.opengl.glUniform3f, location, value.x, value.y, value.z);
    }
    else {
      context.opengl.call(context.opengl.glUniform3d, location, value.x, value.y, value.z);
    }
  }

  void set_vec4(const fan::string& name, const fan::color& color) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    #if fan_debug >= fan_debug_insanity
    fan_validate_value(location, validate_error_message(name));
    #endif
    if constexpr (std::is_same<fan::vec4::value_type, float>::value) {
      context.opengl.call(context.opengl.glUniform4f, location, color.r, color.g, color.b, color.a);
    }
    else {
      context.opengl.call(context.opengl.glUniform4d, location, color.r, color.g, color.b, color.a);
    }
  }

  void set_vec4(const fan::string& name, f32_t x, f32_t y, f32_t z, f32_t w) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    #if fan_debug >= fan_debug_insanity
    fan_validate_value(location, validate_error_message(name));
    #endif
    if constexpr (std::is_same<fan::vec4::value_type, float>::value) {
      context.opengl.call(context.opengl.glUniform4f, location, x, y, z, w);
    }
    else {
      context.opengl.call(context.opengl.glUniform4d, location, x, y, z, w);
    }
  }

  void set_camera(auto* camera, auto write_queue, uint32_t flags = 0) {
    auto& context = gloco->get_context();
    context.opengl.call(context.opengl.glUniformMatrix4fv, get_shader().projection_view[0], 1, fan::opengl::GL_FALSE, &camera->m_projection[0][0]);
    context.opengl.call(context.opengl.glUniformMatrix4fv, get_shader().projection_view[1], 1, fan::opengl::GL_FALSE, &camera->m_view[0][0]);
  }

  void set_mat4(const fan::string& name, fan::mat4 mat) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    #if fan_debug >= fan_debug_insanity
    fan_validate_value(location, validate_error_message(name));
    #endif
    if constexpr (std::is_same<fan::mat4::value_type::value_type, float>::value) {
      context.opengl.call(context.opengl.glUniformMatrix4fv, location, 1, fan::opengl::GL_FALSE, (f32_t*)&mat[0][0]);
    }
    else {
      context.opengl.call(context.opengl.glUniformMatrix4dv, location, 1, fan::opengl::GL_FALSE, (f64_t*)&mat[0][0]);
    }
  }

  void set_mat4(const fan::string& name, f32_t* value, uint32_t count) const {
    use();
    auto& context = gloco->get_context();
    auto location = context.opengl.call(context.opengl.glGetUniformLocation, get_shader().id, name.c_str());
    fan_validate_value(location, validate_error_message(name));
    if constexpr (std::is_same<fan::mat4::value_type::value_type, float>::value) {
      context.opengl.call(context.opengl.glUniformMatrix4fv, location, count, fan::opengl::GL_FALSE, value);
  }
    else {
      context.opengl.call(context.opengl.glUniformMatrix4dv, location, count, fan::opengl::GL_FALSE, (f64_t*)value);
    }
  }

private:

  bool checkCompileErrors(fan::opengl::context_t& context, fan::opengl::GLuint shader, fan::string type)
  {
    fan::opengl::GLint success;

    bool program = type == "PROGRAM";

    if (program == false) {
      context.opengl.call(context.opengl.glGetShaderiv, shader, fan::opengl::GL_COMPILE_STATUS, &success);
    }
    else {
      context.opengl.call(context.opengl.glGetProgramiv, shader, fan::opengl::GL_LINK_STATUS, &success);
    }

    if (success) {
      return true;
    }

    int buffer_size = 0;
    context.opengl.glGetShaderiv(shader, fan::opengl::GL_INFO_LOG_LENGTH, &buffer_size);


    if (buffer_size <= 0) {
      return false;
    }

    fan::string buffer;
    buffer.resize(buffer_size);

    if (!success)
    {
      int test;
#define get_info_log(is_program, program, str_buffer, size) \
            if (is_program) \
            context.opengl.call(context.opengl.glGetProgramInfoLog, program, size, nullptr, buffer.data()); \
            else \
            context.opengl.call(context.opengl.glGetShaderInfoLog, program, size, &test, buffer.data());

      get_info_log(program, shader, buffer, buffer_size);
          
      fan::print("failed to compile type: " + type, buffer);

      return false;
    }
    return true;
  }
};

static fan::string read_shader(const fan::string& path) {
  fan::string code;
  fan::io::file::read(path, &code);
  return code;
}