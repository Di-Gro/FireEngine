#pragma once
#include <string>
#include <vector>


enum class FieldType {
    undefined = -1,
    id = 0,
    attr = 1,
    value = 2,
    num_of_fields_type = 3,
};

class WME {
public:
    std::vector<std::string> fields;
    std::string get_field(FieldType field_type) const;
    WME(std::string id, std::string attr, std::string val);

    void SetValue(const std::string& val);
};


