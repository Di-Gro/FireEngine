#include "UI_Inspector.h"

#include <iostream>
#include <algorithm>
#include <list>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imguizmo/ImGuizmo.h"

#include "../Game.h"
#include "../Actor.h"
#include "../Assets.h"
#include "../AssetStore.h"
#include "UserInterface.h"
#include "UI_Hierarchy.h"
#include "ComponentPicker.h"
#include "../Render.h"
#include "../HotKeys.h"
#include "../AssetStore.h"

#include "../CameraComponent.h"
#include "../ContextMenu.h"

const char* UI_Inspector::ComponentDragType = "Inspector.Actor";
char UI_Inspector::textBuffer[1024] = { 0 };


void UI_Inspector::Init(Game* game, UserInterface* ui)
{
	_game = game;
	_ui = ui;
}

bool UI_Inspector::ButtonCenteredOnLine(const char* label, float alignment)
{
	ImGuiStyle& style = ImGui::GetStyle();

	float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
	float avail = ImGui::GetContentRegionAvail().x;

	float off = (avail - size) * alignment;
	if (off > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

	return ImGui::Button(label);
}

void UI_Inspector::BigSpace() {
	ImGui::Dummy(m_compSpacing);
}

void UI_Inspector::Space() {
	ImGui::Dummy(m_lineSpacing);
}

void UI_Inspector::Draw_UI_Inspector() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 3.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });

	if (ImGui::Begin("Inspector")) {
		if (_ui->HasActor()) {

			m_DrawComponent(&UI_Inspector::m_DrawHeader);

			if (ImGui::CollapsingHeader("Transform", collapsingHeaderFlags)) {
				m_DrawComponent(&UI_Inspector::m_DrawTransformContent);
			}
			m_DrawComponents();
			m_DrawAddComponent();
		}
	}

	if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
		m_dropTargetHeight = 1.1f;

	ImGui::End();
	ImGui::PopStyleVar(4);
}

void UI_Inspector::m_DrawAddComponent() {

	BigSpace();

	ImGui::PushStyleColor(ImGuiCol_Text, { 0.4f, 0.4f ,0.4f ,1.0f });

	auto headerText = "(Right click to Add Component)";
	auto windowWidth = ImGui::GetWindowSize().x;
	auto textSize = ImGui::CalcTextSize(headerText);

	ImGui::SetCursorPosX((windowWidth - textSize.x) * 0.5f);
	ImGui::Text(headerText);

	ImGui::PopStyleColor(1);

	auto actor = _game->ui()->GetActor();
	if (ImGui::GetMousePos().y >= ImGui::GetCursorPosY() + 20 || m_componentPicker.isOpen()) {

		m_componentPicker.header = "AddComponent";
		m_componentPicker.flags = ImGuiPopupFlags_MouseButtonRight;

		if (m_componentPicker.Open(_game)) {
			std::cout << m_componentPicker.selected << "\n";

			actor->AddComponent<Component>(m_componentPicker.selected);
			_game->assets()->MakeDirty(actor->scene()->assetIdHash());
		}
	}

	ImGui::Dummy({ 0, 100 });
}

void UI_Inspector::m_DrawTransformContent()
{
	auto actor = _ui->GetActor();
	auto matrix = (ImGuizmo::matrix_t&)actor->GetLocalMatrix();

	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(matrix.m16, matrixTranslation, matrixRotation, matrixScale);

	bool changed = false;

	changed |= ShowVector3((Vector3*)matrixTranslation, "Position");
	Space();
	changed |= ShowVector3((Vector3*)matrixRotation, "Rotation");
	Space();
	changed |= ShowVector3((Vector3*)matrixScale, "Scale");

	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix.m16);
	actor->SetLocalMatrix((Matrix)matrix);

	if (changed)
		_game->assets()->MakeDirty(actor->scene()->assetIdHash());
}

