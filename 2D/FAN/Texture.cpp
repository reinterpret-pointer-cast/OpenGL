#include "Texture.hpp"
#include "FAN/Bmp.hpp"

void WriteImageData(ImageData imagedata) {
	std::ofstream data("data");
	for (int i = 0; i < sizeof(imagedata.image) / sizeof(*imagedata.image); i++) {
		data << imagedata.image[i] << std::endl;
	}
	data << imagedata.object.EBO << std::endl;
	data << imagedata.object.height << std::endl;
	data << imagedata.object.texture << std::endl;
	data << imagedata.object.VAO << std::endl;
	data << imagedata.object.VBO << std::endl;
	data << imagedata.object.width << std::endl;
	data.close();
}

ImageData GetImageData() {
	ImageData imagedata;
	std::ifstream data;
	data.open("data");

	size_t i = 0;
	unsigned char* image;
	std::string line;
	while (getline(data, line)) {
		// using printf() in all tests for consiste8ncy
		for (int j = 0; j < 8; j++) {
			imagedata.image = reinterpret_cast<const unsigned char*>(line.c_str());
		}

		if (i == 8) {
			imagedata.object.EBO = atoi(line.c_str());
		}
		else if (i == 9) {
			imagedata.object.height = atoi(line.c_str());
		}
		else if (i == 10) {
			imagedata.object.texture = atoi(line.c_str());
		}
		else if (i == 11) {
			imagedata.object.VAO = atoi(line.c_str());
		}
		else if (i == 12) {
			imagedata.object.VBO = atoi(line.c_str());
		}
		else if (i == 13) {
			imagedata.object.width = atoi(line.c_str());
		}
		i++;
	}
	data.close();
	return imagedata;
}

void LoadImg(const char* path, Object& object, Texture& texture) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ALPHA);
	std::ifstream file(path);
	if (!file.good()) {
		printf("File path does not exist\n");
		return;
	}
	glGenVertexArrays(1, &object.VAO);
	glGenBuffers(1, &object.VBO);
	glBindVertexArray(object.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, object.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texture.vertices), texture.vertices, GL_STATIC_DRAW); //almost xd colors are durnk
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
	glGenTextures(1, &object.texture);
	glBindTexture(GL_TEXTURE_2D, object.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, object.width, object.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, LoadBMP(path, object));
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::IntializeImage(Texture& texture) {
	const float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f
	};
	std::copy(std::begin(vertices), std::end(vertices), std::begin(texture.vertices));
}

Sprite::Sprite(const Sprite& info) {
	this->object = info.object;
	this->texture = info.texture;
	this->position = info.position;
}

Sprite::Sprite(Camera* camera, const char* path, Vec2 size, Vec2 position, Shader shader, float angle) : shader(shader), camera(camera), angle(angle), position(0), texture(), object() {
	texture.IntializeImage(texture);
	LoadImg(path, object, texture);
	this->size = Vec2(object.width * size.x, object.height * size.y);
	this->position = Vec2(position.x - this->size.x / 2, position.y - this->size.y / 2);
}

void Sprite::SetPosition(const Vec2& position) {
	this->position = position;
}

void Sprite::Draw() {
	shader.Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, object.texture);
	glUniform1i(glGetUniformLocation(shader.ID, "ourTexture"), 0);
	Mat4x4 view(1);
	view = camera->GetViewMatrix(Translate(view, Vec3(windowSize.x / 2, windowSize.y / 2, -700.0f)));
	Mat4x4 projection(1);
	projection = Ortho(windowSize.x / 2, windowSize.x  + windowSize.x * 0.5, windowSize.y + windowSize.y * 0.5, windowSize.y / 2, 0.1, 1000.0f);
	GLint projLoc = glGetUniformLocation(shader.ID, "projection");
	GLint viewLoc = glGetUniformLocation(shader.ID, "view");
	GLint modelLoc = glGetUniformLocation(shader.ID, "model");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view.vec[0].x);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection.vec[0].x);
	Mat4x4 model(1);
	model = Translate(model, V2ToV3(position));
	if (size.x || size.y) {
		model = Scale(model, Vec3(size.x, size.y, 0));
	}
	else {
		model = Scale(model, Vec3(object.width, object.height, 0));
	}
	if (angle) {
		model = Rotate(model, angle, Vec3(0, 0, 1));
	}
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model.vec[0].x);
	glBindVertexArray(object.VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
}

