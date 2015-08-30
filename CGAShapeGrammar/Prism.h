#pragma once

#include "CGA.h"

namespace cga {

class Prism : public Shape {
private:
	std::vector<glm::vec2> _points;
	float _height;

public:
	Prism() {}
	Prism(const std::string& name, const glm::mat4& modelMat, const std::vector<glm::vec2>& points, float height, const glm::vec3& color);
	Shape* clone(const std::string& name);
	void setupProjection(float texWidth, float texHeight);
	void split(int direction, const std::vector<float> ratios, const std::vector<std::string> names, std::vector<Shape*>& objects);
	void comp(const std::string& front_name, Shape** front, const std::string& sides_name, std::vector<Shape*>& sides, const std::string& top_name, Shape** top, const std::string& bottom_name, Shape** bottom);
	void generate(RenderManager* renderManager, bool showAxes);
};

}
