#include "AIComponent.h"

#include <sstream>
#include <iostream>

#include "AI/WME.h"



DEF_COMPONENT(AIComponent, Engine.AIComponent, 0, RunMode::PlayOnly) {
	//OFFSET(0, AIComponent, color);
}


void AIComponent::OnInit() {
	m_rete.alpha_top = ConstTestNode::dummy();
	m_rete.const_test_nodes.push_back(m_rete.alpha_top);
}

void AIComponent::OnDestroy() {
	for (auto pair : m_conditions) {
		delete pair.second;
	}
	for (auto pair : m_decisions) {
		if (pair.second != nullptr)
			delete pair.second;
	}
	m_conditions.clear();
	m_decisions.clear();
}

void AIComponent::UpdateRete() {
	Rete new_rete;
	new_rete.alpha_top = ConstTestNode::dummy();
	new_rete.const_test_nodes.push_back(new_rete.alpha_top);
	for (auto i = 0; i < m_rete.working_memory.size(); i++) {
		new_rete.addWME(m_rete.working_memory[i]);
	}
	m_rete = (std::move(new_rete));
}

int AIComponent::AddWME(const char* id, const char* attr, const char* value) {
	return m_rete.addWME(new WME(id, attr, value));
}

void AIComponent::SetWMEValue(int index, const char* value) {
	m_rete.setWMEvalue(index, value);
}

void AIComponent::AddCondition(
	int decisionHash,
	ConstOrVar type1, const char* value1,
	ConstOrVar type2, const char* value2,
	ConstOrVar type3, const char* value3)
{
	if (!m_conditions.contains(decisionHash)) {
		m_conditions[decisionHash] = new std::vector<Condition>();
		m_decisions[decisionHash] = nullptr;
	}

	m_conditions[decisionHash]->emplace_back(
		Field(type1, value1),
		Field(type2, value2),
		Field(type3, value3)
	);
}

bool AIComponent::Decide(int decisionHash) {
	if (!m_conditions.contains(decisionHash))
		return false;

	auto** decision = &m_decisions[decisionHash];
	if (*decision != nullptr) {
		delete *decision;
		*decision = nullptr;
	}

	std::string name = ":)";

	auto* conditions = m_conditions[decisionHash];
	*decision = m_rete.addProduction(*conditions, name);
	
	bool hasDecision = (*decision)->items.size() > 0;
	if (!hasDecision) {
		delete *decision;
		*decision = nullptr;
	}
	return hasDecision;
}

const char* AIComponent::GetText(int decisionHash) {
	m_text = "";

	if (!m_decisions.contains(decisionHash))
		return m_text.c_str();

	auto decision = m_decisions[decisionHash];

	std::stringstream sout;
	for (int i = 0; i < decision->items.size(); i++) {
		auto item = decision->items[i];

		sout << i << ": ";
		sout << item->wme->get_field(FieldType::id) << ", ";
		sout << item->wme->get_field(FieldType::attr) << ", ";
		sout << item->wme->get_field(FieldType::value) << ", ";
		sout << "\n";
	}
	m_text = sout.str();
	return m_text.c_str();
}


DEF_FUNC(AIComponent, AddWME, int)(CppRef compRef, const char* id, const char* attr, const char* value) {
	return CppRefs::ThrowPointer<AIComponent>(compRef)->AddWME(id, attr, value);
}

DEF_FUNC(AIComponent, SetWMEValue, void)(CppRef compRef, int index, const char* value) {
	CppRefs::ThrowPointer<AIComponent>(compRef)->SetWMEValue(index, value);
}

DEF_FUNC(AIComponent, AddCondition, void)(CppRef compRef,
	int decisionHash,
	int type1, const char* value1,
	int type2, const char* value2,
	int type3, const char* value3) 
{
	auto aiComponent = CppRefs::ThrowPointer<AIComponent>(compRef);
	aiComponent->AddCondition(
		decisionHash,
		(ConstOrVar)type1, value1,
		(ConstOrVar)type2, value2,
		(ConstOrVar)type3, value3
	);
}

DEF_FUNC(AIComponent, Decide, bool)(CppRef compRef, int decisionHash) {
	return CppRefs::ThrowPointer<AIComponent>(compRef)->Decide(decisionHash);
}

DEF_FUNC(AIComponent, GetText, size_t)(CppRef compRef, int decisionHash) {
	return (size_t)CppRefs::ThrowPointer<AIComponent>(compRef)->GetText(decisionHash);
}

DEF_FUNC(AIComponent, UpdateRete, void)(CppRef compRef) {
       CppRefs::ThrowPointer<AIComponent>(compRef)->UpdateRete();
}