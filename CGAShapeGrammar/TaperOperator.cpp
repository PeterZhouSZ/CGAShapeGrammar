#include "TaperOperator.h"
#include "CGA.h"
#include "Shape.h"

namespace cga {

TaperOperator::TaperOperator(float height, float top_ratio) {
	this->name = "taper";
	this->height = height;
	this->top_ratio = top_ratio;
}

Shape* TaperOperator::apply(Shape* obj, const RuleSet& ruleSet, std::list<Shape*>& stack) {
	Shape* new_obj = obj->taper(obj->_name, height, top_ratio);
	delete obj;
	return new_obj;
}

}