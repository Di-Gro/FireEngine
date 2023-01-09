#pragma once

#include "../SimpleMath.h"
#include "../imgui/imgui.h"
#include "../CSBridge.h"
#include "../Actor.h"

#include "ComponentPicker.h"
#include "AssetPickerPopup.h"

class Game;
class Actor;
class UserInterface;
class Component;

class UI_Inspector
{
public:
	static const char* ComponentDragType;

	static char textBuffer[1024];
	float widthComponent = 0;
	CsRef csRef;

private:
	ImGuiTreeNodeFlags treeNodeFlags = 0
		| ImGuiTreeNodeFlags_DefaultOpen
		| ImGuiTreeNodeFlags_OpenOnArrow
		| ImGuiTreeNodeFlags_OpenOnDoubleClick
		| ImGuiTreeNodeFlags_Framed
		;

	ImGuiTreeNodeFlags collapsingHeaderFlags = 0
		| ImGuiTreeNodeFlags_DefaultOpen
		| ImGuiTreeNodeFlags_Framed
		;

	Game* _game;
	UserInterface* _ui;
	bool isLocal = false;

	float m_dropTargetHeight = 0.0f;
	float m_mousePosY = 0.0f;
	float m_closestHeight = 0.0f;
	bool m_dragTargetNodeOpen = false;

	ImVec2 m_compSpacing = { 0, 8 };
	ImVec2 m_lineSpacing = { 0, 3 };

	ImVec4 m_dragTargetColor = { 0.11f, 0.64f, 0.92f, 0.60f };
	ImVec4 m_dragTargetColorHovered = { 0.11f, 0.64f, 0.92f, 1.00f };

	ComponentPicker m_componentPicker;
	AssetPickerPopup m_assetPickerPopup;

	std::string m_componentName;

	CsRef m_focusedRef;

public:
	void Draw_UI_Inspector();
	void Init(Game* game, UserInterface* ui);

	void DrawActorTransform();
	void DrawActorComponents();

	void AddComponent();

	//Actor* GetDroppedActor();

	bool ButtonCenteredOnLine(const char* label, float alignment = 0.5f);

	bool ShowVector3(Vector3* vec3, const std::string& title);
	bool ShowAsset(const std::string& label, int scriptIdHash, int* assetIdHash);
	bool ShowActor(const std::string& label, CsRef* csRef, CppRef cppRef);
	bool ShowComponent(const std::string& label, CsRef* csRef, CppRef cppRef, int scriptIdHash);

	void BigSpace();
	void Space();

	const std::string& RequestComponentName(Component* component);

	void SetComponentName(const std::string& value) { m_componentName = value; }
	const std::string& GetComponentName() { return m_componentName; }

	bool CollapsingHeader(Component* component, const std::string& name);

	bool HasDraggedActor(CsRef* currentRef);
	bool HasDraggedComponent(int scriptIdHash, CsRef* currentRef);

	bool AcceptDroppedActor(CsRef* currentRef);
	bool AcceptDroppedComponent(int scriptIdHash, CsRef* currentRef);

private:
	void m_DrawHeader();
	void m_DrawComponents();
	void m_DrawComponent(void(UI_Inspector::* func)());
	void m_DrawTransformContent();
	void m_DrawComponentContent();
	void m_DrawAddComponent();

	void m_DrawComponentContextMenu(Component* component);

};

FUNC(UI_Inspector, ShowText, bool)(CppRef gameRef, const char* label, const char* buffer, int length, size_t* ptr);
FUNC(UI_Inspector, SetComponentName, void)(CppRef gameRef, const char* value);

FUNC(UI_Inspector, ShowAsset, bool)(CppRef gameRef, const char* label, int scriptIdHash, int* assetIdHash);
FUNC(UI_Inspector, ShowActor, bool)(CppRef gameRef, const char* label, CsRef* csRef, CppRef cppRef);
FUNC(UI_Inspector, ShowComponent, bool)(CppRef gameRef, const char* label, CsRef* csRef, CppRef cppRef, int scriptIdHash);

FUNC(ImGui, CalcTextWidth, float)(const char* value);
