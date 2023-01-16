#pragma once

#include "Actor.h"

#include "ShaderResource.h"

#include "imgui\TextEditor\TextEditor.h"

class CameraComponent;
class Player;
class ShadowPass;
class DirectionLight;
class AmbientLight;
class PointLight;
class SpotLight;

class RenderPassUI : public Component {
	PURE_COMPONENT(RenderPassUI)

private:
	ShaderResource resourceDs;

	TextEditor editor;

private:
	std::string m_currentPath;
	bool m_editorOpened = false;

	char shaderPashBuf[255];

public:
	void OnStart() override;
	void OnUpdate() override;

	void OpenFile(const std::string& path);
	void SaveFile(const std::string& text);

private:
	void m_DrawTextEditor();
	void m_DrawTextEditorMenu();

};



