﻿#include "CGA.h"
#include "GLUtils.h"
#include <map>

namespace cga {

const float M_PI = 3.1415926f;
const bool showAxes = false;

BoundingBox::BoundingBox(const std::vector<glm::vec2>& points) {
	bottom_left = glm::vec2((std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)());
	upper_right = -bottom_left;

	for (int i = 0; i < points.size(); ++i) {
		bottom_left.x = min(bottom_left.x, points[i].x);
		bottom_left.y = min(bottom_left.y, points[i].y);
		upper_right.x = max(upper_right.x, points[i].x);
		upper_right.y = max(upper_right.y, points[i].y);
	}
}

void Object::translate(const glm::vec3& v) {
	modelMat = glm::translate(modelMat, v);
}

void Object::rotate(float xAngle, float yAngle, float zAngle) {
	modelMat = glm::rotate(modelMat, xAngle * M_PI / 180.0f, glm::vec3(1, 0, 0));
	modelMat = glm::rotate(modelMat, yAngle * M_PI / 180.0f, glm::vec3(0, 1, 0));
	modelMat = glm::rotate(modelMat, zAngle * M_PI / 180.0f, glm::vec3(0, 0, 1));
}

void Object::nil() {
	removed = true;
}

void Object::setTexture(const std::string& texture) {
	this->texture = texture;
	textureEnabled = true;
}

Object* Object::clone() {
	throw "clone() is not supported.";
}

void Object::setupProjection(float texWidth, float texHeight) {
	throw "setupProjection() is not supported.";
}

Object* Object::extrude(const std::string& name, float height) {
	throw "extrude() is not supported.";
}

Object* Object::taper(const std::string& name, float height, float top_ratio) {
	throw "taper() is not supported.";
}

Object* Object::offset(const std::string& name, float offsetRatio) {
	throw "offset() is not supported.";
}

Object* offset(const std::string& name, float offsetRatio) {
	throw "offset() is not supported.";
}

Object* Object::revolve(const std::string& name, int direction) {
	throw "revolve() is not supported.";
}

void Object::split(int direction, const std::vector<float> ratios, const std::vector<std::string> names, std::vector<Object*>& objects) {
	throw "split() is not supported.";
}

void Object::componentSplit(const std::string& front_name, Rectangle** front, const std::string& sides_name, std::vector<Rectangle*>& sides, const std::string& top_name, Polygon** top, const std::string& base_name, Polygon** base) {
	throw "componentSplit() is not supported.";
}

void Object::generate(RenderManager* renderManager) {
	throw "generate() is not supported.";
}

Line::Line(const std::string& name, const glm::mat4& modelMat, const std::vector<glm::vec2>& points, const glm::vec3& color) {
	this->name = name;
	this->removed = false;
	this->modelMat = modelMat;
	this->points = points;
	this->color = color;
	this->textureEnabled = false;

	BoundingBox bbox(points);
	this->scope = glm::vec3(bbox.upper_right.x, bbox.upper_right.y, 0);
}

Object* Line::clone() {
	return new Line(*this);
}

Object* Line::revolve(const std::string& name, int direction) {
	return new RevolvedLine(name, modelMat, points, direction, color);
}

void Line::generate(RenderManager* renderManager) {
	// render nothing
}

RevolvedLine::RevolvedLine(const std::string& name, const glm::mat4& modelMat, const std::vector<glm::vec2>& points, int direction, const glm::vec3& color) {
	this->name = name;
	this->removed = false;
	this->modelMat = modelMat;
	this->points = points;
	this->direction = direction;
	this->color = color;
	this->textureEnabled = false;

	BoundingBox bbox(points);
	this->scope = glm::vec3(bbox.upper_right.x, bbox.upper_right.y, bbox.upper_right.y);
}

Object* RevolvedLine::clone() {
	return new RevolvedLine(*this);
}

void RevolvedLine::generate(RenderManager* renderManager) {
	std::vector<Vertex> vertices;

	if (direction == REVOLVE_X) {
		glm::mat4 mat = modelMat;
		for (int i = 0; i < points.size() - 1; ++i) {
			glutils::drawCylinderX(points[i].y, points[i + 1].y, points[i + 1].x - points[i].x, color, mat, vertices);

			mat = glm::translate(mat, glm::vec3(points[i + 1].x - points[i].x, 0, 0));
		}
	} else {
		glm::mat4 mat = modelMat;
		for (int i = 0; i < points.size() - 1; ++i) {
			glutils::drawCylinderY(points[i].x, points[i + 1].x, points[i + 1].y - points[i].y, color, mat, vertices);

			mat = glm::translate(mat, glm::vec3(0, points[i + 1].y - points[i].y, 0));
		}
	}

	renderManager->addObject(name.c_str(), "", vertices);
}

PrismObject::PrismObject(const std::string& name, const glm::mat4& modelMat, const std::vector<glm::vec2>& points, float height, const glm::vec3& color) {
	this->name = name;
	this->removed = false;
	this->modelMat = modelMat;
	this->points = points;
	this->height = height;
	this->color = color;

	BoundingBox bbox(points);
	this->scope = glm::vec3(bbox.upper_right.x, bbox.upper_right.y, height);
}

Object* PrismObject::clone() {
	return new PrismObject(*this);
}

void PrismObject::setupProjection(float texWidth, float texHeight) {
}

void PrismObject::split(int direction, const std::vector<float> ratios, const std::vector<std::string> names, std::vector<Object*>& objects) {
	throw "PrismObject does not support split operation.";
}


void PrismObject::componentSplit(const std::string& front_name, Rectangle** front, const std::string& sides_name, std::vector<Rectangle*>& sides, const std::string& top_name, Polygon** top, const std::string& base_name, Polygon** base) {
	// front face
	{
		*front = new Rectangle(front_name, glm::rotate(modelMat, M_PI * 0.5f, glm::vec3(1, 0, 0)), glm::length(points[1] - points[0]), height, color);
	}

	// side faces
	{
		glm::mat4 mat;
		sides.resize(points.size() - 1);
		for (int i = 1; i < points.size(); ++i) {
			glm::vec2 a = points[i] - points[i - 1];
			glm::vec2 b = points[(i + 1) % points.size()] - points[i];

			mat = glm::translate(mat, glm::vec3(glm::length(a), 0, 0));
			float theta = acos(glm::dot(a, b) / glm::length(a) / glm::length(b));
			mat = glm::rotate(mat, theta, glm::vec3(0, 0, 1));
			glm::mat4 mat2 = glm::rotate(mat, M_PI * 0.5f, glm::vec3(1, 0, 0));
			glm::mat4 invMat = glm::inverse(mat2);

			std::vector<glm::vec2> sidePoints(4);
			sidePoints[0] = glm::vec2(invMat * glm::vec4(points[i], 0, 1));
			sidePoints[1] = glm::vec2(invMat * glm::vec4(points[(i + 1) % points.size()], 0, 1));
			sidePoints[2] = glm::vec2(invMat * glm::vec4(points[(i + 1) % points.size()], height, 1));
			sidePoints[3] = glm::vec2(invMat * glm::vec4(points[i], height, 1));

			sides[i - 1] = new Rectangle(sides_name, modelMat * mat2, glm::length(points[(i + 1) % points.size()] - points[i]), height, color);
		}
	}

	// top face
	{
		*top = new Polygon(top_name, glm::translate(modelMat, glm::vec3(0, 0, height)), points, color, texture);
	}

	// bottom face
	{
		std::vector<glm::vec2> basePoints = points;
		std::reverse(basePoints.begin(), basePoints.end());
		*base = new Polygon(base_name, modelMat, basePoints, color, texture);
	}
}

void PrismObject::generate(RenderManager* renderManager) {
	if (removed) return;

	std::vector<Vertex> vertices((points.size() - 2) * 6 + points.size() * 6);

	int num = 0;

	// top
	{
		glm::vec4 p0(points.back(), height, 1);
		p0 = modelMat * p0;
		glm::vec4 normal(0, 0, 1, 0);
		normal = modelMat * normal;

		glm::vec4 p1(points[0], height, 1);
		p1 = modelMat * p1;

		for (int i = 0; i < points.size() - 2; ++i) {
			glm::vec4 p2(points[i + 1], height, 1);
			p2 = modelMat * p2;

			vertices[i * 3] = Vertex(glm::vec3(p0), glm::vec3(normal), color);
			vertices[i * 3 + 1] = Vertex(glm::vec3(p1), glm::vec3(normal), color);
			vertices[i * 3 + 2] = Vertex(glm::vec3(p2), glm::vec3(normal), color);

			p1 = p2;
		}

		num += (points.size() - 2) * 3;
	}

	// bottom
	{
		glm::vec4 p0(points.back(), 0, 1);
		p0 = modelMat * p0;
		glm::vec4 normal(0, 0, -1, 0);
		normal = modelMat * normal;

		glm::vec4 p1(points[0], 0, 1);
		p1 = modelMat * p1;

		for (int i = 0; i < points.size() - 2; ++i) {
			glm::vec4 p2(points[i + 1], 0, 1);
			p2 = modelMat * p2;

			vertices[num + i * 3] = Vertex(glm::vec3(p0), glm::vec3(normal), color);
			vertices[num + i * 3 + 1] = Vertex(glm::vec3(p2), glm::vec3(normal), color);
			vertices[num + i * 3 + 2] = Vertex(glm::vec3(p1), glm::vec3(normal), color);

			p1 = p2;
		}

		num += (points.size() - 2) * 3;
	}

	// side
	{
		glm::vec4 p1(points.back(), 0, 1);
		glm::vec4 p2(points.back(), height, 1);
		p1 = modelMat * p1;
		p2 = modelMat * p2;

		for (int i = 0; i < points.size(); ++i) {
			glm::vec4 p3(points[i], 0, 1);
			glm::vec4 p4(points[i], height, 1);
			p3 = modelMat * p3;
			p4 = modelMat * p4;

			glm::vec3 normal = glm::normalize(glm::cross(glm::vec3(p3) - glm::vec3(p1), glm::vec3(p2) - glm::vec3(p1)));
			
			vertices[num + i * 6 + 0] = Vertex(glm::vec3(p1), normal, color);
			vertices[num + i * 6 + 1] = Vertex(glm::vec3(p3), normal, color);
			vertices[num + i * 6 + 2] = Vertex(glm::vec3(p4), normal, color);

			vertices[num + i * 6 + 3] = Vertex(glm::vec3(p1), normal, color);
			vertices[num + i * 6 + 4] = Vertex(glm::vec3(p4), normal, color);
			vertices[num + i * 6 + 5] = Vertex(glm::vec3(p2), normal, color);

			p1 = p3;
			p2 = p4;
		}
	}

	renderManager->addObject(name.c_str(), "", vertices);

	if (showAxes) {
		vertices.resize(0);
		glutils::drawAxes(0.1, 3, modelMat, vertices);
		renderManager->addObject("axis", "", vertices);
	}
}

Rectangle::Rectangle(const std::string& name, const glm::mat4& modelMat, float width, float height, const glm::vec3& color) {
	this->name = name;
	this->removed = false;
	this->modelMat = modelMat;
	this->width = width;
	this->height = height;
	this->color = color;
	this->scope = glm::vec3(width, height, 0);
	this->textureEnabled = false;
}

Rectangle::Rectangle(const std::string& name, const glm::mat4& modelMat, float width, float height, const std::string& texture, float u1, float v1, float u2, float v2) {
	this->name = name;
	this->removed = false;
	this->modelMat = modelMat;
	this->width = width;
	this->height = height;
	this->color = color;
	this->texture = texture;
	this->scope = glm::vec3(width, height, 0);

	texCoords.resize(4);
	texCoords[0] = glm::vec2(u1, v1);
	texCoords[1] = glm::vec2(u2, v1);
	texCoords[2] = glm::vec2(u2, v2);
	texCoords[3] = glm::vec2(u1, v2);
	this->textureEnabled = true;
}

Object* Rectangle::clone() {
	return new Rectangle(*this);
}

void Rectangle::setupProjection(float texWidth, float texHeight) {
	texCoords.resize(4);
	texCoords[0] = glm::vec2(0, 0);
	texCoords[1] = glm::vec2(width / texWidth, 0);
	texCoords[2] = glm::vec2(width / texWidth, height / texHeight);
	texCoords[3] = glm::vec2(0, height / texHeight);
}

Object* Rectangle::extrude(const std::string& name, float height) {
	std::vector<glm::vec2> points(4);
	points[0] = glm::vec2(0, 0);
	points[1] = glm::vec2(this->width, 0);
	points[2] = glm::vec2(this->width, this->height);
	points[3] = glm::vec2(0, this->height);
	return new PrismObject(name, modelMat, points, height, color);
}

Object* Rectangle::taper(const std::string& name, float height, float top_ratio) {
	std::vector<glm::vec2> points(4);
	points[0] = glm::vec2(0, 0);
	points[1] = glm::vec2(this->width, 0);
	points[2] = glm::vec2(this->width, this->height);
	points[3] = glm::vec2(0, this->height);
	return new Pyramid(name, modelMat, points, glm::vec2(this->width * 0.5, this->height * 0.5), height, top_ratio, color, texture);
}

Object* Rectangle::offset(const std::string& name, float offsetRatio) {
	glm::mat4 mat = glm::translate(modelMat, glm::vec3(scope.x * 0.5f * offsetRatio, scope.y * 0.5f * offsetRatio, 0));
	if (textureEnabled) {
		return new Rectangle(name, mat, width * (1.0f - offsetRatio), height * (1.0f - offsetRatio), texture, texCoords[0].x, texCoords[0].y, texCoords[2].x, texCoords[2].y);
	} else {
		return new Rectangle(name, mat, width * (1.0f - offsetRatio), height * (1.0f - offsetRatio), color);
	}
}

void Rectangle::split(int direction, const std::vector<float> ratios, const std::vector<std::string> names, std::vector<Object*>& objects) {
	objects.resize(ratios.size());

	float offset = 0.0f;
	
	for (int i = 0; i < ratios.size(); ++i) {
		glm::mat4 mat;
		if (direction == DIRECTION_X) {
			mat = glm::translate(glm::mat4(), glm::vec3(offset, 0, 0));
			if (textureEnabled) {
				objects[i] = new Rectangle(names[i], modelMat * mat, width * ratios[i], height, texture, 
					texCoords[0].x + (texCoords[1].x - texCoords[0].x) * offset / width, texCoords[0].y,
					texCoords[0].x + (texCoords[1].x - texCoords[0].x) * (offset / width + ratios[i]), texCoords[2].y);
			} else {
				objects[i] = new Rectangle(names[i], modelMat * mat, width * ratios[i], height, color); 
			}
			offset += width * ratios[i];
		} else {
			mat = glm::translate(glm::mat4(), glm::vec3(0, offset, 0));
			if (textureEnabled) {
				objects[i] = new Rectangle(names[i], modelMat * mat, width, height * ratios[i], texture,
					texCoords[0].x, texCoords[0].y + (texCoords[2].y - texCoords[0].y) * offset / height,
					texCoords[1].x, texCoords[0].y + (texCoords[2].y - texCoords[0].y) * (offset / height + ratios[i]));
			} else {
				objects[i] = new Rectangle(names[i], modelMat * mat, width, height * ratios[i], color);
			}
			offset += height * ratios[i];
		}
	}
}

void Rectangle::generate(RenderManager* renderManager) {
	if (removed) return;

	std::vector<Vertex> vertices;

	vertices.resize(6);

	glm::vec4 p1(0, 0, 0, 1);
	p1 = modelMat * p1;
	glm::vec4 p2(width, 0, 0, 1);
	p2 = modelMat * p2;
	glm::vec4 p3(width, height, 0, 1);
	p3 = modelMat * p3;
	glm::vec4 p4(0, height, 0, 1);
	p4 = modelMat * p4;

	glm::vec4 normal(0, 0, 1, 0);
	normal = modelMat * normal;

	if (textureEnabled) {
		vertices[0] = Vertex(glm::vec3(p1), glm::vec3(normal), color, glm::vec3(texCoords[0], 0));
		vertices[1] = Vertex(glm::vec3(p2), glm::vec3(normal), color, glm::vec3(texCoords[1], 0));
		vertices[2] = Vertex(glm::vec3(p3), glm::vec3(normal), color, glm::vec3(texCoords[2], 0));

		vertices[3] = Vertex(glm::vec3(p1), glm::vec3(normal), color, glm::vec3(texCoords[0], 0));
		vertices[4] = Vertex(glm::vec3(p3), glm::vec3(normal), color, glm::vec3(texCoords[2], 0));
		vertices[5] = Vertex(glm::vec3(p4), glm::vec3(normal), color, glm::vec3(texCoords[3], 0));

		renderManager->addObject(name.c_str(), texture.c_str(), vertices);
	} else {
		vertices[0] = Vertex(glm::vec3(p1), glm::vec3(normal), color);
		vertices[1] = Vertex(glm::vec3(p2), glm::vec3(normal), color);
		vertices[2] = Vertex(glm::vec3(p3), glm::vec3(normal), color);

		vertices[3] = Vertex(glm::vec3(p1), glm::vec3(normal), color);
		vertices[4] = Vertex(glm::vec3(p3), glm::vec3(normal), color);
		vertices[5] = Vertex(glm::vec3(p4), glm::vec3(normal), color);

		renderManager->addObject(name.c_str(), "", vertices);
	}
	
	vertices.resize(0);
	if (showAxes) {
		glutils::drawAxes(0.1, 3, modelMat, vertices);
		renderManager->addObject("axis", "", vertices);
	}
}

Polygon::Polygon(const std::string& name, const glm::mat4& modelMat, const std::vector<glm::vec2>& points, const glm::vec3& color, const std::string& texture) {
	this->name = name;
	this->removed = false;
	this->modelMat = modelMat;
	this->points = points;
	this->color = color;
	this->texture = texture;

	BoundingBox bbox(points);
	this->scope = glm::vec3(bbox.upper_right.x, bbox.upper_right.y, 0);

	this->center = glm::vec2(0, 0);
	for (int i = 0; i < points.size(); ++i) {
		center += points[i];
	}
	center /= points.size();
}

Object* Polygon::clone() {
	return new Polygon(*this);
}


void Polygon::setupProjection(float texWidth, float texHeight) {
	texCoords.resize(points.size());
	for (int i = 0; i < points.size(); ++i) {
		texCoords[i] = glm::vec2(points[i].x / texWidth, points[i].y / texHeight);
	}
}

Object* Polygon::extrude(const std::string& name, float height) {
	return new PrismObject(name, modelMat, points, height, color);
}

Object* Polygon::taper(const std::string& name, float height, float top_ratio) {
	return new Pyramid(name, modelMat, points, center, height, top_ratio, color, texture);
}

Object* Polygon::offset(const std::string& name, float offsetRatio) {
	glm::vec2 t = glm::vec2(points[0] * (1.0f - offsetRatio) + center * offsetRatio) - points[0];

	std::vector<glm::vec2> new_points(points.size());
	for (int i = 0; i < points.size(); ++i) {
		new_points[i] = glm::vec2(points[i] * (1.0f - offsetRatio) + center * offsetRatio) - t;
	}

	glm::mat4 mat = glm::translate(modelMat, glm::vec3(t, 0));
	return new Polygon(name, mat, new_points, color, texture);
}

void Polygon::generate(RenderManager* renderManager) {
	if (removed) return;

	std::vector<Vertex> vertices((points.size() - 2) * 3);

	glm::vec4 p0(points[0], 0, 1);
	p0 = modelMat * p0;
	glm::vec4 normal(0, 0, 1, 0);
	normal = modelMat * normal;

	glm::vec4 p1(points[1], 0, 1);
	p1 = modelMat * p1;

	glm::vec3 uv1(points[1].x / scope.x, points[1].y / scope.y, 0);

	for (int i = 1; i < points.size() - 1; ++i) {
		glm::vec4 p2(points[i + 1], 0, 1);
		p2 = modelMat * p2;

		glm::vec3 uv2(points[i + 1].x / scope.x, points[i + 1].y / scope.y, 0);

		vertices[(i - 1) * 3] = Vertex(glm::vec3(p0), glm::vec3(normal), color, glm::vec3(0, 0, 0));
		vertices[(i - 1) * 3 + 1] = Vertex(glm::vec3(p1), glm::vec3(normal), color, uv1);
		vertices[(i - 1) * 3 + 2] = Vertex(glm::vec3(p2), glm::vec3(normal), color, uv2);

		p1 = p2;
		uv1 = uv2;
	}

	renderManager->addObject(name.c_str(), texture.c_str(), vertices);

	if (showAxes) {
		vertices.resize(0);
		glutils::drawAxes(0.1, 3, modelMat, vertices);
		renderManager->addObject("axis", "", vertices);
	}
}

Pyramid::Pyramid(const std::string& name, const glm::mat4& modelMat, const std::vector<glm::vec2>& points, const glm::vec2& center, float height, float top_ratio, const glm::vec3& color, const std::string& texture) {
	this->name = name;
	this->removed = false;
	this->modelMat = modelMat;
	this->points = points;
	this->center = center;
	this->height = height;
	this->top_ratio = top_ratio;
	this->color = color;
	this->texture = texture;

	BoundingBox bbox(points);
	this->scope = glm::vec3(bbox.upper_right.x, bbox.upper_right.y, height);
}

void Pyramid::componentSplit(const std::string& front_name, Rectangle** front, const std::string& sides_name, std::vector<Rectangle*>& sides, const std::string& top_name, Polygon** top, const std::string& base_name, Polygon** base) {
	// front face (To be fixed)
	{
		*front = new Rectangle(front_name, glm::rotate(modelMat, M_PI * 0.5f, glm::vec3(1, 0, 0)), glm::length(points[1] - points[0]), height, color);
	}

	// side faces (To be fixed);
	{
		glm::mat4 mat;
		sides.resize(points.size() - 1);
		for (int i = 1; i < points.size(); ++i) {
			glm::vec2 a = points[i] - points[i - 1];
			glm::vec2 b = points[(i + 1) % points.size()] - points[i];

			mat = glm::translate(mat, glm::vec3(glm::length(a), 0, 0));
			float theta = acos(glm::dot(a, b) / glm::length(a) / glm::length(b));
			mat = glm::rotate(mat, theta, glm::vec3(0, 0, 1));
			glm::mat4 mat2 = glm::rotate(mat, M_PI * 0.5f, glm::vec3(1, 0, 0));
			glm::mat4 invMat = glm::inverse(mat2);

			std::vector<glm::vec2> sidePoints(4);
			sidePoints[0] = glm::vec2(invMat * glm::vec4(points[i], 0, 1));
			sidePoints[1] = glm::vec2(invMat * glm::vec4(points[(i + 1) % points.size()], 0, 1));
			sidePoints[2] = glm::vec2(invMat * glm::vec4(points[(i + 1) % points.size()], height, 1));
			sidePoints[3] = glm::vec2(invMat * glm::vec4(points[i], height, 1));

			sides[i - 1] = new Rectangle(sides_name, modelMat * mat2, glm::length(points[(i + 1) % points.size()] - points[i]), height, color);
		}
	}

	// top face
	{
		std::vector<glm::vec2> new_points(points.size());
		for (int i = 0; i < points.size(); ++i) {
			new_points[i] = glm::vec2(points[i] * top_ratio + center * (1.0f - top_ratio));
		}
		*top = new Polygon(top_name, glm::translate(modelMat, glm::vec3(0, 0, height)), new_points, color, texture);
	}

	// bottom face
	{
		std::vector<glm::vec2> basePoints = points;
		std::reverse(basePoints.begin(), basePoints.end());
		*base = new Polygon(base_name, modelMat, basePoints, color, texture);
	}
}

void Pyramid::generate(RenderManager* renderManager) {
	if (removed) return;

	if (top_ratio == 0.0f) {
		std::vector<Vertex> vertices(points.size() * 3);

		glm::vec4 p0(center, height, 1);
		p0 = modelMat * p0;

		glm::vec4 p1(points.back(), 0, 1);
		p1 = modelMat * p1;

		for (int i = 0; i < points.size(); ++i) {
			glm::vec4 p2(points[i], 0, 1);
			p2 = modelMat * p2;

			glm::vec3 normal = glm::cross(glm::vec3(p1 - p0), glm::vec3(p2 - p0));

			vertices[i * 3] = Vertex(glm::vec3(p0), normal, color, glm::vec3(0, 0, 0));
			vertices[i * 3 + 1] = Vertex(glm::vec3(p1), normal, color, glm::vec3(0, 0, 0));
			vertices[i * 3 + 2] = Vertex(glm::vec3(p2), normal, color, glm::vec3(0, 0, 0));

			p1 = p2;
		}

		renderManager->addObject(name.c_str(), texture.c_str(), vertices);
	} else {
		std::vector<Vertex> vertices(points.size() * 6);

		glm::vec4 p0(points.back(), 0, 1);
		p0 = modelMat * p0;

		glm::vec4 p1(points.back() * top_ratio + center * (1.0f - top_ratio), height, 1);
		p1 = modelMat * p1;

		std::vector<glm::vec3> pts3(points.size());
		for (int i = 0; i < points.size(); ++i) {
			glm::vec4 p2(points[i], 0, 1);
			p2 = modelMat * p2;

			glm::vec4 p3(points[i] * top_ratio + center * (1.0f - top_ratio), height, 1);
			pts3[i] = glm::vec3(p3);
			p3 = modelMat * p3;

			glm::vec3 normal = glm::cross(glm::vec3(p2 - p0), glm::vec3(p3 - p0));

			vertices[i * 6 + 0] = Vertex(glm::vec3(p0), normal, color, glm::vec3(0, 0, 0));
			vertices[i * 6 + 1] = Vertex(glm::vec3(p2), normal, color, glm::vec3(0, 0, 0));
			vertices[i * 6 + 2] = Vertex(glm::vec3(p3), normal, color, glm::vec3(0, 0, 0));

			vertices[i * 6 + 3] = Vertex(glm::vec3(p0), normal, color, glm::vec3(0, 0, 0));
			vertices[i * 6 + 4] = Vertex(glm::vec3(p3), normal, color, glm::vec3(0, 0, 0));
			vertices[i * 6 + 5] = Vertex(glm::vec3(p1), normal, color, glm::vec3(0, 0, 0));

			p0 = p2;
			p1 = p3;
		}

		glutils::drawPolygon(pts3, color, modelMat, vertices);

		renderManager->addObject(name.c_str(), texture.c_str(), vertices);
	}

	if (showAxes) {
		std::vector<Vertex> vertices;
		glutils::drawAxes(0.1, 3, modelMat, vertices);
		renderManager->addObject("axis", "", vertices);
	}
}

CGA::CGA() {
}

void CGA::generatePyramid(RenderManager* renderManager) {
	std::list<Object*> stack;
	
	Rectangle* lot = new Rectangle("Lot", glm::rotate(glm::mat4(), -M_PI * 0.5f, glm::vec3(1, 0, 0)), 20, 20, glm::vec3(1, 1, 1));
	stack.push_back(lot);
	
	while (!stack.empty()) {
		Object* obj = stack.front();
		stack.pop_front();

		if (obj->removed) {
			delete obj;
			continue;
		}
		
		if (obj->name == "Lot") {
			stack.push_back(obj->taper("pyramid", 10));
		} else {
			obj->generate(renderManager);
			delete obj;
		}
	}
}

void CGA::generateSimpleBuilding(RenderManager* renderManager) {
	std::list<Object*> stack;
	
	Rectangle* lot = new Rectangle("Lot", glm::rotate(glm::mat4(), -M_PI * 0.5f, glm::vec3(1, 0, 0)), 35, 10, glm::vec3(1, 1, 1));
	stack.push_back(lot);
	
	while (!stack.empty()) {
		Object* obj = stack.front();
		stack.pop_front();

		if (obj->removed) {
			delete obj;
			continue;
		}
		
		if (obj->name == "Lot") {
			std::vector<float> ratios(5);
			for (int i = 0; i < ratios.size(); ++i) ratios[i] = 1.0f / ratios.size();
			std::vector<std::string> names(5);
			for (int i = 0; i < names.size(); ++i) names[i] = "SmallLot";
			std::vector<Object*> polygons;
			obj->split(DIRECTION_X, ratios, names, polygons);

			for (int i = 0; i < polygons.size(); ++i) {
				if (i % 2 == 1) {
					polygons[i]->nil();
				}
			}

			stack.insert(stack.end(), polygons.begin(), polygons.end());
		} else if (obj->name == "SmallLot") {
			stack.push_back(obj->extrude("Building", 20));
		} else {
			obj->generate(renderManager);
			delete obj;
		}
	}
}

void CGA::generateBuilding(RenderManager* renderManager) {
	std::list<Object*> stack;
	
	Rectangle* lot = new Rectangle("Lot", glm::rotate(glm::mat4(), -M_PI * 0.5f, glm::vec3(1, 0, 0)), 35, 10, glm::vec3(1, 1, 1));
	stack.push_back(lot);
	
	while (!stack.empty()) {
		Object* obj = stack.front();
		stack.pop_front();

		if (obj->removed) {
			delete obj;
			continue;
		}
		
		if (obj->name == "Lot") {
			stack.push_back(obj->extrude("Building", 11));
		} else if (obj->name == "Building") {
			Rectangle* frontFacade;
			Polygon* roof;
			Polygon* base;
			std::vector<Rectangle*> sideFacades;
			obj->componentSplit("FrontFacade", &frontFacade, "SideFacade", sideFacades, "Roof", &roof, "Base", &base);

			stack.push_back(frontFacade);
			for (int i = 0; i < sideFacades.size(); ++i) {
				stack.push_back(sideFacades[i]);
			}
			stack.push_back(roof);
			stack.push_back(base);
		} else if (obj->name == "Roof") {
			obj->setupProjection(obj->scope.x, obj->scope.y);
			obj->setTexture("textures/roof.jpg");

			Object* obj2 = obj->clone();
			obj2->name = "Roof_1";
			stack.push_back(obj2);
		} else if (obj->name == "FrontFacade") {
			obj->setupProjection(2.5f, 1.0f);
			obj->setTexture("textures/brick.jpg");

			std::vector<float> floor_ratios(3);
			floor_ratios[0] = 4.0f / obj->scope.y;
			floor_ratios[1] = 3.5f / obj->scope.y;
			floor_ratios[2] = 3.5f / obj->scope.y;
			std::vector<std::string> floor_names(3);
			floor_names[0] = "Floor";
			floor_names[1] = "Floor";
			floor_names[2] = "Floor";

			std::vector<Object*> floors;
			obj->split(DIRECTION_Y, floor_ratios, floor_names, floors);

			stack.insert(stack.end(), floors.begin(), floors.end());
		} else if (obj->name == "SideFacade") {
			obj->setupProjection(2.5f, 1.0f);
			obj->setTexture("textures/brick.jpg");

			// 分割比率、名前をセット
			std::vector<float> floor_ratios(3);
			floor_ratios[0] = 4.0f / obj->scope.y;
			floor_ratios[1] = 3.5f / obj->scope.y;
			floor_ratios[2] = 3.5f / obj->scope.y;
			std::vector<std::string> floor_names(3);
			floor_names[0] = "Floor";
			floor_names[1] = "Floor";
			floor_names[2] = "Floor";

			std::vector<Object*> floors;
			obj->split(DIRECTION_Y, floor_ratios, floor_names, floors);

			stack.insert(stack.end(), floors.begin(), floors.end());
		} else if (obj->name == "Floor") {
			std::vector<float> tile_ratios;
			std::vector<std::string> tile_names;

			// 分割比率、名前をセット
			float margin_ratio = 1.0f / obj->scope.x;
			int numTiles = (obj->scope.x - 2.0f) / 3.0f;
			float tile_width_ratio = (obj->scope.x - 2.0f) / numTiles / obj->scope.x;

			tile_ratios.push_back(margin_ratio);
			tile_names.push_back("Wall");
			for (int i = 0; i < numTiles; ++i) {
				tile_ratios.push_back(tile_width_ratio);
				tile_names.push_back("Tile");
			}
			tile_ratios.push_back(margin_ratio);
			tile_names.push_back("Wall");

			std::vector<Object*> tiles;
			obj->split(DIRECTION_X, tile_ratios, tile_names, tiles);

			stack.insert(stack.end(), tiles.begin(), tiles.end());
		} else if (obj->name == "Tile") {
			std::vector<float> wall_ratios(3);
			wall_ratios[0] = 0.1f;
			wall_ratios[1] = 0.8f;
			wall_ratios[2] = 0.1f;
			std::vector<std::string> wall_names(3);
			wall_names[0] = "Wall";
			wall_names[1] = "WallWindowWall";
			wall_names[2] = "Wall";

			std::vector<Object*> walls;
			obj->split(DIRECTION_X, wall_ratios, wall_names, walls);

			stack.insert(stack.end(), walls.begin(), walls.end());
		} else if (obj->name == "WallWindowWall") {
			std::vector<float> wall_ratios(3);
			wall_ratios[0] = 0.2f;
			wall_ratios[1] = 0.6f;
			wall_ratios[2] = 0.2f;
			std::vector<std::string> wall_names(3);
			wall_names[0] = "Wall";
			wall_names[1] = "Window";
			wall_names[2] = "Wall";

			std::vector<Object*> walls;
			obj->split(DIRECTION_Y, wall_ratios, wall_names, walls);

			stack.insert(stack.end(), walls.begin(), walls.end());
		} else if (obj->name == "Window") {
			obj->translate(glm::vec3(0, 0, -0.25f));
			obj->setupProjection(obj->scope.x, obj->scope.y);
			obj->setTexture("textures/window.jpg");

			Object* obj2 = obj->clone();
			obj2->name = "Window_1";
			stack.push_back(obj2);
		} else {
			obj->generate(renderManager);
			delete obj;
		}
	}
}

void CGA::generateVase1(RenderManager* renderManager) {
	std::list<Object*> stack;
	
	std::vector<glm::vec2> points;
	points.push_back(glm::vec2(0, 6));
	points.push_back(glm::vec2(38, 12));
	Line* line = new Line("Line", glm::rotate(glm::mat4(), -M_PI * 0.5f, glm::vec3(1, 0, 0)), points, glm::vec3(1, 1, 1));
	stack.push_back(line);
	
	while (!stack.empty()) {
		Object* obj = stack.front();
		stack.pop_front();

		if (obj->removed) {
			delete obj;
			continue;
		}
		
		if (obj->name == "Line") {
			Object* obj2 = obj->clone();
			obj2->name = "RotatedLine";
			obj2->rotate(0, -90, 0);
			stack.push_back(obj2);
		} else if (obj->name == "RotatedLine") {
			stack.push_back(obj->revolve("Vase", REVOLVE_X));
		} else {
			obj->generate(renderManager);
			delete obj;
		}
	}
}

void CGA::generateVase2(RenderManager* renderManager) {
	std::list<Object*> stack;
	
	std::vector<glm::vec2> points;
	for (int i = 0; i <= 30; ++i) {
		float y = 6.0f + 6.0f * sinf((float)i / 30 * M_PI * 0.5);
		points.push_back(glm::vec2(i, y));
	}
	points.push_back(glm::vec2(34, 8));
	points.push_back(glm::vec2(38, 12));
	Line* line = new Line("Line", glm::rotate(glm::mat4(), -M_PI * 0.5f, glm::vec3(1, 0, 0)), points, glm::vec3(1, 1, 1));
	stack.push_back(line);
	
	while (!stack.empty()) {
		Object* obj = stack.front();
		stack.pop_front();

		if (obj->removed) {
			delete obj;
			continue;
		}
		
		if (obj->name == "Line") {
			Object* obj2 = obj->clone();
			obj2->name = "RotatedLine";
			obj2->rotate(0, -90, 0);
			stack.push_back(obj2);
		} else if (obj->name == "RotatedLine") {
			stack.push_back(obj->revolve("Vase", REVOLVE_X));
		} else {
			obj->generate(renderManager);
			delete obj;
		}
	}
}

void CGA::generateVase3(RenderManager* renderManager) {
	std::list<Object*> stack;
	
	std::vector<glm::vec2> points;
	points.push_back(glm::vec2(0, 8));
	points.push_back(glm::vec2(0.5, 8));
	points.push_back(glm::vec2(0.6, 6));
	points.push_back(glm::vec2(2, 5));
	points.push_back(glm::vec2(2.1, 6));
	points.push_back(glm::vec2(3.0, 6));
	points.push_back(glm::vec2(4.5, 2));
	for (int i = 0; i < 5; ++i) {
		float x = 5.0f + (float)i / 5 * 3;
		float y = 2.0f + sinf((float)i / 5 * M_PI);
		points.push_back(glm::vec2(x, y));
	}

	//points.push_back(glm::vec2(5, 2));
	//points.push_back(glm::vec2(6, 4));
	for (int i = 0; i <= 22; ++i) {
		float y = 1.5f + 10.5f * sinf((float)i / 22 * M_PI * 0.5);
		points.push_back(glm::vec2(8 + i, y));
	}
	points.push_back(glm::vec2(34, 8));
	points.push_back(glm::vec2(38, 12));
	Line* line = new Line("Line", glm::rotate(glm::mat4(), -M_PI * 0.5f, glm::vec3(1, 0, 0)), points, glm::vec3(1, 1, 1));
	stack.push_back(line);
	
	while (!stack.empty()) {
		Object* obj = stack.front();
		stack.pop_front();

		if (obj->removed) {
			delete obj;
			continue;
		}
		
		if (obj->name == "Line") {
			Object* obj2 = obj->clone();
			obj2->name = "RotatedLine";
			obj2->rotate(0, -90, 0);
			stack.push_back(obj2);
		} else if (obj->name == "RotatedLine") {
			stack.push_back(obj->revolve("Vase", REVOLVE_X));
		} else {
			obj->generate(renderManager);
			delete obj;
		}
	}
}

void CGA::generateSaltShaker1(RenderManager* renderManager) {
	std::list<Object*> stack;
	
	Rectangle* base = new Rectangle("Base", glm::rotate(glm::mat4(), -M_PI * 0.5f, glm::vec3(1, 0, 0)), 10, 10, glm::vec3(1, 1, 1));
	stack.push_back(base);
	
	while (!stack.empty()) {
		Object* obj = stack.front();
		stack.pop_front();

		if (obj->removed) {
			delete obj;
			continue;
		}
		
		if (obj->name == "Base") {
			stack.push_back(obj->extrude("Body", 20));
		} else {
			obj->generate(renderManager);
			delete obj;
		}
	}
}

void CGA::generateSaltShaker2(RenderManager* renderManager) {
	std::list<Object*> stack;

	Rectangle* base = new Rectangle("Base", glm::rotate(glm::mat4(), -M_PI * 0.5f, glm::vec3(1, 0, 0)), 10, 10, glm::vec3(1, 1, 1));
	stack.push_back(base);
	
	while (!stack.empty()) {
		Object* obj = stack.front();
		stack.pop_front();

		if (obj->removed) {
			delete obj;
			continue;
		}
		
		if (obj->name == "Base") {
			Object* obj2 = obj->taper("Body", 20, 0.7f);
			Rectangle* front;
			std::vector<Rectangle*> sides;
			Polygon* top;
			Polygon* base;
			obj2->componentSplit("front", &front, "side", sides, "top", &top, "base", &base);
			stack.push_back(obj2);
			stack.push_back(top);
		} else if (obj->name == "top") {
			stack.push_back(obj->offset("CapBase", 0.2f));
		} else if (obj->name == "CapBase") {
			stack.push_back(obj->extrude("Cap", 6));
		} else {
			obj->generate(renderManager);
			delete obj;
		}
	}
}

}