void UI_Inspector::m_DrawHeader() {

	auto actor = _game->ui()->GetActor();
	bool isActiveSelf = actor->activeSelf();

	const auto bufSize = 80;
	char nameBuffer[bufSize] = { 0 };
	auto name = actor->name();
	std::memcpy(&nameBuffer, name.c_str(), min(name.size(), bufSize));

	auto id = "ID: " + std::to_string(actor->Id());

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 10.0f, 0.0f });

	ImGui::Indent(10);
	if(ImGui::Checkbox("##ObjectIsActive", &isActiveSelf))
		actor->activeSelf(isActiveSelf);

	ImGui::SameLine();
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 10.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 3.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });

		ImGui::PushStyleColor(ImGuiCol_PopupBg, { 0.7f, 0.7f ,0.7f ,1.0f });
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f ,0.0f ,1.0f });
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.8f, 0.8f ,0.9f ,1.0f });
		ImGui::PushStyleColor(ImGuiCol_Header, { 0.2f, 0.2f, 0.4f, 0.4f });

		_ui->_callbacks.onDrawActorTags(actor->csRef());

		ImGui::PopStyleVar(6);
		ImGui::PopStyleColor(4);
	}
	ImGui::SameLine();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 10);
	ImGui::InputText("##ObjectName", nameBuffer, bufSize);

	if (ImGui::IsItemDeactivatedAfterEdit()) {
		std::string newName = nameBuffer;
		if (newName.size() != 0) {
			actor->name(newName);
			_game->assets()->MakeDirty(actor->scene()->assetIdHash());
		}
	}
	ImGui::PopStyleVar(1);

	ImGui::Unindent(10);

	widthComponent = ImGui::GetContentRegionAvail().x;
	m_DrawComponent(&UI_Inspector::m_DrawActorHeader);

	
}

bool UI_Inspector::CollapsingHeader(Component* component, const std::string& name) {
	bool result = false;

	bool isCrashed = component->f_isCrashed;
	auto nodeId = name + "##" + std::to_string(component->cppRef().value);

	if (isCrashed) {
		ImGui::PushStyleColor(ImGuiCol_Header, { 0.4f, 0.0f, 0.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.5f, 0.0f, 0.0f, 1.0f });
	}

	ImVec2 lastCursor = ImGui::GetCursorPos();
	result = ImGui::CollapsingHeader(nodeId.c_str(), collapsingHeaderFlags);
	ImVec2 nextCursor = ImGui::GetCursorPos();

	m_DrawComponentContextMenu(component);
	if (component->IsDestroyed())
		return false;

	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload(UI_Inspector::ComponentDragType, &component, sizeof(Component*));
		ImGui::EndDragDropSource();
	}

	if (isCrashed) {
		auto textSize = ImGui::CalcTextSize("Crashed");
		auto itemSize = ImGui::GetItemRectSize();

		lastCursor.x = itemSize.x - textSize.x - 10;
		lastCursor.y += (itemSize.y - textSize.y) / 2;

		ImGui::SetCursorPos(lastCursor);
		ImGui::Text("Crashed");
		ImGui::SetCursorPos(nextCursor);

		ImGui::PopStyleColor(2);
	}
	return result;
}

void UI_Inspector::m_DrawComponents()
{
	auto actor = _ui->GetActor();

	float mouseHeight = ImGui::GetMousePos().y;
	float cursorHeight = ImGui::GetCursorScreenPos().y;

	m_mousePosY = mouseHeight - cursorHeight;

	bool isClosest = false;

	if (m_mousePosY < m_closestHeight && cursorHeight <= mouseHeight)
	{
		isClosest = true;
		m_closestHeight = m_mousePosY;
	}

	auto list = actor->GetComponentList();

	for (auto component : *list) {
		m_closestHeight = 10'000.0f;
		m_dragTargetNodeOpen = false;

		csRef = component->csRef();

		if (!component->IsDestroyed() && csRef.value > 0) {
			auto name = RequestComponentName(component);

			bool isOpen = CollapsingHeader(component, name);
			if (isOpen) {
				widthComponent = ImGui::GetContentRegionAvail().x;
				m_groupRef = component->csRef();
				m_DrawComponent(&UI_Inspector::m_DrawComponentContent);
			}
		}
	}
}

