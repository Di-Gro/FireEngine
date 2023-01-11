#pragma once

class Game;
class Scene;
class Actor;
class Component;

class SceneMenu {
public:
	static void AddActor(Scene* scene);
	static void AddLight(Scene* scene);
};

class ActorMenu {
public:
	static void AddChild(Actor* actor);
	static void Remove(Actor* actor);
	static void Copy(Actor* actor);
	static Actor* Paste(Game* game);
};

class ComponentMenu {
public:
	static void Remove(Component* component);
	static void Copy(Component* component);
	static Component* Paste(Actor* actor);
	static bool CanPaste(Game* game);
};

