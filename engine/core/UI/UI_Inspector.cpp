#include "UI_Inspector.h"

#include <iostream>
#include <algorithm>
#include <list>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/misc/cpp/imgui_stdlib.h"
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
char UI_Inspector::textBuffer[UI_Inspector::TEXT_BUF_SIZE] = { 0 };

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

void UI_Inspector::DrawHeaderContext(const std::string& idText, ImVec2 headerSize, ImVec2 lastCursor, ImVec2 nextCursor) {
	static std::string id;
	id.clear();
	
	auto textSize = ImGui::CalcTextSize(idText.c_str());

	lastCursor.x = headerSize.x - textSize.x - 10;
	lastCursor.y += (headerSize.y - textSize.y) / 2;

	id += "##m_DrawActorId";
	id += idText;

	ImGui::PushID(id.c_str());

	ImGui::PushStyleColor(ImGuiCol_Text, { 0.4f, 0.4f ,0.4f ,1.0f });

	ImGui::SetCursorPos(lastCursor);
	ImGui::Text(idText.c_str());
	ImGui::SetCursorPos(nextCursor);

	ImGui::PopStyleColor(1);

	ImGui::PopID();
}

void UI_Inspector::m_DrawAddComponent() {
	static auto headerText = "(Right click to Add Component)";

	BigSpace();

	ImGui::PushStyleColor(ImGuiCol_Text, { 0.4f, 0.4f ,0.4f ,1.0f });
	
	auto windowWidth = ImGui::GetWindowSize().x;
	auto textSize = ImGui::CalcTextSize(headerText);

	ImGui::SetCursorPosX((windowWidth - textSize.x) * 0.5f);
	ImGui::Text(headerText);

	ImGui::PopStyleColor(1);

	auto mousePos = ImGui::GetScrollY() + ImGui::GetMousePos().y;

	auto actor = _game->ui()->GetActor();
	if (mousePos >= ImGui::GetCursorPosY() + 20 || m_componentPicker.isOpen()) {

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

	_ui->rectWidth(ImGui::GetContentRegionAvail().x);
	m_DrawComponent(&UI_Inspector::m_DrawActorHeader);

	
}

bool UI_Inspector::CollapsingHeader(Component* component, const std::string& name) {
	static std::string compId;
	
	bool result = false;

	bool isCrashed = component->f_isCrashed;
	auto nodeId = name + "##" + std::to_string(component->cppRef().value);

	if (isCrashed) {
		ImGui::PushStyleColor(ImGuiCol_Header, { 0.4f, 0.0f, 0.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.5f, 0.0f, 0.0f, 1.0f });
	}

	auto lastCursor = ImGui::GetCursorPos();
	result = ImGui::CollapsingHeader(nodeId.c_str(), collapsingHeaderFlags);
	auto nextCursor = ImGui::GetCursorPos();
	auto headerSize = ImGui::GetItemRectSize();

	m_DrawComponentContextMenu(component);
	if (component->IsDestroyed())
		return false;

	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload(UI_Inspector::ComponentDragType, &component, sizeof(Component*));
		ImGui::EndDragDropSource();
	}
	
	compId.clear();
	compId += isCrashed ? "Crashed " : "";
	compId += "ID:";
	compId += std::to_string(component->csRef());

	DrawHeaderContext(compId, headerSize, lastCursor, nextCursor);

	if (isCrashed)
		ImGui::PopStyleColor(2);

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
				_ui->rectWidth(ImGui::GetContentRegionAvail().x);
				_ui->groupId(component->csRef().value);

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
	_ui->_callbacks.onDrawComponent(csRef);
}

