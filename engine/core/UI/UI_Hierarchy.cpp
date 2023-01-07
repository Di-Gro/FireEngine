#include "UI_Hierarchy.h"

#include "../Game.h"
#include "../Scene.h"
#include "UserInterface.h"

void UI_Hierarchy::Draw_UI_Hierarchy()
{
	auto scene = m_game->ui()->selectedScene();
	m_ui->selectedScene(scene);
	if (scene == nullptr)
		return;
	
	m_isMouseReleaseOnDragActor = false;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (ImGui::Begin("Hierarchy"))
	{
		auto it = scene->GetNextRootActors(scene->BeginActor());
		for (; it != scene->EndActor(); it = scene->GetNextRootActors(++it))
		{	
			VisitActor(*it, -1, it);
		}

		if (m_isMouseReleaseOnDragActor && m_dragTargetActor != nullptr)
		{
			m_dragTargetActor = nullptr;
			std::cout << "End Drag" << std::endl;
		}

		ImGui::Dummy({ 0, 200.0f });
	}
	ImGui::End();
	ImGui::PopStyleVar();
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

	auto actorID = std::to_string(actor->Id());
	auto name = actor->GetName() == "" ? "Empty" : actor->GetName();
	auto treeNodeId = name + "##" + actorID + "SceneTreeNodeEx";

	auto currentCursor = ImGui::GetCursorPos();

	bool selectedTree = ImGui::TreeNodeEx(treeNodeId.c_str(), node_flags);

	auto imGuiItemSize = ImGui::GetItemRectSize();

	float height = mousePosY / (ImGui::GetFrameHeight() - 1);

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