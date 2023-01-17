#pragma once

#include <vector>
#include <unordered_map>

#include "Actor.h"
#include "CSBridge.h"

#include "AI/Rete.h"
#include "AI/Condition.h"
#include "AI/ProductionNode.h"


class AIComponent : public Component {
	COMPONENT(AIComponent);

private:
	Rete m_rete;

	std::unordered_map<int, std::vector<Condition>*> m_conditions;
	std::unordered_map<int, ProductionNode*> m_decisions;

	std::string m_text;

public:
	void OnInit() override;
	void OnDestroy() override;

	int AddWME(const char* id, const char* attr, const char* value);
	void SetWMEValue(int index, const char* value);

	void AddCondition(int decisionHash,
		ConstOrVar type1, const char* value1,
		ConstOrVar type2, const char* value2,
		ConstOrVar type3, const char* value3);

	bool Decide(int decisionHash);

	const char* GetText(int decisionHash);

};

DEC_COMPONENT(AIComponent);

FUNC(AIComponent, AddWME, int)(CppRef compRef, const char* id, const char* attr, const char* value);
FUNC(AIComponent, SetWMEValue, void)(CppRef compRef, int index, const char* value);

FUNC(AIComponent, AddCondition, void)(CppRef compRef, 
	int decisionHash,
	int type1, const char* value1, 
	int type2, const char* value2, 
	int type3, const char* value3);

FUNC(AIComponent, Decide, bool)(CppRef compRef, int decisionHash);
FUNC(AIComponent, GetText, size_t)(CppRef compRef, int decisionHash);