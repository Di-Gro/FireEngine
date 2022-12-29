#include "PlatformUtils.h"
#include "Game.h"

std::string FileDialogs::OpenFile(const char* filter)
{
	OPENFILENAME ofn;
	char szFile[260] = { 0 };

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner = ;
	ofn.lpstrFile = (LPWSTR)szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = (LPWSTR)filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetOpenFileName(&ofn) == TRUE)
	{
		std::wstring ws(ofn.lpstrFile);
		return std::string(ws.begin(), ws.end());
	}
	return std::string();
}

std::string FileDialogs::SaveFile(const char* filter)
{
	OPENFILENAME ofn;
	char szFile[260] = { 0 };

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner = ;
	ofn.lpstrFile = (LPWSTR)szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = (LPWSTR)filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetSaveFileName(&ofn) == TRUE)
	{
		std::wstring ws(ofn.lpstrFile);
		return std::string(ws.begin(), ws.end());
	}
	return std::string();
}

void FileDialogs::Init(Game* game)
{
	_game = game;
}