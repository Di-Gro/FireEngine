#include "UI_Hierarchy.h"

#include "../Game.h"
#include "../Scene.h"
#include "../Actor.h"
#include "../Assets.h"
#include "../Lighting.h"
#include "../ContextMenu.h"
#include "../HotKeys.h"
#include "../AssetStore.h"

#include "UserInterface.h"

const char* UI_Hierarchy::ActorDragType = "Hierarchy.Actor";


void UI_Hierarchy::Init(Game* game) {
	m_game = game;
	m_ui = m_game->ui();

	m_InitIcons();
}

void UI_Hierarchy::Draw_UI_Hierarchy() {
	m_isMouseReleaseOnDragActor = false;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (ImGui::Begin("Hierarchy")) {
		auto scene = m_game->ui()->selectedScene();
		if (scene != nullptr) {
			m_game->PushScene(scene);
			m_DrawSceneContextMenu();

			m_DrawSceneHeader();

			auto it = scene->GetNextRootActors(scene->BeginActor());
			for (; it != scene->EndActor(); it = scene->GetNextRootActors(++it))
				VisitActor(*it, -1, it);

			if (m_isMouseReleaseOnDragActor && m_dragTargetActor != nullptr)
				m_dragTargetActor = nullptr;

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				m_clickedActor = nullptr;

			ImGui::Dummy({ 0, 200.0f });

			m_HandleInput();

			m_game->PopScene();
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void UI_Hierarchy::m_DrawSceneHeader() {
	ImGuiTreeNodeFlags node_flags = 0
		//| ImGuiTreeNodeFlags_SpanFullWidth
		| ImGuiTreeNodeFlags_FramePadding
		| ImGuiTreeNodeFlags_Framed
		| ImGuiTreeNodeFlags_Leaf
		| ImGuiTreeNodeFlags_NoTreePushOnOpen;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.5f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

	auto scene = m_game->ui()->selectedScene();

	auto treeNodeId = "\t" + scene->name() + "##m_DrawSceneHeader";
	
	auto lastCursor = ImGui::GetCursorPos();
	bool selectedTree = ImGui::TreeNodeEx(treeNodeId.c_str(), node_flags);
	auto nextCursor = ImGui::GetCursorPos();
	auto imGuiItemSize = ImGui::GetItemRectSize();

	bool hasSelected = m_scenePickerPopup.Open(scene, treeNodeId.c_str());
	
	m_ui->inspector()->DrawHeaderContext("Scene", imGuiItemSize, lastCursor, nextCursor);

	if (hasSelected && m_game->CanChangeScene()) {
		if (m_scenePickerPopup.selected != scene->assetIdHash()) {
			auto guid = m_game->assetStore()->GetAssetGuid(m_scenePickerPopup.selected);
			auto nextScene = m_game->CreateScene(true, guid);
			m_game->ChangeScene(nextScene);
		}
	}

	ImGui::PopStyleVar(2);
}

void UI_Hierarchy::m_HandleInput() {
	if (ImGui::IsWindowHovered() || m_game->ui()->isSceneHovered) {
		auto actor = m_game->ui()->GetActor();

		if (m_game->hotkeys()->GetButtonDownEd(Keys::C, Keys::Ctrl))
			ActorMenu::Copy(actor);

		if (m_game->hotkeys()->GetButtonDownEd(Keys::V, Keys::Ctrl))
			ActorMenu::Paste(m_game);

		if (m_game->hotkeys()->GetButtonDownEd(Keys::Delete))
			ActorMenu::Remove(actor);
	}
}

void UI_Hierarchy::PushPopupStyles() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 10.0f, 10.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 3.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 5.0f, 5.0f });

	ImGui::PushStyleColor(ImGuiCol_PopupBg, { 0.7f, 0.7f ,0.7f ,1.0f });
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f ,0.0f ,1.0f });
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.8f, 0.8f ,0.9f ,1.0f });
}

