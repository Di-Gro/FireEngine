#pragma once

#include <string>

class Game;
class Scene;
class Actor;
class Component;

class SceneMenu {
public:
	static void AddActor(Scene* scene);
	static void AddLight(Scene* scene);

	static bool CanCreate(Game* game, const std::string& name);
	static int Create(Game* game, const std::string& name);

	static bool CanRename(Scene* scene, const std::string& name);
	static void Rename(Scene* scene, const std::string& name);

	static bool CanSave(Scene* scene);
	static void Save(Scene* scene);

	static bool CanSetAsStartup(Scene* scene);
	static void SetAsStartup(Scene* scene);
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

class PrefabMenu {
public:
	static bool CanCreate(Actor* actor, const std::string& name);
	static bool CanSave(Actor* actor);
	static bool CanLoad(Actor* actor);

	static void Create(Actor* actor, const std::string& name);
	static void Save(Actor* actor);
	static void Load(Actor* actor);
};