void UI_Inspector::m_DrawComponent(void(UI_Inspector::* itemFunc)()) {
	BigSpace();
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 3.0f, 0.0f });
	ImGui::Indent(10);

	(this->*itemFunc)();

	ImGui::Unindent(10);
	ImGui::PopStyleVar(2);
	BigSpace();
	ImGui::Separator();
}

void UI_Inspector::m_DrawActorHeader() {
	ShowActorPrefab(_game->ui()->GetActor());
}

void UI_Inspector::m_DrawComponentContent() {
	_ui->_callbacks.onDrawComponent(csRef, widthComponent);
}

bool UI_Inspector::ShowVector3(Vector3* values, const std::string& title)
{
	auto tmpValues = *values;

	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnWidth(0, 80.0f);
	ImGui::Text(title.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 50.0f);

	std::string nameX = "##X_" + title;
	std::string nameY = "##Y_" + title;
	std::string nameZ = "##Z_" + title;

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0.0f });

	ImGui::Text(" X ");
	ImGui::SameLine();
	ImGui::DragFloat(nameX.c_str(), &values->x, 0.1f);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::Text(" Y ");
	ImGui::SameLine();
	ImGui::DragFloat(nameY.c_str(), &values->y, 0.1f);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::Text(" Z ");
	ImGui::SameLine();
	ImGui::DragFloat(nameZ.c_str(), &values->z, 0.1f);
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	ImGui::Columns(1);

	return tmpValues != *values;
}

const std::string& UI_Inspector::RequestComponentName(Component* component) {
	_ui->_callbacks.requestComponentName(component->csRef());
	return GetComponentName();
}

bool UI_Inspector::ShowActorPrefab(Actor* actor) {

	auto& prefabId = actor->prefabId();
	auto assetIdHash = prefabId == "" ? 0 : _game->assets()->GetCsHash(prefabId);
	auto scriptIdHash = _game->assetStore()->prefabTypeIdHash;
		
	std::string fieldName = "##Prefab" + std::to_string(m_groupRef.value);
	std::string assetName = _game->assetStore()->GetAssetName(assetIdHash);
	std::string assetType = _game->assetStore()->GetScriptName(scriptIdHash);
	std::string buttonText = assetName + " (" + assetType + ") ";

	auto& style = ImGui::GetStyle();

	float column1 = 50;
	float column2 = widthComponent - column1;
	float iconWidth = ImGui::GetFrameHeight();
	float holderWidth = column2 - iconWidth - style.ItemSpacing.x - 23;

	ImGui::Columns(2, "Prefab", false);
	ImGui::SetColumnWidth(0, column1);
	ImGui::SetColumnWidth(1, column2);

	ImGui::Text("Prefab");

	ImGui::NextColumn();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 0, 0, 0 });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 0, 0, 0 });
	ImGui::ImageButton(_ui->icPickupActor.get(), { iconWidth, iconWidth }, { 0, 0 }, { 1, 1 }, 0, { 0,0,0,0 }, { 1,1,1,0.8f });
	ImGui::PopStyleColor(3);

	ImGui::PushID(ImGui::GetID((fieldName + "##2").c_str()));
	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.0f, 0.5f });
	ImGui::Button(buttonText.c_str(), { holderWidth, iconWidth });
	ImGui::PopStyleVar(1);

	if (ImGui::IsMouseReleased(ImGuiPopupFlags_MouseButtonRight) && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
		int h = 0;

	m_prefabPickerPopup.flags = ImGuiPopupFlags_MouseButtonRight;
	bool assetWasSelected = m_prefabPickerPopup.Open(actor);
	ImGui::PopID();

	ImGui::Columns(1);

	if (assetWasSelected) {
		int newAssetIdHash = m_prefabPickerPopup.selected;
		if (newAssetIdHash != assetIdHash) {
			_game->callbacks().setPrefabId(actor->csRef(), newAssetIdHash);
			_game->assets()->MakeDirty(actor->scene()->assetIdHash());
			return true;
		}
	}
	return false;
}