void UI_Hierarchy::PopPopupStyles() {
	ImGui::PopStyleVar(6);
	ImGui::PopStyleColor(3);
}

void UI_Hierarchy::m_DrawSceneContextMenu() {
	PushPopupStyles();

	auto scene = m_game->ui()->selectedScene();

	auto flags = ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems;
	if (ImGui::BeginPopupContextWindow(0, flags))
	{
		if (ImGui::Selectable("Add Actor"))
			SceneMenu::AddActor(scene);

		bool canAddLight =
			scene->directionLight == nullptr ||
			scene->ambientLight == nullptr;

		auto lightFlags = !canAddLight ? ImGuiSelectableFlags_Disabled : 0;

		if (ImGui::Selectable("Add Light", false, lightFlags))
			SceneMenu::AddLight(scene);

		ImGui::EndPopup();
	}

	PopPopupStyles();
}

void UI_Hierarchy::m_DrawActorContextMenu(Actor* actor) {
	PushPopupStyles();

	auto flags = ImGuiPopupFlags_MouseButtonRight;
	if (ImGui::BeginPopupContextItem(0, flags)) {
		if (ImGui::Selectable("Add Child"))
			ActorMenu::AddChild(actor);

		if (ImGui::Selectable("Remove"))
			ActorMenu::Remove(actor);

		ImGui::EndPopup();
	}
	PopPopupStyles();
}

void UI_Hierarchy::VisitActor(Actor* actor, int index, std::list<Actor*>::iterator rootIter)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.5f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

	ImGuiTreeNodeFlags node_flags = (actor == m_ui->GetActor() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow
		| ImGuiTreeNodeFlags_OpenOnDoubleClick
		| ImGuiTreeNodeFlags_SpanFullWidth
		| ImGuiTreeNodeFlags_FramePadding;

	bool isChild = actor->GetChildrenCount() == 0;
	bool isActive = actor->isActive();

	if (isChild)
		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	float mouseHeight = ImGui::GetMousePos().y;
	float cursorHeight = ImGui::GetCursorScreenPos().y;
	float mousePosY = mouseHeight - cursorHeight;

	//std::string actorId = std::to_string(actor->Id());
	auto actorId = "ID:" + std::to_string(actor->Id());
	auto treeNodeId = actor->name() + "##" + actorId + "SceneTreeNodeEx";
		
	if (!isActive)
		ImGui::PushStyleColor(ImGuiCol_Text, { 1.00f, 1.00f, 1.00f, 0.3f });

	auto lastCursor = ImGui::GetCursorPos();
	bool selectedTree = ImGui::TreeNodeEx(treeNodeId.c_str(), node_flags);
	auto nextCursor = ImGui::GetCursorPos();
	auto imGuiItemSize = ImGui::GetItemRectSize();

	if (!isActive)
		ImGui::PopStyleColor(1);

	float height = mousePosY / (ImGui::GetFrameHeight() - 1);

	m_DrawActorContextMenu(actor);

	HandleDrag(actor);
	HandleDrop(actor, selectedTree, height, imGuiItemSize, lastCursor);

	ImGui::PopStyleVar(2);

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
		m_clickedActor = actor;

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
		if (ImGui::IsItemHovered() && m_clickedActor == actor)
			m_ui->SelectedActor(actor);
	}

	m_ui->inspector()->DrawHeaderContext(actorId, imGuiItemSize, lastCursor, nextCursor);

	if (selectedTree)
	{
		for (int i = 0; i < actor->GetChildrenCount(); ++i)
			VisitActor(actor->GetChild(i), i, rootIter);

		if(!isChild)
			ImGui::TreePop();
	}
}



void UI_Hierarchy::HandleDrag(Actor* actor)
{
	if (ImGui::BeginDragDropSource())
	{
		if (m_dragTargetActor == nullptr)
		{
			m_dragTargetActor = actor;
			m_clickedActor = nullptr;
			ImGui::SetDragDropPayload(ActorDragType, &actor, sizeof(Actor*));
		}
		ImGui::EndDragDropSource();
	}
}

