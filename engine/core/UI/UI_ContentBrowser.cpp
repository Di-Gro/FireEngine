#include "UI_ContentBrowser.h"
#include "../Game.h"

static const std::filesystem::path s_AssetsPath = "assets";

UI_ContentBrowser::UI_ContentBrowser()
	: m_CurrentDirectory(s_AssetsPath)
{
	//m_DirectoryIcon = Texture2D::Create("../UI/Icons/DirectoryIcon.png");
	//m_FileIcon = Texture2D::Create("../UI/Icons/FileIcon.png");
}

void UI_ContentBrowser::Draw_UI_ContentBrowser()
{
	if (ImGui::Begin("Content Browser"))
	{
		if (m_CurrentDirectory != std::filesystem::path(s_AssetsPath))
		{
			if (ImGui::ArrowButton("##back", ImGuiDir_Left))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("+"))
		{
			ImGui::OpenPopup("Filesystem_ContentBrowser");
		}

		if (ImGui::BeginPopup("Filesystem_ContentBrowser"))
		{
			//ImGui::InputText();
			/*std::error_code err;
			if (!std::filesystem::create_directories(dirName, err))
			{
				if (std::filesystem::exists(dirName))
				{
					std::cout << "FAILED to create directory. The directory already exists." << std::endl;
				}
			}*/

			ImGui::Button("Create File");
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		ImGui::Text(m_CurrentDirectory.string().c_str());
		ImGui::Separator();

		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_AssetsPath);
			std::string filenameString = relativePath.filename().string();

			//ID3D11Texture2D* icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
			//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0, 0, 0, 0 });
			////ImGui::ImageButton((ImTextureID)icon, { thumbnailSize, thumbnailSize }, { 0, 1 }, { 0, 1 });
			//ImGui::ImageButton("##ImageID", {thumbnailSize, thumbnailSize}, {0, 1}, {0, 1});
			//ImGui::PopStyleColor();

			//if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			//{
			//	if(directoryEntry.is_directory())
			//		m_CurrentDirectory /= path.filename();
			//}
			
			if (directoryEntry.is_directory())
			{
				if (ImGui::Button(filenameString.c_str()))
				{
					m_CurrentDirectory /= path.filename();
				}
			}
			else
			{
				if (ImGui::Button(filenameString.c_str()))
				{

				}
			}
		}
		ImGui::Columns(1);
	}ImGui::End();
}

void UI_ContentBrowser::Draw_InnerContentBrowser()
{
	ImGuiWindowClass inner_content_browser;
	inner_content_browser.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
	ImGui::SetNextWindowClass(&inner_content_browser);
	if (ImGui::Begin("##inner_content_browser", NULL, ImGuiWindowFlags_NoTitleBar))
	{

	}ImGui::End();
}