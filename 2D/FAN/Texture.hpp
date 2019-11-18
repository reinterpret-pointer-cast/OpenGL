#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../SOIL2/SOIL2.h"
#include <vector>
#include "../SOIL2/stb_image.h"

#include "Input.hpp"
#include "Settings.hpp"
#include "Math.hpp"
#include "Shader.h"
#include "Camera.hpp"
#include "Color.hpp"

#ifdef _MSC_VER
#pragma warning (disable : 26495)
#endif

#define BackgroundSize 1500

class Object;
class Texture;
class Sprite;
class Main;
enum class GroupId;

using namespace Settings;
using namespace WindowNamespace;
using namespace CursorNamespace;

constexpr float triangle_vertices[] = {
	-0.433, 0.25,
	 0.433, 0.25,
	 0.0,  -0.5f
};

constexpr float square_vertices[] = {
	0.5f, 0.5f,
	0.5f, -0.5f,
   -0.5f, -0.5f,
   -0.5f, 0.5f
};

void LoadImg(const char* path, Object& object, Texture& texture);

class Object {
public:
	unsigned int texture;
	int width, height;
	unsigned int VBO, VAO, EBO;
	Object() : texture(0), width(0), height(0), VBO(0), VAO(0), EBO(0) { }
};

class Texture {
public:
	Texture() : vertices{ 0 } { }
	void IntializeImage(Texture& texture);
	float vertices[32];
};

class Sprite {
public:
	Sprite() : object(), texture(), position(0) {}

	Sprite(const Sprite& info);

	Sprite(Object const& _object, Texture const& _texture, Vec2 const& _position);

	Sprite(const char* path);

	void SetPosition(const Vec2& position);

	void Draw(Main& _Main, const Vec3& rotation, float angle, const Vec2& scale);
protected:
	Object object;
	Texture texture;
	Vec2 position;
};

class Shape {
public:
	Shape() {}

	Shape(Camera* camera, const Vec2& position, const Vec2& pixelSize, const Color& color, std::vector<float> vec);

	~Shape();

	void Draw();

	void SetPosition(const Vec2& position);

	void SetColor(Color color);



protected:
	std::vector<float> vertices;
	Object object;
	Shader shader;
	Camera* camera;
	Vec2 position;
	Vec2 size;
	Color color;
	long long type;
	uint64_t points;
};

class Triangle : public Shape {
public:
	Triangle(Camera* camera, const Vec2& position, const Vec2& pixelSize, const Color& color) :
		Shape(camera, position, pixelSize, color, std::vector<float>(triangle_vertices, triangle_vertices + ArrLen(triangle_vertices))) {
		type = GL_TRIANGLES;
		points = 3;
	}
	void Add(const Vec2& position, Vec2 size = Vec2(), Color color = Color());
};

class Square : public Shape {
public:
	Square(Camera* camera, const Vec2& position, const Vec2& pixelSize, const Color& color) :
		Shape(camera, position, pixelSize, color, std::vector<float>(square_vertices, square_vertices + ArrLen(square_vertices))) {
		type = GL_QUADS;
		points = 4;
	}
};

class Line : public Shape {
public:
	Line() {};
	Line(Camera* camera, const Color& color = Color(255, 0, 0, 255)) : Shape(camera, Vec2(), Vec2(), color, std::vector<float>{}) {
		type = GL_LINES;
		points = 0;
	};
	Line(Camera* camera, const Mat2x2& begin_end, const Color& color) : 
		Shape(camera, Vec2(), Vec2(), color, std::vector<float> {
		begin_end.vec[0].x, begin_end.vec[0].y,
		begin_end.vec[1].x, begin_end.vec[1].y

	}){
		type = GL_LINES;
		points = 2;
	}

	void Add(const Mat2x2& begin_end);

	void SetPosition(size_t where, const Mat2x2& begin_end);
};

enum class GroupId {
	NotAssigned = -1,
	LocalPlayer = 0,
	Enemy = 1
};

class Entity : public Sprite {
public:
	Entity() : groupId(GroupId::NotAssigned) {}

	Entity(const char* path, GroupId _groupId);

	Entity(const char* path, Vec2 pos, GroupId _groupId);

	GroupId GetGroupId();

	void SetGroupId(GroupId groupId);

	void Move(Main& _Main);

private:
	GroupId groupId;
	Vec2 velocity;
	float health;
	float movementSpeed = 2000;
	float friction = 5;
};

class Main {
public:
	std::vector<Entity> entity;
	Shader shader = Shader("GLSL/core.vs", "GLSL/core.frag");
	Camera camera;
	GLFWwindow* window;
	Main() : camera(Vec3()) {}
};