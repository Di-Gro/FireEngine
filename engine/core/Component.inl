#include <mono_game_types.h>

#include "ActorBase.h"
#include "ComponentMeta.h"


class Component : public ActorBase {
	friend class Actor;

public:
	Component() { }

	virtual void OnInit() { };
	virtual void OnStart() { };
	virtual void OnUpdate() {};
	virtual void OnDestroy() {};

	virtual void OnDraw() {};
	virtual void OnDrawShape(int index) {};
	virtual void OnDrawDebug() {};
	virtual void OnDrawUI() {};
	
	virtual ~Component() {}

	virtual ComponentMeta GetMeta() = 0;

private:
	ComponentCallbacks m_callbacks = {};

};

