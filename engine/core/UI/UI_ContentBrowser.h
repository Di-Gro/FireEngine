#pragma once

#include <filesystem>

class UI_ContentBrowser
{
public:
	UI_ContentBrowser();
	void Draw_UI_ContentBrowser();
	void Draw_InnerContentBrowser();
private:
	std::filesystem::path m_CurrentDirectory;
	//ID3D11Texture2D* m_DirectoryIcon;
	//ID3D11Texture2D* m_FileIcon;
};