bool UI_Inspector::ShowAsset(const std::string& label, int scriptIdHash, int* assetIdHash, bool isActive) {
	std::string fieldName = "##" + label + std::to_string(m_groupRef.value);
	std::string assetName = _game->assetStore()->GetAssetName(*assetIdHash);
	std::string assetType = _game->assetStore()->GetScriptName(scriptIdHash);
	std::string buttonText = assetName + " (" + assetType + ") ";

	m_assetPickerPopup.header = assetType;
	m_assetPickerPopup.scriptIdHash = scriptIdHash;

	auto& style = ImGui::GetStyle();

	float column1 = 100;
	float column2 = widthComponent - column1;
	float iconWidth = ImGui::GetFrameHeight();
	float holderWidth = column2 - iconWidth - style.ItemSpacing.x - 20;

	ImGui::Columns(2, "", false);
	ImGui::SetColumnWidth(0, column1);
	ImGui::SetColumnWidth(1, column2);

	ImGui::Text(label.c_str());
		
	ImGui::NextColumn();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 0, 0, 0 });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 0, 0, 0 });
	ImGui::ImageButton(_ui->icPickupActor.get(), { iconWidth, iconWidth }, { 0, 0 }, { 1, 1 }, 0, { 0,0,0,0 }, { 1,1,1,0.8f });
	ImGui::PopStyleColor(3);

	ImGui::PushID(ImGui::GetID((fieldName + "##2").c_str()));
	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.0f, 0.5f });
	ImGui::Button(buttonText.c_str(), { holderWidth, iconWidth });
	ImGui::PopStyleVar(1);

	if (ImGui::IsMouseReleased(ImGuiPopupFlags_MouseButtonRight) && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
		int h = 0;

	m_assetPickerPopup.flags = ImGuiPopupFlags_MouseButtonRight;
	bool assetWasSelected = m_assetPickerPopup.Open(_game);
	ImGui::PopID();

	ImGui::Columns(1);

	if (assetWasSelected) {
		int newAssetIdHash = m_assetPickerPopup.selected;
		if (newAssetIdHash != scriptIdHash) {
			*assetIdHash = newAssetIdHash;
			return true;
		}
	}
	return false;
}

bool UI_Inspector::ShowActor(const std::string& label, CsRef* csRef, CppRef cppRef) {
	auto actor = CppRefs::GetPointer<Actor>(cppRef);

	if (actor == nullptr && cppRef.value != 0)
		throw std::exception("ShowComponent: Bad actor reference");

	std::string actorId = "";
	std::string fieldName = "##" + label + std::to_string(m_groupRef.value);
	std::string actorName = csRef->value == 1 ? "Missing" : "Null"; 

	if (actor != nullptr) {
		actorId = "[" + std::to_string(actor->Id()) + "] ";
		actorName = actor->name();
	}
	std::string buttonText = actorId + actorName + " (Actor) ";

	auto& style = ImGui::GetStyle();

	float column1 = 100;
	float column2 = widthComponent - column1;
	float iconWidth = ImGui::GetFrameHeight();
	float holderWidth = column2 - iconWidth - style.ItemSpacing.x - 20;

	ImGui::Columns(2, "", false);
	ImGui::SetColumnWidth(0, column1);
	ImGui::SetColumnWidth(1, column2);

	ImGui::Text(label.c_str());

	ImGui::NextColumn();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 0, 0, 0 });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 0, 0, 0 });
	ImGui::ImageButton(_ui->icPickupActor.get(), { iconWidth, iconWidth }, { 0, 0 }, { 1, 1 }, 0, { 0,0,0,0 }, { 1,1,1,0.8f });
	ImGui::PopStyleColor(3);

	bool hasDraggedActor = HasDraggedActor(csRef);
	if (hasDraggedActor) 
		ImGui::PushStyleColor(ImGuiCol_Button, m_dragTargetColor);

	ImGui::PushID(ImGui::GetID((fieldName + "##2").c_str()));
	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.0f, 0.5f });
	ImGui::Button(buttonText.c_str(), { holderWidth, iconWidth });
	ImGui::PopStyleVar(1);
	ImGui::PopID();

	if (hasDraggedActor)
		ImGui::PopStyleColor(1);

	ImGui::Columns(1);

	if (m_DrawActorFieldContextMenu(csRef))
		return true;

	bool changed = AcceptDroppedActor(csRef);

	return changed;
}

