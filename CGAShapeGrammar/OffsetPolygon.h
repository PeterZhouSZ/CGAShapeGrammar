#pragma once

#include "CGA.h"
#include "Shape.h"

namespace cga {

class OffsetPolygon : public Shape {
private:
	std::vector<glm::vec2> _points;
	float _offsetDistance;
	glm::vec2 _center;

public:
	OffsetPolygon() {}
	OffsetPolygon(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, const std::vector<glm::vec2>& points, float offsetDistance, const glm::vec3& color, const std::string& texture);
	Shape* clone(const std::string& name);
	void comp(const std::string& front_name, Shape** front, const std::string& sides_name, std::vector<Shape*>& sides, const std::string& top_name, Shape** top, const std::string& bottom_name, Shape** bottom);
	void generate(RenderManager* renderManager, bool showScopeCoordinateSystem);
};

}