Shape::Shape(Camera* camera, const Vec2& position, const Vec2& pixelSize, const Color& color, std::vector<float> vec, 
	Shader shader) : shader(shader), position(position), size(pixelSize), color(color), camera(camera), angle(0) {
	glGenVertexArrays(1, &this->object.VAO);
	glGenBuffers(1, &this->object.VBO);
	glBindVertexArray(this->object.VAO);

	for (auto i : vec) {
		vertices.push_back(i);
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->object.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Shape::~Shape() {
	glDeleteVertexArrays(1, &this->object.VAO);
	glDeleteBuffers(1, &this->object.VBO);
}

void Shape::Draw() {
	this->shader.Use();
	Mat4x4 view(1);

	view = camera->GetViewMatrix(Translate(view, Vec3(windowSize.x / 2, windowSize.y / 2, -700.0f)));
	
	Mat4x4 projection(1);
	projection = Ortho(windowSize.x / 2, windowSize.x + windowSize.x * 0.5, windowSize.y + windowSize.y * 0.5, windowSize.y / 2, 0.1, 1000.0f);
	static int projLoc = glGetUniformLocation(shader.ID, "projection");
	static int viewLoc = glGetUniformLocation(shader.ID, "view");
	static int modelLoc = glGetUniformLocation(shader.ID, "model");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view.vec[0].x);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection.vec[0].x);

	static int colorLoc = glGetUniformLocation(shader.ID, "color");
	glUniform4f(colorLoc, this->color.r / 0xff, this->color.g / 0xff, this->color.b / 0xff, this->color.a / 0xff);

	Mat4x4 model(1);
	model = Translate(model, Vec3(this->position));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &view.vec[0].x);

	glBindVertexArray(this->object.VAO);
	glDrawArrays(this->type, 0, this->points);
	glBindVertexArray(0);
}

void Shape::DrawLight(Square& light) {
	this->shader.Use();

	Mat4x4 view(1);

	view = camera->GetViewMatrix(Translate(view, Vec3(windowSize.x / 2, windowSize.y / 2, -700.0f)));

	Mat4x4 projection(1);
	projection = Ortho(windowSize.x / 2, windowSize.x + windowSize.x * 0.5, windowSize.y + windowSize.y * 0.5, windowSize.y / 2, 0.1, 1000.0f);
	static int projLoc = glGetUniformLocation(shader.ID, "projection");
	static int viewLoc = glGetUniformLocation(shader.ID, "view");
	static int modelLoc = glGetUniformLocation(shader.ID, "model");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view.vec[0].x);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection.vec[0].x);

	static int colorLoc = glGetUniformLocation(shader.ID, "color");
	glUniform4f(colorLoc, this->color.r / 0xff, this->color.g / 0xff, this->color.b / 0xff, this->color.a / 0xff);

	Mat4x4 model(1);
	model = Translate(model, Vec3(this->position));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model.vec[0].x);

	glBindVertexArray(this->object.VAO);
	glDrawArrays(this->type, 0, this->points);
	glBindVertexArray(0);
}

void Shape::SetColor(Color color) {
	this->color = color;
}

