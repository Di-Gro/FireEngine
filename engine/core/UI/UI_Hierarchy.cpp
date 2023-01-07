#include "UI_Hierarchy.h"

#include "../Game.h"
#include "../Scene.h"
#include "UserInterface.h"
#include "../Game.h"
#include "../Actor.h"
#include "../Lighting.h"
#include "../DirectionLight.h"
#include "../AmbientLight.h"
#include "../LinedPlain.h"

void UI_Hierarchy::Draw_UI_Hierarchy() {
	m_isMouseReleaseOnDragActor = false;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (ImGui::Begin("Hierarchy")) {
		auto scene = m_game->ui()->selectedScene();
		if (scene != nullptr) {
			m_game->PushScene(scene);

			m_DrawSceneContextMenu();
			auto it = scene->GetNextRootActors(scene->BeginActor());
			for (; it != scene->EndActor(); it = scene->GetNextRootActors(++it))
				VisitActor(*it, -1, it);

			if (m_isMouseReleaseOnDragActor && m_dragTargetActor != nullptr)
				m_dragTargetActor = nullptr;

			ImGui::Dummy({ 0, 200.0f });

			m_game->PopScene();
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void UI_Hierarchy::m_PushPopupStyles() {
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

void UI_Hierarchy::m_PopPopupStyles() {
	ImGui::PopStyleVar(6);
	ImGui::PopStyleColor(3);
}

void UI_Hierarchy::m_DrawSceneContextMenu() {
	m_PushPopupStyles();

	auto flags = ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems;
	if (ImGui::BeginPopupContextWindow(0, flags))
	{
		if (ImGui::Selectable("Add Actor")) {
			auto actor = m_game->currentScene()->CreateActor();
		}

		bool canAddLight =
			m_game->currentScene()->directionLight == nullptr
			|| m_game->currentScene()->ambientLight == nullptr;

		auto lightFlags = !canAddLight ? ImGuiSelectableFlags_Disabled : 0;

		if (ImGui::Selectable("Add Light", false, lightFlags)) {
			auto actor = m_game->currentScene()->CreateActor();
			actor->name("Light");

			actor->localPosition({ 0, 0, 300 });
			actor->localRotation({ rad(-45), rad(45 + 180), 0 });

			if (m_game->currentScene()->directionLight == nullptr)
				actor->AddComponent<DirectionLight>();

			if (m_game->currentScene()->ambientLight == nullptr)
				actor->AddComponent<AmbientLight>();
		}

		ImGui::EndPopup();
	}

	m_PopPopupStyles();
}

void UI_Hierarchy::m_DrawActorContextMenu(Actor* actor) {
	m_PushPopupStyles();

	auto flags = ImGuiPopupFlags_MouseButtonRight;
	if (ImGui::BeginPopupContextItem(0, flags)) {
		if (ImGui::Selectable("Add Child")) {
			auto child = m_game->currentScene()->CreateActor(actor);
		}
		if (ImGui::Selectable("Remove")) {
			if (m_game->ui()->GetActor() == actor)
				m_game->ui()->SelectedActor(nullptr);
			actor->Destroy();
		}
		ImGui::EndPopup();
	}
	m_PopPopupStyles();
}

void UI_Hierarchy::Init(Game* game)
{
	m_game = game;
	m_ui = m_game->ui();

	m_InitIcons();
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

	if (isChild)
		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	float mouseHeight = ImGui::GetMousePos().y;
	float cursorHeight = ImGui::GetCursorScreenPos().y;
	float mousePosY = mouseHeight - cursorHeight;

	std::string actorId = std::to_string(actor->Id());
	auto treeNodeId = actor->name() + "##" + actorId + "SceneTreeNodeEx";

	auto currentCursor = ImGui::GetCursorPos();
	bool selectedTree = ImGui::TreeNodeEx(treeNodeId.c_str(), node_flags);
	auto imGuiItemSize = ImGui::GetItemRectSize();

	float height = mousePosY / (ImGui::GetFrameHeight() - 1);

	m_DrawActorContextMenu(actor);

	HandleDrag(actor);
	HandleDrop(actor, selectedTree, height, imGuiItemSize, currentCursor);

	ImGui::PopStyleVar(2);

	if (ImGui::IsItemClicked())
		m_ui->SelectedActor(actor);

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
			std::cout << "Begin Drag" << std::endl;
			m_dragTargetActor = actor;
			ImGui::SetDragDropPayload("hierarchy", actor, sizeof(actor));
		}
		ImGui::EndDragDropSource();
	}
}

void UI_Hierarchy::HandleDrop(Actor* actor, bool selectedTree, float height, ImVec2 size, ImVec2 cursor)
{
	if (ImGui::BeginDragDropTarget())
	{
		bool isUpSide = height <= 0.25;
		bool isBottomSide = height >= 0.75;
		bool isCenter = !isUpSide && !isBottomSide;

		if (isCenter)
		{
			auto getP = ImGui::GetDragDropPayload();
			ImGui::AcceptDragDropPayload("hierarchy");
		}
		else
			m_DrawItemSeparator(&m_moveSeparatorRes, isUpSide, size, cursor);

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && m_dragTargetActor != nullptr)
		{
			std::cout << "Drop" << std::endl;
			HandeDragDrop(m_dragTargetActor, actor, selectedTree, height);
			m_dragTargetActor = nullptr;
			std::cout << "End Drag" << std::endl;
		}

		ImGui::EndDragDropTarget();
	}
	else if(actor == m_dragTargetActor && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		m_isMouseReleaseOnDragActor = true;
	}
}

void UI_Hierarchy::HandeDragDrop(Actor* drag, Actor* drop, bool isDropOpen, float height)
{
	if (drop == nullptr || drag == nullptr)
		return;

	if (m_FindTargetInActorParent(drop, drag))
		return;

	bool isUpSide = height <= 0.25;
	bool isBottomSide = height >= 0.75;
	bool isCenter = !isUpSide && !isBottomSide;

	bool hasChildren = drop->GetChildrenCount() > 0;
	bool needInsertInOpen = hasChildren && isDropOpen && isBottomSide;

	bool isMove = drag->parent() == drop->parent() && !needInsertInOpen;
	bool isInsert = !isMove || isCenter;

	if (isInsert)
	{
		if (isCenter)
		{
			drag->parent(drop);
			std::cout << "CENTER" << std::endl;
		}
		else
		{
			if (needInsertInOpen)
			{
				drag->parent(drop);
				drop->MoveChild(drag, drop->GetChild(0), true);
				return;
			}

			if (drag->parent() != drop->parent())
				drag->parent(drop->parent());

			isMove = true;
			isInsert = false;
		}
	}

	if (isMove && !isInsert)
	{
		if(!drop->HasParent())
			m_ui->selectedScene()->MoveActor(drag, drop, isUpSide);
		else
			drop->parent()->MoveChild(drag, drop, isUpSide);
	}
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
	m_game->imageAsset()->InitImage(&m_icMoveSeparator, "../../data/engine/icons/ic_separator.png");
	m_moveSeparatorTex = Texture::CreateFromImage(m_game->render(), &m_icMoveSeparator);
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
