#pragma once

#include "Rule.h"
#include <glm/gtc/matrix_transform.hpp>

namespace cga {

class TranslateOperator : public Operator {
private:
	glm::vec3 vec;

public:
	TranslateOperator(const glm::vec3& vec);
	Shape* apply(Shape* obj, std::list<Shape*>& stack);
};

}
