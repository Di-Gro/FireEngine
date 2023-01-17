#pragma once
#include <string>
#include "WME.h"


enum class ConstOrVar
{
	Const = 0,
	Var = 1,
};

struct Field
{
    ConstOrVar type;
    std::string v;

    static Field var(const std::string& name) {
        return Field(ConstOrVar::Var, name);
    }

    static Field constant(const std::string& name) {
        return Field(ConstOrVar::Const, name);
    }

    Field() {}
    Field(ConstOrVar type, const std::string& name) {
        this->type = type;
        this->v = name; 
    }
};




class Condition
{
public:
    Field attrs[(int)FieldType::num_of_fields_type];

    Condition(Field ident, Field attr, Field val) {
        attrs[0] = ident; attrs[1] = attr; attrs[2] = val;
    }

};
