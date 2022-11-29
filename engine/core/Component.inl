
class Component : public GameObjectBase {
public:
	Component() { }

	virtual void OnInit() { };
	virtual void OnStart() { };
	virtual void OnUpdate() {};
	virtual void OnDestroy() {};

	virtual void OnDraw() {};
	virtual void OnDrawDebug() {};
	virtual void OnDrawUI() {};
	
	virtual ~Component() {}

};