bool UI_Inspector::ShowVector3(Vector3* values, const std::string& title)
{
	static std::string nameX;
	static std::string nameY;
	static std::string nameZ;
	
	nameX.clear();
	nameX += "##X_";
	nameX += title;

	nameY.clear();
	nameY += "##Y_";
	nameY += title;

	nameZ.clear();
	nameZ += "##Z_";
	nameZ += title;

	auto tmpValues = *values;

	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnWidth(0, 80.0f);
	ImGui::Text(title.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 50.0f);

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
	static std::string fieldName;
	static std::string assetName;
	static std::string assetType;
	static std::string buttonText;
	static std::string buttonId;

	auto& prefabId = actor->prefabId();
	auto assetIdHash = prefabId == "" ? 0 : _game->assets()->GetAssetIDHash(prefabId);
	auto scriptIdHash = _game->assetStore()->prefabTypeIdHash;
		
	fieldName.clear();
	fieldName += "##Prefab";
	fieldName += std::to_string(_ui->groupId());

	assetName.clear();
	assetName += _game->assetStore()->GetAssetName(assetIdHash);

	assetType.clear();
	assetType += _game->assetStore()->GetScriptName(scriptIdHash);

	buttonText.clear();
	buttonText += assetName;
	buttonText += " (";
	buttonText += assetType;
	buttonText += ") ";

	buttonId.clear();
	buttonId += fieldName;
	buttonId += "##2";

	auto& style = ImGui::GetStyle();

	float column1 = 50;
	float column2 = _ui->rectWidth() - column1;
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

	ImGui::PushID(ImGui::GetID(buttonId.c_str()));
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
	static std::string fieldName;
	static std::string assetName;
	static std::string assetType;
	static std::string buttonText;
	static std::string buttonId;
	
	fieldName.clear();
	fieldName += "##";
	fieldName += label;
	fieldName += std::to_string(_ui->groupId());

	assetName.clear();
	assetName += _game->assetStore()->GetAssetName(*assetIdHash);

	assetType.clear();
	assetType += _game->assetStore()->GetScriptName(scriptIdHash);

	buttonText.clear();
	buttonText += assetName;
	buttonText += " (";
	buttonText += assetType;
	buttonText += ") ";

	buttonId.clear();
	buttonId += fieldName;
	buttonId += "##2";

	if (scriptIdHash == _game->assetStore()->prefabTypeIdHash)
		scriptIdHash = _game->assetStore()->actorTypeIdHash;

	m_assetPickerPopup.header = assetType;
	m_assetPickerPopup.scriptIdHash = scriptIdHash; 

	auto& style = ImGui::GetStyle();

	float column1 = 100;
	float column2 = _ui->rectWidth() - column1;
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

	ImGui::PushID(ImGui::GetID(buttonId.c_str()));
	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.0f, 0.5f });
	ImGui::Button(buttonText.c_str(), { holderWidth, iconWidth });
	ImGui::PopStyleVar(1);

	bool isDoubleClick = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left);
	bool isHovered = ImGui::IsItemHovered();

	if (*assetIdHash != 0 && isHovered && isDoubleClick)
		_ui->SelectedAsset(*assetIdHash);

	if (ImGui::IsMouseReleased(ImGuiPopupFlags_MouseButtonRight) && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
		int h = 0;

	m_assetPickerPopup.flags = ImGuiPopupFlags_MouseButtonRight;
	bool assetWasSelected = m_assetPickerPopup.Open(_game);
	ImGui::PopID();

	ImGui::Columns(1);

	if (assetWasSelected) {
		int newAssetIdHash = m_assetPickerPopup.selected;
		if (newAssetIdHash != *assetIdHash) {
			*assetIdHash = newAssetIdHash;
			return true;
		}
	}
	return false;
}

