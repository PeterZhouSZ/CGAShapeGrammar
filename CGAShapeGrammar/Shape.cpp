#include "Shape.h"
#include "OBJLoader.h"
#include "GeneralObject.h"
#include "GLUtils.h"
#include <iostream>
#include <sstream>

namespace cga {

Shape* Shape::clone(const std::string& name) {
	throw "clone() is not supported.";
}

void Shape::comp(const std::map<std::string, std::string>& name_map, std::vector<Shape*>& shapes) {
	throw "comp() is not supported.";
}

Shape* Shape::extrude(const std::string& name, float height) {
	throw "extrude() is not supported.";
}

Shape* Shape::inscribeCircle(const std::string& name) {
	throw "inscribeCircle() is not supported.";
}

Shape* Shape::insert(const std::string& name, const std::string& geometryPath) {
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	if (!OBJLoader::load(geometryPath.c_str(), points, normals, texCoords)) {
		std::stringstream ss;
		ss << "OBJ file cannot be read: " << geometryPath.c_str() << "." << std::endl;
		std::cout << ss.str() << std::endl;
		throw ss.str();
	}

	// compute scale
	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float scaleZ = 1.0f;

	BoundingBox bbox(points);
	if (_scope.x != 0 && _scope.y != 0 && _scope.z != 0) {			// all non-zero
		scaleX = _scope.x / bbox.sx();
		scaleY = _scope.y / bbox.sy();
		scaleZ = _scope.z / bbox.sz();
	} else if (_scope.x == 0 && _scope.y != 0 && _scope.z != 0) {	// sx == 0
		scaleY = _scope.y / bbox.sy();
		scaleZ = _scope.z / bbox.sz();
		scaleX = (scaleY + scaleZ) * 0.5f;
	} else if (_scope.x != 0 && _scope.y == 0 && _scope.z != 0) {	// sy == 0
		scaleX = _scope.x / bbox.sx();
		scaleZ = _scope.z / bbox.sz();
		scaleY = (scaleX + scaleZ) * 0.5f;
	} else if (_scope.x != 0 && _scope.y != 0 && _scope.z == 0) {	// sz == 0
		scaleX = _scope.x / bbox.sx();
		scaleY = _scope.y / bbox.sy();
		scaleZ = (scaleX + scaleY) * 0.5f;
	} else if (_scope.x != 0) {										// sy == 0 && sz == 0
		scaleX = _scope.x / bbox.sx();
		scaleY = scaleX;
		scaleZ = scaleX;
	} else if (_scope.y != 0) {										// sx == 0 && sz == 0
		scaleY = _scope.y / bbox.sy();
		scaleX = scaleY;
		scaleZ = scaleY;
	} else if (_scope.z != 0) {										// sx == 0 && sy == 0
		scaleZ = _scope.z / bbox.sz();
		scaleX = scaleZ;
		scaleY = scaleZ;
	} else { // all zero
		// do nothing
	}

	// scale the points
	for (int i = 0; i < points.size(); ++i) {
		points[i].x = (points[i].x - bbox.minPt.x) * scaleX;
		points[i].y = (points[i].y - bbox.minPt.y) * scaleY;
		points[i].z = (points[i].z - bbox.minPt.z) * scaleZ;
	}

	// if texCoords are not defined in obj file, generate them automatically.
	if (_texCoords.size() > 0 && texCoords.size() == 0) {
		texCoords.resize(points.size());
		for (int i = 0; i < points.size(); ++i) {
			texCoords[i].x = points[i].x / _scope.x * (_texCoords[1].x - _texCoords[0].x) + _texCoords[0].x;
			texCoords[i].y = points[i].y / _scope.y * (_texCoords[2].y - _texCoords[0].y) + _texCoords[0].y;
		}
	}

	if (texCoords.size() > 0) {
		return new GeneralObject(name, _pivot, _modelMat, points, normals, _color, texCoords, _texture);
	} else {
		return new GeneralObject(name, _pivot, _modelMat, points, normals, _color);
	}
}

void Shape::nil() {
	_removed = true;
}

Shape* Shape::offset(const std::string& name, float offsetDistance) {
	throw "offset() is not supported.";
}

Shape* Shape::roofHip(const std::string& name, float angle) {
	throw "roofHip() is not supported.";
}

void Shape::rotate(const std::string& name, float xAngle, float yAngle, float zAngle) {
	_modelMat = glm::rotate(_modelMat, xAngle * M_PI / 180.0f, glm::vec3(1, 0, 0));
	_modelMat = glm::rotate(_modelMat, yAngle * M_PI / 180.0f, glm::vec3(0, 1, 0));
	_modelMat = glm::rotate(_modelMat, zAngle * M_PI / 180.0f, glm::vec3(0, 0, 1));
}

void Shape::setupProjection(int axesSelector, float texWidth, float texHeight) {
	throw "setupProjection() is not supported.";
}

Shape* Shape::shapeL(const std::string& name, float frontWidth, float leftWidth) {
	throw "shapeL() is not supported.";
}

void Shape::size(float xSize, float ySize, float zSize) {
	throw "size() is not supported.";
}

void Shape::split(int splitAxis, const std::vector<float>& sizes, const std::vector<std::string>& names, std::vector<Shape*>& objects) {
	throw "split() is not supported.";
}

Shape* Shape::taper(const std::string& name, float height, float top_ratio) {
	throw "taper() is not supported.";
}

void Shape::texture(const std::string& tex) {
	this->_texture = tex;
	_textureEnabled = true;
}

void Shape::translate(int mode, int coordSystem, float x, float y, float z) {
	if (mode == MODE_ABSOLUTE) {
		if (coordSystem == COORD_SYSTEM_WORLD) {
			_modelMat[3].x = x;
			_modelMat[3].y = y;
			_modelMat[3].z = z;
		} else if (coordSystem == COORD_SYSTEM_OBJECT) {
			_modelMat = glm::translate(_modelMat, glm::vec3(x, y, z));
		}
	} else if (mode == MODE_RELATIVE) {
		if (coordSystem == COORD_SYSTEM_WORLD) {
			_modelMat[3].x += x;
			_modelMat[3].y += y;
			_modelMat[3].z += z;
		} else if (coordSystem == COORD_SYSTEM_OBJECT) {
			_modelMat = glm::translate(_modelMat, glm::vec3(x, y, z));
		}
	}
}

void Shape::generate(RenderManager* renderManager, bool showScopeCoordinateSystem) {
	throw "generate() is not supported.";
}

void Shape::drawAxes(RenderManager* renderManager, const glm::mat4& modelMat) {
	std::vector<Vertex> vertices;
	glutils::drawAxes(0.1, 3, modelMat, vertices);
	renderManager->addObject("axis", "", vertices);
}

}