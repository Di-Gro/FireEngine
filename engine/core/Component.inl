#include <mono_game_types.h>

#include "ActorBase.h"
#include "ComponentMeta.h"
#include "RunMode.h"

class Component : public ActorBase {
	friend class Actor;

public:
	bool runtimeOnly = false;
	bool f_isCrashed = false;

public:
	Component() { }

	virtual void OnInit() { };
	virtual void OnStart() { };
	virtual void OnUpdate() { };
	virtual void OnDestroy() { };

	virtual void OnFixedUpdate() { };

	virtual void OnDraw() { };
	virtual void OnDrawShape(int index) { };
	
	virtual ~Component() { }

	virtual ComponentMeta GetMeta() = 0;
	virtual bool NeedRunInEditor() = 0;
	virtual bool NeedRunInPlayer() = 0;

private:
	ComponentCallbacks f_callbacks = { };
	bool f_isInited = false;
	bool f_isStarted = false;
	Actor* f_selfActor;

};