bool UI_Inspector::ShowActor(const std::string& label, CsRef* csRef, CppRef cppRef) {
	static std::string actorId;
	static std::string fieldName;
	static std::string actorName;
	static std::string buttonText;
	static std::string buttonId;
	
	auto actor = CppRefs::GetPointer<Actor>(cppRef);

	if (actor == nullptr && cppRef.value != 0)
		throw std::exception("ShowComponent: Bad actor reference");
	
	actorId.clear();

	fieldName.clear();
	fieldName += "##";
	fieldName += label;
	fieldName += std::to_string(_ui->groupId());
	
	actorName.clear();
	actorName += csRef->value == 1 ? "Missing" : "Null"; 

	if (actor != nullptr) {
		actorId += "[";
		actorId += actor->IdStr();
		actorId += "] ";

		actorName.clear();
		actorName += actor->name();
	}
	buttonText.clear();
	buttonText += actorId;
	buttonText += actorName;
	buttonText += " (Actor) ";

	buttonId.clear();
	buttonId += fieldName;
	buttonId += "##2";

	auto& style = ImGui::GetStyle();

	float column1 = 100;
	float column2 = _ui->rectWidth() - column1;
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

	ImGui::PushID(ImGui::GetID(buttonId.c_str()));
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
	static std::string fieldName;
	static std::string actorName;
	static std::string compId;
	static std::string assetType;
	static std::string buttonText;
	static std::string buttonId;
	
	auto component = CppRefs::GetPointer<Component>(cppRef);
	auto actor = component == nullptr ? nullptr : component->actor();

	if (component == nullptr && cppRef.value != 0)
		throw std::exception("ShowComponent: Bad component reference");

	fieldName.clear();
	fieldName += "##";
	fieldName += label;
	fieldName += std::to_string(_ui->groupId());

	actorName.clear();
	actorName += csRef->value == 1 ? "Missing" : "Null";
	if (actor != nullptr) {
		actorName.clear();
		actorName += actor->name();
	}

	compId.clear();
	if (component != nullptr) {
		compId += "[";
		compId += std::to_string(component->csRef());
		compId += "] ";
	}

	assetType.clear();
	assetType += _game->assetStore()->GetScriptName(scriptIdHash);

	buttonText.clear();
	buttonText += compId;
	buttonText += actorName;
	buttonText += " (";
	buttonText += assetType;
	buttonText += ") ";

	buttonId.clear();
	buttonId += fieldName;
	buttonId += "##2";

	auto& style = ImGui::GetStyle();

	float column1 = 100;
	float column2 = _ui->rectWidth() - column1;
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
	
	ImGui::PushID(ImGui::GetID(buttonId.c_str()));
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
	static std::string itemId;

	itemId.clear();
	itemId += "##_";
	itemId += label;
	itemId += "_";
	itemId += std::to_string(_ui->groupId());

	auto& style = ImGui::GetStyle();

	float column1 = 100;
	float column2 = _ui->rectWidth() - column1;
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
	static std::string itemId;

	itemId.clear();
	itemId += "##_";
	itemId += label;
	itemId += "_";
	itemId += std::to_string(_ui->groupId());

	auto& style = ImGui::GetStyle();

	float column1 = 100;
	float column2 = _ui->rectWidth() - column1;
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

bool UI_Inspector::ShowText(const char* label, const char* labelId, const char* buffer, int length, size_t* ptr, int flags) {
	static std::string text;

	/// out
	*ptr = (size_t)UI_Inspector::textBuffer;
	
	text.clear();
	text += buffer;

	auto& style = ImGui::GetStyle();
	auto column1 = labelWidth;
	auto column2 = _ui->rectWidth() - labelWidth - padding;

	ImGui::Columns(2, "", false);
	ImGui::SetColumnWidth(0, column1);

	ImGui::Text(label);
	ImGui::NextColumn();
	ImGui::PushItemWidth(column2);

	ImGui::InputText(labelId, &text, flags);

	bool changed = ImGui::IsItemDeactivatedAfterChange();

	ImGui::PopItemWidth();
	ImGui::Columns(1);

	int textSize = text.size();
	if (textSize > UI_Inspector::TEXT_BUF_SIZE - 1)
		textSize = UI_Inspector::TEXT_BUF_SIZE - 1;

	std::memcpy(UI_Inspector::textBuffer, text.c_str(), textSize);
	UI_Inspector::textBuffer[textSize] = '\0';

	return changed;
}

DEF_FUNC(UI_Inspector, ShowText, bool)(CppRef gameRef, const char* label, const char* labelId, const char* buffer, int length, size_t* ptr, int flags) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	return game->ui()->inspector()->ShowText(label, labelId, buffer, length, ptr, flags);
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

DEF_FUNC(ImGui, CalcTextSize, Vector3)(const char* value) {
	auto size = ImGui::CalcTextSize(value);
	return Vector3(size.x, size.y, 0);
}

DEF_FUNC(ImGui, GetItemRectSize, Vector3)() {
	auto size = ImGui::GetItemRectSize();
	return Vector3(size.x, size.y, 0);
}

DEF_FUNC(ImGui, GetContentRegionAvail, Vector3)() {
	auto vec2 = ImGui::GetContentRegionAvail();
	return Vector3(vec2.x, vec2.y, 0);
}

DEF_FUNC(ImGui, Begin, bool)(const char* name, bool* isOpen, int flags) {
	return ImGui::Begin(name, isOpen, flags);
}

DEF_FUNC(UI_Inspector, ShowColor3, bool)(CppRef gameRef, const char* label, Vector3* value) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	return game->ui()->inspector()->ShowColor3(label, value);
}

DEF_FUNC(UI_Inspector, ShowColor4, bool)(CppRef gameRef, const char* label, Vector4* value) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	return game->ui()->inspector()->ShowColor4(label, value);
}