void UI_Hierarchy::HandleDrop(Actor* actor, bool selectedTree, float height, ImVec2 size, ImVec2 cursor)
{
	if (ImGui::BeginDragDropTarget())
	{
		auto payload = ImGui::GetDragDropPayload();
		if (payload == nullptr || !payload->IsDataType(ActorDragType))
			return;

		bool isUpSide = height <= 0.25;
		bool isBottomSide = height >= 0.75;
		bool isCenter = !isUpSide && !isBottomSide;

		if (isCenter)
		{
			m_DrawItemSeparator(&m_moveSeparatorRes, true, size, cursor);
			m_DrawItemSeparator(&m_moveSeparatorRes, false, size, cursor);
		}
		else
			m_DrawItemSeparator(&m_moveSeparatorRes, isUpSide, size, cursor);

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && m_dragTargetActor != nullptr)
		{
			if(HandeDragDrop(m_dragTargetActor, actor, selectedTree, height))
				m_game->assets()->MakeDirty(actor->scene()->assetIdHash());

			m_dragTargetActor = nullptr;
		}

		ImGui::EndDragDropTarget();
	}
	else if(actor == m_dragTargetActor && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		m_isMouseReleaseOnDragActor = true;
	}
}

bool UI_Hierarchy::HandeDragDrop(Actor* drag, Actor* drop, bool isDropOpen, float height) {
	if (drop == nullptr || drag == nullptr)
		return false;

	if (m_FindTargetInActorParent(drop, drag))
		return false;

	bool isUpSide = height <= 0.25;
	bool isBottomSide = height >= 0.75;
	bool isCenter = !isUpSide && !isBottomSide;

	bool hasChildren = drop->GetChildrenCount() > 0;
	bool needInsertInOpen = hasChildren && isDropOpen && isBottomSide;

	bool isMove = drag->parent() == drop->parent() && !needInsertInOpen;
	bool isInsert = !isMove || isCenter;

	bool changed = false;

	if (isInsert) {
		if (isCenter) {
			drag->parent(drop);
			changed = true;
		}
		else {
			if (needInsertInOpen) {
				drag->parent(drop);
				drop->MoveChild(drag, drop->GetChild(0), true);
			}
			if (drag->parent() != drop->parent())
				drag->parent(drop->parent());

			isMove = true;
			isInsert = false;
			changed = true;
		}
	}
	if (isMove && !isInsert) {
		if(!drop->HasParent())
			m_ui->selectedScene()->MoveActor(drag, drop, isUpSide);
		else
			drop->parent()->MoveChild(drag, drop, isUpSide);

		changed = true;
	}
	return changed;
}

bool UI_Hierarchy::m_FindTargetInActorParent(Actor* actor, Actor* target)
{
	auto parent = actor;

	while (parent != nullptr)
	{
		if (parent == target)
			return true;
		parent = parent->parent();
	}
	return false;
}

void UI_Hierarchy::m_InitIcons() {
	m_icMoveSeparator = ImageResource::CreateFromFile("../../engine/data/icons/ic_separator.png");
	m_moveSeparatorTex = TextureResource::CreateFromImage(m_game->render(), &m_icMoveSeparator);
	m_moveSeparatorRes = ShaderResource::Create(&m_moveSeparatorTex);
}

void UI_Hierarchy::m_DrawItemSeparator(ShaderResource* icon, bool isBeforeItem, ImVec2 size, ImVec2 cursor)
{
	auto tmpCursor = ImGui::GetCursorPos();
	//cursor.y -= 1.5f;
	ImGui::SetCursorPos(cursor);

	ImVec4 color{ 0.11f, 0.64f, 0.92f, 1.00f };

	if (isBeforeItem)
		ImGui::Image(icon->get(), size, { 1, 1 }, { 0, 0 }, color);
	else
		ImGui::Image(icon->get(), size, { 0, 0 }, { 1, 1 }, color);

	ImGui::SetCursorPos(tmpCursor);
}
