#pragma once
#include "WME.h"
#include "AlphaMemory.h"
#include <string>
#include <vector>
class ConstTestNode
{
public:
	FieldType field_to_test;
	std::string eq_field;
	AlphaMemory* out_memory;
	std::vector<ConstTestNode*> children;
	ConstTestNode(FieldType f_type,std::string eq_field, AlphaMemory* out_memory):field_to_test(f_type),eq_field(eq_field),out_memory(out_memory){}
	static ConstTestNode* dummy()
	{
		ConstTestNode* node = new ConstTestNode(FieldType::undefined, "-", nullptr);
		return node;
	}
};
