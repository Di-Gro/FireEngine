#include "WME.h"
#include<assert.h>

WME::WME(std::string id, std::string attr, std::string val) {
    fields.resize(3);
    fields[0] = id;
    fields[1] = attr;
    fields[2] = val;
}

void WME::SetValue(const std::string& val) {
    fields[2] = val;
}

std::string WME::get_field(FieldType field_type) const {
    assert(field_type != FieldType::undefined);
    return fields[(int)field_type];
}