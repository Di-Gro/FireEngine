// Rete_AI.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Rete.h"

void test_rete()
{
    using namespace std;

    Rete rete;
    rete.alpha_top = ConstTestNode::dummy();
    rete.const_test_nodes.push_back(rete.alpha_top);

    rete.addWME(new WME("B1", "on", "B2"));
    rete.addWME(new WME("B2", "left", "B3"));
    rete.addWME(new WME("B3", "color", "red"));
    rete.addWME(new WME("B4", "on", "B5"));

    vector<Condition> conds;
    conds.push_back(Condition(Field::var("x"), Field::constant("on"), Field::var("y")));
    conds.push_back(Condition(Field::var("y"), Field::constant("left"), Field::var("z")));
    conds.push_back(Condition(Field::var("z"), Field::constant("color"), Field::var("m")));
    
  
    auto* production = rete.addProduction(conds, "prod1");
  
    for (auto i : production->items)
    {
        std::cout << i->wme->get_field(FieldType::id) << i->wme->get_field(FieldType::attr) << i->wme->get_field(FieldType::value) << "\n";
    }
    cout << "---\n";


    cout << "====\n";
}


