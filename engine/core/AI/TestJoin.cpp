#include "TestJoin.h"

bool TestJoin::operator==(const TestJoin& other_node) {
    return field_of_arg1 == other_node.field_of_arg1 &&
        field_of_arg2 == other_node.field_of_arg2 &&
        condition_number_of_arg2 == other_node.condition_number_of_arg2;
}