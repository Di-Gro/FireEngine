#pragma once

#include "WME.h"

class TestJoin {
public:
	FieldType field_of_arg1, field_of_arg2;
	int condition_number_of_arg2;
	bool operator==(const TestJoin& other_node);
};
