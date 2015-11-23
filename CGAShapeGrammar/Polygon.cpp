#include "Polygon.h"
#include "Pyramid.h"
#include "GableRoof.h"
#include "HipRoof.h"
#include "Prism.h"
#include "CGA.h"
#include "GLUtils.h"

namespace cga {

Polygon::Polygon(const std::string& name, const std::string& grammar_type, const glm::mat4& pivot, const glm::mat4& modelMat, const std::vector<glm::vec2>& points, const glm::vec3& color, const std::string& texture) {
	this->_active = true;
	this->_name = name;
	this->_grammar_type = grammar_type;
	this->_pivot = pivot;
	this->_modelMat = modelMat;
	this->_points = points;
	this->_color = color;
	this->_texture = texture;

	glutils::BoundingBox bbox(points);
	this->_scope = glm::vec3(bbox.maxPt.x, bbox.maxPt.y, 0);

	this->_center = glm::vec2(0, 0);
	for (int i = 0; i < points.size(); ++i) {
		_center += points[i];
	}
	_center /= points.size();
}

boost::shared_ptr<Shape> Polygon::clone(const std::string& name) const {
	boost::shared_ptr<Shape> copy = boost::shared_ptr<Shape>(new Polygon(*this));
	copy->_name = name;
	return copy;
}

boost::shared_ptr<Shape> Polygon::extrude(const std::string& name, float height) {
	return boost::shared_ptr<Shape>(new Prism(name, _grammar_type, _pivot, _modelMat, _points, height, _color));
}

boost::shared_ptr<Shape> Polygon::inscribeCircle(const std::string& name) {
	return NULL;
}

void Polygon::offset(const std::string& name, float offsetDistance, const std::string& inside, const std::string& border, std::vector<boost::shared_ptr<Shape> >& shapes) {
	std::vector<glm::vec2> offset_points;
	glutils::offsetPolygon(_points, offsetDistance, offset_points);

	// inner shape
	if (!inside.empty()) {
		shapes.push_back(boost::shared_ptr<Shape>(new Polygon(name, _grammar_type, _pivot, _modelMat, offset_points, _color, _texture)));
	}

	// border shape
	if (!border.empty()) {
		std::vector<glm::vec3> pts;
		std::vector<glm::vec3> normals;
		for (int i = 0; i < _points.size(); ++i) {
			pts.push_back(glm::vec3(offset_points[i], 0));
			pts.push_back(glm::vec3(_points[i], 0));
			pts.push_back(glm::vec3(_points[(i+1) % _points.size()], 0));
			pts.push_back(glm::vec3(offset_points[(i+1) % offset_points.size()], 0));

			float rot_z = atan2f(_points[(i + 1) % _points.size()].y - _points[i].y, _points[(i + 1) % _points.size()].x - _points[i].x);
			glm::mat4 convMat = glm::rotate(glm::translate(glm::mat4(), glm::vec3(_points[i], 0)), rot_z, glm::vec3(0, 0, 1));
			glm::mat4 invMat = glm::inverse(convMat);
			glm::mat4 mat = _modelMat * convMat;

			std::vector<glm::vec2> pts2d;
			pts2d.push_back(glm::vec2(invMat * glm::vec4(pts[0], 1)));
			pts2d.push_back(glm::vec2(invMat * glm::vec4(pts[1], 1)));
			pts2d.push_back(glm::vec2(invMat * glm::vec4(pts[2], 1)));
			pts2d.push_back(glm::vec2(invMat * glm::vec4(pts[3], 1)));

			shapes.push_back(boost::shared_ptr<Shape>(new Polygon(name, _grammar_type, _pivot, mat, pts2d, _color, _texture)));
		}		
	}
}

boost::shared_ptr<Shape> Polygon::roofHip(const std::string& name, float angle) {
	return boost::shared_ptr<Shape>(new HipRoof(name, _grammar_type, _pivot, _modelMat, _points, angle, _color));
}

boost::shared_ptr<Shape> Polygon::roofGable(const std::string& name, float angle) {
	return boost::shared_ptr<Shape>(new GableRoof(name, _grammar_type, _pivot, _modelMat, _points, angle, _color));
}

void Polygon::setupProjection(int axesSelector, float texWidth, float texHeight) {
	if (axesSelector == AXES_SCOPE_XY) {
		_texCoords.resize(_points.size());
		for (int i = 0; i < _points.size(); ++i) {
			_texCoords[i] = glm::vec2(_points[i].x / texWidth, _points[i].y / texHeight);
		}
	}
	else {
		throw "Polygon supports only scope.xy for setupProjection().";
	}
}

void Polygon::size(float xSize, float ySize, float zSize) {
	_prev_scope = _scope;

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

boost::shared_ptr<Shape> Polygon::taper(const std::string& name, float height, float top_ratio) {
	return boost::shared_ptr<Shape>(new Pyramid(name, _grammar_type, _pivot, _modelMat, _points, _center, height, top_ratio, _color, _texture));
}

void Polygon::generateGeometry(std::vector<boost::shared_ptr<glutils::Face> >& faces, float opacity) const {
	if (!_active) return;

	if (!_texture.empty() && _texCoords.size() >= _points.size()) {
		std::vector<Vertex> vertices;
		glutils::drawConcavePolygon(_points, glm::vec4(_color, opacity), _texCoords, _pivot * _modelMat, vertices);

		faces.push_back(boost::shared_ptr<glutils::Face>(new glutils::Face(_name, _grammar_type, vertices, _texture)));
	} else {
		std::vector<Vertex> vertices;
		glutils::drawConcavePolygon(_points, glm::vec4(_color, opacity), _pivot * _modelMat, vertices);

		faces.push_back(boost::shared_ptr<glutils::Face>(new glutils::Face(_name, _grammar_type, vertices)));
	}
}

}