bool UI_Inspector::ShowComponent(const std::string& label, CsRef* csRef, CppRef cppRef, int scriptIdHash) {
	auto component = CppRefs::GetPointer<Component>(cppRef);
	auto actor = component == nullptr ? nullptr : component->actor();

	if (component == nullptr && cppRef.value != 0)
		throw std::exception("ShowComponent: Bad component reference");

	std::string fieldName = "##" + label + std::to_string(m_groupRef.value);
	std::string actorName = csRef->value == 1 ? "Missing" : "Null";
	if(actor != nullptr) 
		actorName = actor->name();

	std::string assetType = _game->assetStore()->GetScriptName(scriptIdHash);
	std::string buttonText = actorName + " (" + assetType + ") ";

	auto& style = ImGui::GetStyle();

	float column1 = 100;
	float column2 = widthComponent - column1;
	float iconWidth = ImGui::GetFrameHeight();
	float holderWidth = column2 - iconWidth - style.ItemSpacing.x - 20;

	ImGui::Columns(2, "", false);
	ImGui::SetColumnWidth(0, column1);
	ImGui::SetColumnWidth(1, column2);

	ImGui::Text(label.c_str());

	ImGui::NextColumn();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 0, 0, 0 });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 0, 0, 0 });
	ImGui::ImageButton(_ui->icPickupActor.get(), { iconWidth, iconWidth }, { 0, 0 }, { 1, 1 }, 0, { 0,0,0,0 }, { 1,1,1,0.8f });
	ImGui::PopStyleColor(3);

	bool hasDraggedComponent = HasDraggedComponent(scriptIdHash, csRef);
	if (hasDraggedComponent) 
		ImGui::PushStyleColor(ImGuiCol_Button, m_dragTargetColor);
	
	ImGui::PushID(ImGui::GetID((fieldName + "##2").c_str()));
	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.0f, 0.5f });
	ImGui::Button(buttonText.c_str(), { holderWidth, iconWidth });
	ImGui::PopStyleVar(1);
	ImGui::PopID();

	if (hasDraggedComponent)
		ImGui::PopStyleColor(1);

	ImGui::Columns(1);

	if (m_DrawComponentFieldContextMenu(scriptIdHash, csRef))
		return true;

	bool changed = AcceptDroppedComponent(scriptIdHash, csRef);

	return changed;
}

bool UI_Inspector::ShowColor3(const std::string& label, Vector3* value) {

	std::string itemId = "##_" + label + "_" + std::to_string(m_groupRef.value);

	auto& style = ImGui::GetStyle();

	float column1 = 100;
	float column2 = widthComponent - column1;
	float colorWidth = column2 - style.ItemSpacing.x - 20;

	ImGui::Columns(2, "", false);
	ImGui::SetColumnWidth(0, column1);
	ImGui::SetColumnWidth(1, column2);

	ImGui::Text(label.c_str());

	ImGui::NextColumn();
	ImGui::SetNextItemWidth(colorWidth);

	auto color = Vector4(value->x, value->y, value->z, 1);
	bool changed = ImGui::ColorEdit3(itemId.c_str(), &color.x);
	*value = Vector3(color.x, color.y, color.z);

	ImGui::Columns(1);

	return changed;
}

