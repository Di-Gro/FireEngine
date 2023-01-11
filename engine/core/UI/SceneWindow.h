#pragma once
#include "../imgui/imgui.h"
#include "../SimpleMath.h"

class Game;
class Scene;

class SceneWindow {
public:
	std::string sceneId = "";
	bool visible = false;

public:

	SceneWindow(const std::string& _sceneId) : sceneId(_sceneId) { }
	virtual ~SceneWindow() {}

	void Init(Game* game);
	void Draw();

	Game* game() { return m_game; }
	void focus();

	Scene* scene() { return m_scene; }
	void scene(Scene* value) { m_scene = value; }

	const std::string& name();
	std::string windowId() { return name() + "##" + sceneId; }

	Vector2 viewportSize() { return m_viewportSize; }
	Vector2 viewportPosition() { return m_viewportPosition; }
	Vector2 mouseViewportPosition() { return m_mouseViewportPosition ; }

protected:
	virtual void OnInit() { }
	virtual void AfterDrawScene() { }

private:
	Game* m_game;
	Scene* m_scene;

	bool m_needSetFocus = false;

	Vector2 m_viewportSize = {800, 800};
	Vector2 m_viewportPosition;
	Vector2 m_mouseViewportPosition;

private:
	void m_DrawRender();
	void m_UpdateViewportInfo();
};