void Shape::Rotatef(float angle, Vec2 point) {
	angle = -angle;
	float c1 = this->vertices[2] - this->vertices[0];
	float c2 = this->vertices[3] - this->vertices[1];
	Vec2 middle;
	if (point != MIDDLE) {
		middle = point;
	}
	else {
		middle = Vec2(this->vertices[0] + 0.5 * c1, this->vertices[1] + 0.5 * c2);
	}

	this->vertices[0] = -(cos(Radians(angle)) * c1 - sin(Radians(angle)) * c2) * 0.5 + middle.x;
	this->vertices[1] = -(sin(Radians(angle)) * c1 + cos(Radians(angle)) * c2) * 0.5 + middle.y;
	this->vertices[2] = (cos(Radians(angle)) * c1 - sin(Radians(angle)) * c2) * 0.5 + middle.x;
	this->vertices[3] = (sin(Radians(angle)) * c1 + cos(Radians(angle)) * c2) * 0.5 + middle.y;

	glBindBuffer(GL_ARRAY_BUFFER, this->object.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices[0]) * this->vertices.size(), this->vertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Vec2 Shape::Size() const {
	return this->size;
}

Vec2 Shape::GetPosition() const {
	return this->position;
}

void Triangle::Add(const Vec2& position, Vec2 size) {
	this->vertices.push_back(position.x - (size.x / 2));
	this->vertices.push_back(position.y + (size.y / 2));
	this->vertices.push_back(position.x + (size.x / 2));
	this->vertices.push_back(position.y + (size.y / 2));
	this->vertices.push_back(position.x);
	this->vertices.push_back(position.y - (size.y / 2));

	glBindBuffer(GL_ARRAY_BUFFER, this->object.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices[0]) * this->vertices.size(), this->vertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	this->points += 3;
}

void Triangle::SetPosition(size_t _Where, const Vec2& position) {
	this->vertices[_Where * TRIANGLEVERT + 0] = position.x - (size.x / 2);
	this->vertices[_Where * TRIANGLEVERT + 1] = position.y + (size.y / 2);
	this->vertices[_Where * TRIANGLEVERT + 2] = position.x + (size.x / 2);
	this->vertices[_Where * TRIANGLEVERT + 3] = position.y + (size.y / 2);
	this->vertices[_Where * TRIANGLEVERT + 4] = position.x;
	this->vertices[_Where * TRIANGLEVERT + 5] = position.y - (size.y / 2);

	glBindBuffer(GL_ARRAY_BUFFER, this->object.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices[0]) * this->vertices.size(), this->vertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Square::Add(const Vec2& position, const Vec2& size) {
	this->vertices.push_back(position.x - (this->size.x / 2));
	this->vertices.push_back(position.y - (this->size.y / 2));
	this->vertices.push_back(position.x + (this->size.x / 2));
	this->vertices.push_back(position.y - (this->size.y / 2));
	this->vertices.push_back(position.x + (this->size.x / 2));
	this->vertices.push_back(position.y + (this->size.y / 2));
	this->vertices.push_back(position.x - (this->size.x / 2));
	this->vertices.push_back(position.y + (this->size.y / 2));

	glBindBuffer(GL_ARRAY_BUFFER, this->object.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices[0]) * this->vertices.size(), this->vertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	this->points += SQUAREVERT / 2;
}

void Square::SetPosition(size_t _Where, const Vec2& position) {

	if (position.x == cursorPos.x && position.y == cursorPos.y) {
		this->vertices[_Where * SQUAREVERT + 0] = camera->position.x + position.x - (this->size.x / 2);
		this->vertices[_Where * SQUAREVERT + 1] = camera->position.y + position.y - (this->size.y / 2);
		this->vertices[_Where * SQUAREVERT + 2] = camera->position.x + position.x + (this->size.x / 2);
		this->vertices[_Where * SQUAREVERT + 3] = camera->position.y + position.y - (this->size.y / 2);
		this->vertices[_Where * SQUAREVERT + 4] = camera->position.x + position.x + (this->size.x / 2);
		this->vertices[_Where * SQUAREVERT + 5] = camera->position.y + position.y + (this->size.y / 2);
		this->vertices[_Where * SQUAREVERT + 6] = camera->position.x + position.x - (this->size.x / 2);
		this->vertices[_Where * SQUAREVERT + 7] = camera->position.y + position.y + (this->size.y / 2);

	}
	else {
		this->vertices[_Where * SQUAREVERT + 0] = position.x - (this->size.x / 2);
		this->vertices[_Where * SQUAREVERT + 1] = position.y - (this->size.y / 2);
		this->vertices[_Where * SQUAREVERT + 2] = position.x + (this->size.x / 2);
		this->vertices[_Where * SQUAREVERT + 3] = position.y - (this->size.y / 2);
		this->vertices[_Where * SQUAREVERT + 4] = position.x + (this->size.x / 2);
		this->vertices[_Where * SQUAREVERT + 5] = position.y + (this->size.y / 2);
		this->vertices[_Where * SQUAREVERT + 6] = position.x - (this->size.x / 2);
		this->vertices[_Where * SQUAREVERT + 7] = position.y + (this->size.y / 2);
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->object.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices[0]) * this->vertices.size(), this->vertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	this->position = position;
}

void Line::Add(const Mat2x2& begin_end) {
	for (int i = 0; i < 4; i++) {
		this->vertices.push_back(begin_end.vec[(i & 2) >> 1][i & 1]);
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->object.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices[0]) * this->vertices.size(), this->vertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	this->points += 2;
}

void Line::SetPosition(size_t _Where, const Mat2x2& begin_end) {
	for (int i = _Where * 4, j = 0; i < _Where * 4 + 4; i++, j++) {
		this->vertices[i] = begin_end.vec[(i & 2) >> 1][i & 1];
	}
	 
	glBindBuffer(GL_ARRAY_BUFFER, this->object.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices[0]) * this->vertices.size(), this->vertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Line::SetPosition(size_t _Where, const Vec2& position) {
	for (int i = 0; i < this->vertSize; i++) {
		this->vertices[_Where * this->vertSize + i] = i < 2 ? position[i & 1] - size[i & 1] / 2 : position[i & 1] + size[i & 1] / 2;
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->object.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices[0]) * this->vertices.size(), this->vertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Entity::Entity(const char* path, GroupId _groupId = GroupId::NotAssigned) : velocity(0), groupId(_groupId) {
	texture.IntializeImage(texture);
	LoadImg(path, object, texture);
}

GroupId Entity::GetGroupId() {
	return this->groupId;
}

void Entity::SetGroupId(GroupId groupId) {
	this->groupId = groupId;
}

void Entity::Move() {
	
	auto groundPosition = windowSize.y - GRASSHEIGHT - size.y / 2;
	bool playerOnGround = this->position.y == groundPosition;

	if (playerOnGround) {
		velocity.x /= (deltaTime * friction) + 1;
		velocity.y /= (deltaTime * friction) + 1;
	}

	if (KeyPress(GLFW_KEY_A) && playerOnGround) {
		if (this->object.texture != this->objects[1].texture) {
			this->object = this->objects[1];
			this->texture = this->textures[1];
		}
		velocity.x -= deltaTime * this->movementSpeed;
	}
	if (KeyPress(GLFW_KEY_D) && playerOnGround) {
		if (this->object.texture != this->objects[0].texture) {
			this->object = this->objects[0];
			this->texture = this->textures[0];
		}
		velocity.x += deltaTime * this->movementSpeed;
	}
	
	static bool jump = false;
	static double jumpTime;

	if (KeyPressA(GLFW_KEY_SPACE) && playerOnGround) {
		this->velocity.y -= this->jumpForce;
	}

	static float downAccel = 0;
	if (position.y > groundPosition) {
		this->position.y = groundPosition;
		this->velocity.y = 0;
		downAccel = 0;
	}
	else if (position.y < groundPosition) {
		downAccel += deltaTime * (this->gravity);
		this->velocity.y += deltaTime * downAccel;
	}

	position += (velocity * deltaTime);

	const float middle = position.x - windowSize.x / 2;
	const float cameraMoveOffset = 200;

	if (camera->position.x < middle - cameraMoveOffset) {
		camera->position.x = middle - cameraMoveOffset;
	}
	else if (camera->position.x > middle + cameraMoveOffset) {
		camera->position.x = middle + cameraMoveOffset;
	}
}