bool UI_Inspector::ShowColor4(const std::string& label, Vector4* value) {

	std::string itemId = "##_" + label + "_" + std::to_string(m_groupRef.value);

	auto& style = ImGui::GetStyle();

	float column1 = 100;
	float column2 = widthComponent - column1;
	float colorWidth = column2 - style.ItemSpacing.x - 20;

	ImGui::Columns(2, "", false);
	ImGui::SetColumnWidth(0, column1);
	ImGui::SetColumnWidth(1, column2);

	ImGui::Text(label.c_str());

	ImGui::NextColumn();
	ImGui::SetNextItemWidth(colorWidth);

	bool changed = ImGui::ColorEdit4(itemId.c_str(), &value->x);

	ImGui::Columns(1);

	return changed;
}

bool UI_Inspector::HasDraggedActor(CsRef* currentRef) {
	auto dataType = UI_Hierarchy::ActorDragType;

	auto payload = ImGui::GetDragDropPayload();
	if (payload != nullptr && payload->IsDataType(dataType)) {

		auto draggedActor = *(Actor**)payload->Data;
		return draggedActor != nullptr;
	}
	return false;
}

bool UI_Inspector::HasDraggedComponent(int scriptIdHash, CsRef* currentRef) {
	auto dataType = UI_Inspector::ComponentDragType;

	auto payload = ImGui::GetDragDropPayload();
	if (payload != nullptr && payload->IsDataType(dataType)) {

		auto draggedComponent = *(Component**)payload->Data;
		if (draggedComponent != nullptr) {
			auto draggedRef = draggedComponent->csRef();
			bool isAssignable = _game->callbacks().isAssignable(draggedRef, scriptIdHash);
			return isAssignable;
		}
	}
	return false;
}

bool UI_Inspector::AcceptDroppedActor(CsRef* currentRef) {
	auto dataType = UI_Hierarchy::ActorDragType;

	auto payload = ImGui::GetDragDropPayload();
	if (payload != nullptr && payload->IsDataType(dataType)) {

		auto draggedActor = *(Actor**)payload->Data;
		if (draggedActor != nullptr && ImGui::BeginDragDropTarget()) {
			ImGui::AcceptDragDropPayload(dataType);

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {

				auto draggedRef = draggedActor->csRef();
				if (*currentRef != draggedRef) {
					*currentRef = draggedRef;
					return true;
				}
			}
		}
	}	
	return false;
}

bool UI_Inspector::AcceptDroppedComponent(int scriptIdHash, CsRef* currentRef) {
	auto dataType = UI_Inspector::ComponentDragType;

	auto payload = ImGui::GetDragDropPayload();
	if (payload != nullptr && payload->IsDataType(dataType)) {

		auto draggedComponent = *(Component**)payload->Data;
		if (draggedComponent != nullptr && ImGui::BeginDragDropTarget()) {
			auto draggedRef = draggedComponent->csRef();

			if (_game->callbacks().isAssignable(draggedRef, scriptIdHash)) {
				ImGui::AcceptDragDropPayload(dataType);

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
					if (*currentRef != draggedRef) {
						*currentRef = draggedRef;
						return true;
					}
				}
			}
		}
	}
	return false;
}

void UI_Inspector::m_DrawComponentContextMenu(Component* component)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 10.0f, 10.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 3.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 5.0f, 5.0f });

	ImGui::PushStyleColor(ImGuiCol_PopupBg, { 0.7f, 0.7f, 0.7f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f, 0.0f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.8f, 0.8f, 0.9f, 1.0f });

	if (ImGui::BeginPopupContextItem(0, ImGuiPopupFlags_MouseButtonRight))
	{
		
		if (ImGui::Selectable("Copy"))
			ComponentMenu::Copy(component);

		if (component->csRef().value != 0) {
			if (ImGui::Selectable("Copy Ref"))
				m_copyedCompRef = component->csRef();
		}
		if (ImGui::Selectable("Remove"))
			ComponentMenu::Remove(component);

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar(6);
	ImGui::PopStyleColor(3);
}

