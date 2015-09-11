#include "Polygon.h"
#include "Pyramid.h"
#include "HipRoof.h"
#include "OffsetPolygon.h"
#include "Prism.h"
#include "GLUtils.h"

namespace cga {

Polygon::Polygon(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, const std::vector<glm::vec2>& points, const glm::vec3& color, const std::string& texture) {
	this->_name = name;
	this->_removed = false;
	this->_pivot = pivot;
	this->_modelMat = modelMat;
	this->_points = points;
	this->_color = color;
	this->_texture = texture;

	BoundingBox bbox(points);
	this->_scope = glm::vec3(bbox.maxPt.x, bbox.maxPt.y, 0);

	this->_center = glm::vec2(0, 0);
	for (int i = 0; i < points.size(); ++i) {
		_center += points[i];
	}
	_center /= points.size();
}

Shape* Polygon::clone(const std::string& name) {
	Shape* copy = new Polygon(*this);
	copy->_name = name;
	return copy;
}

Shape* Polygon::extrude(const std::string& name, float height) {
	return new Prism(name, _pivot, _modelMat, _points, height, _color);
}

Shape* Polygon::inscribeCircle(const std::string& name) {
	return NULL;
}

Shape* Polygon::offset(const std::string& name, float offsetDistance) {
	return new OffsetPolygon(name, _pivot, _modelMat, _points, offsetDistance, _color, _texture);
}

Shape* Polygon::roofHip(const std::string& name, float angle) {
	return new HipRoof(name, _pivot, _modelMat, _points, angle, _color);
}

void Polygon::setupProjection(float texWidth, float texHeight) {
	_texCoords.resize(_points.size());
	for (int i = 0; i < _points.size(); ++i) {
		_texCoords[i] = glm::vec2(_points[i].x / texWidth, _points[i].y / texHeight);
	}
}

void Polygon::size(float xSize, float ySize, float zSize) {
	float scaleX = xSize / _scope.x;
	float scaleY = ySize / _scope.y;
	for (int i = 0; i < _points.size(); ++i) {
		_points[i].x *= scaleX;
		_points[i].y *= scaleY;
	}
	_scope.x = xSize;
	_scope.y = ySize;
	_scope.z = 0.0f;
}

Shape* Polygon::taper(const std::string& name, float height, float top_ratio) {
	return new Pyramid(name, _pivot, _modelMat, _points, _center, height, top_ratio, _color, _texture);
}

void Polygon::generate(RenderManager* renderManager, bool showScopeCoordinateSystem) {
	if (_removed) return;

	std::vector<Vertex> vertices;
	glutils::drawConcavePolygon(_points, _color, _pivot * _modelMat, vertices);

	renderManager->addObject(_name.c_str(), _texture.c_str(), vertices);

	if (showScopeCoordinateSystem) {
		drawAxes(renderManager, _pivot * _modelMat);
	}
}

}