bool UI_Inspector::m_DrawActorFieldContextMenu(CsRef* compRef) {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 10.0f, 10.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 3.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 5.0f, 5.0f });

	ImGui::PushStyleColor(ImGuiCol_PopupBg, { 0.7f, 0.7f, 0.7f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f, 0.0f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.8f, 0.8f, 0.9f, 1.0f });

	bool changed = false;

	if (ImGui::BeginPopupContextItem(0, ImGuiPopupFlags_MouseButtonRight))
	{
		if (ImGui::Selectable("Set Null")) {
			*compRef = CsRef(0);
			changed = true;
		}
		ImGui::EndPopup();
	}

	ImGui::PopStyleVar(6);
	ImGui::PopStyleColor(3);

	return changed;
}

bool UI_Inspector::m_DrawComponentFieldContextMenu(int scriptIdHash, CsRef* compRef) {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 10.0f, 10.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 3.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 5.0f, 5.0f });

	ImGui::PushStyleColor(ImGuiCol_PopupBg, { 0.7f, 0.7f, 0.7f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f, 0.0f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.8f, 0.8f, 0.9f, 1.0f });

	bool changed = false;

	if (ImGui::BeginPopupContextItem(0, ImGuiPopupFlags_MouseButtonRight))
	{
		if (ImGui::Selectable("Set Null")) {
			*compRef = CsRef(0);
			changed = true;
		}
		if (m_copyedCompRef.value != 0) {

			bool isAssignable = _game->callbacks().isAssignable(m_copyedCompRef, scriptIdHash);
			if (isAssignable) {
				if (ImGui::Selectable("Past Ref")) {
					*compRef = m_copyedCompRef;
					changed = true;
				}
			}
		}
		ImGui::EndPopup();
	}

	ImGui::PopStyleVar(6);
	ImGui::PopStyleColor(3);

	return changed;
}

DEF_FUNC(UI_Inspector, ShowText, bool)(CppRef gameRef, const char* label, const char* buffer, int length, size_t* ptr)
{
	length = (length < 1024) ? length : 1024;
	std::memcpy(UI_Inspector::textBuffer, buffer, length);

	*ptr = (size_t)UI_Inspector::textBuffer;

	//auto game = CppRefs::ThrowPointer<Game>(gameRef);

	std::string tmpLabel = "##" + (std::string)label;

	ImGui::Columns(2, "", false);
	ImGui::SetColumnWidth(0, 100.0f);

	ImGui::Text(label);

	ImGui::NextColumn();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 7.0f);

	ImGui::InputText(tmpLabel.c_str(), UI_Inspector::textBuffer, sizeof(UI_Inspector::textBuffer));

	bool changed = ImGui::IsItemDeactivatedAfterChange();

	ImGui::PopItemWidth();
	ImGui::Columns(1);

	return changed;
}

DEF_FUNC(UI_Inspector, SetComponentName, void)(CppRef gameRef, const char* value) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	game->ui()->inspector()->SetComponentName(value);
}

DEF_FUNC(UI_Inspector, ShowAsset, bool)(CppRef gameRef, const char* label, int scriptIdHash, int* assetIdHash, bool isActive) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	return game->ui()->inspector()->ShowAsset(label, scriptIdHash, assetIdHash, isActive);
}

DEF_FUNC(UI_Inspector, ShowActor, bool)(CppRef gameRef, const char* label, CsRef* csRef, CppRef cppRef) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	return game->ui()->inspector()->ShowActor(label, csRef, cppRef);
}

DEF_FUNC(UI_Inspector, ShowComponent, bool)(CppRef gameRef, const char* label, CsRef* csRef, CppRef cppRef, int scriptIdHash) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	return game->ui()->inspector()->ShowComponent(label, csRef, cppRef, scriptIdHash);
}

DEF_FUNC(ImGui, CalcTextWidth, float)(const char* value) {
	return ImGui::CalcTextSize(value).x;
}

DEF_FUNC(UI_Inspector, ShowColor3, bool)(CppRef gameRef, const char* label, Vector3* value) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	return game->ui()->inspector()->ShowColor3(label, value);
}

DEF_FUNC(UI_Inspector, ShowColor4, bool)(CppRef gameRef, const char* label, Vector4* value) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	return game->ui()->inspector()->ShowColor4(label, value);
}