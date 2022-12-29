#include <windows.h>
#include <shobjidl.h>
#include <fstream>
#include "UI_BarMenu.h"
#include "../Game.h"

bool UI_BarMenu::ButtonCenteredOnLine(const char* label, float alignment)
{
	ImGuiStyle& style = ImGui::GetStyle();

	float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
	float avail = ImGui::GetContentRegionAvail().x;

	float off = (avail - size) * alignment;
	if (off > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

	return ImGui::Button(label);
}

void UI_BarMenu::Draw_UI_BarMenu()
{

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("New", "CTRL + N");
			
			if (ImGui::MenuItem("Open", "CTRL + O"))
			{
				std::string filepath = OpenFileDialog("Fire Engine files\0*.fireengine_map\0");
				if (!filepath.empty())
				{
					// Serializ
					MessageBoxW(NULL, L"File oppened!", L"INFO", MB_OK);
				}
			}

			if (ImGui::MenuItem("Save", "CTRL + S"))
			{

			}

			if (ImGui::MenuItem("Save as", NULL))
			{
				std::string filepath = SaveAsFileDialog("Fire Engine files\0*.fireengine_map\0");
				if (!filepath.empty())
				{
					// Serializ
					MessageBoxW(NULL, L"File oppened!", L"INFO", MB_OK);
				}
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
			{
				_game->Exit(0);
			};

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options"))
		{
			ImGui::EndMenu();
		}

		if (ButtonCenteredOnLine("Play", 0.4f))
		{

		};
		ImGui::SameLine();
		if (ButtonCenteredOnLine("Pause", 0))
		{
			
		};
		ImGui::SameLine();
		if (ButtonCenteredOnLine("Stop", 0))
		{

		};

		ImGui::EndMenuBar();
	}
}

void UI_BarMenu::Init(Game* game)
{
	_game = game;
}

std::string UI_BarMenu::OpenFileDialog(const char* filter)
{
	OPENFILENAME ofn;
	char szFile[260] = { 0 };

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = win.GetHWindow();
	ofn.lpstrFile = (LPWSTR)szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"Fire Engine files\0*.fireengine_map\0";//(LPWSTR)filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetOpenFileName(&ofn) == TRUE)
	{
		std::wstring ws(ofn.lpstrFile);
		return std::string(ws.begin(), ws.end());
	}
	return std::string();
}

std::string UI_BarMenu::SaveAsFileDialog(const char* filter)
{
	OPENFILENAME ofn;
	char szFile[260] = { 0 };

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = win.GetHWindow();
	ofn.lpstrFile = (LPWSTR)szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"Fire Engine files\0*.fireengine_map\0";//(LPWSTR)filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetSaveFileName(&ofn) == TRUE)
	{
		std::wstring ws(ofn.lpstrFile);
		return std::string(ws.begin(), ws.end());
	}
	return std::string();
}

//void UI_BarMenu::OpenFileDialog()
//{
//	/*OPENFILENAME ofn;
//	char file_name[100];
//
//	ZeroMemory(&ofn, sizeof(OPENFILENAME));
//	ofn.lStructSize = sizeof(OPENFILENAME);
//	ofn.hwndOwner = win.GetHWindow();
//	ofn.lpstrFile = (LPWSTR)file_name;
//	ofn.lpstrFile[0] = '\0';
//	ofn.nMaxFile = 100;
//	ofn.lpstrFilter = L"Fire Engine files\0*.*\0";
//	ofn.nFilterIndex = 1;
//
//	GetOpenFileName(&ofn);*/
//
//
//	//HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
//	//	COINIT_DISABLE_OLE1DDE);
//	//if (SUCCEEDED(hr))
//	//{
//	//	IFileOpenDialog* pFileOpen;
//
//	//	// Create the FileOpenDialog object.
//	//	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
//	//		IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
//
//	//	if (SUCCEEDED(hr))
//	//	{
//	//		// Show the Open dialog box.
//	//		hr = pFileOpen->Show(NULL);
//
//	//		// Get the file name from the dialog box.
//	//		if (SUCCEEDED(hr))
//	//		{
//	//			IShellItem* pItem;
//	//			hr = pFileOpen->GetResult(&pItem);
//	//			if (SUCCEEDED(hr))
//	//			{
//	//				PWSTR pszFilePath;
//	//				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
//
//	//				// Display the file name to the user.
//	//				if (SUCCEEDED(hr))
//	//				{
//	//					MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
//	//					CoTaskMemFree(pszFilePath);
//	//				}
//	//				pItem->Release();
//	//			}
//	//		}
//	//		pFileOpen->Release();
//	//	}
//	//	CoUninitialize();
//	//}
//}
//
//void UI_BarMenu::SaveAsFileDialog()
//{
//	OPENFILENAME ofn;
//	char file_name[100];
//
//	ZeroMemory(&ofn, sizeof(OPENFILENAME));
//
//	ofn.lStructSize = sizeof(OPENFILENAME);
//	ofn.hwndOwner = win.GetHWindow();
//	ofn.lpstrFile = file_name;
//	ofn.lpstrFile[0] = '\0';
//	ofn.nMaxFile = 100;
//	ofn.lpstrFilter = L"Fire Engine files\0*.fireengine_map\0";
//	ofn.nFilterIndex = 1;
//
//
//	GetSaveFileName(&ofn);
//	//WriteDatasIntoFile((char*)ofn.lpstrFile);
//
//	//HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
//	//	COINIT_DISABLE_OLE1DDE);
//	//if (SUCCEEDED(hr))
//	//{
//	//	IFileSaveDialog* pFileSave;
//
//	//	// Create the FileOpenDialog object. CLSID_FileOpenDialog | IID_IFileOpenDialog
//	//	hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
//	//		IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));
//
//	//	if (SUCCEEDED(hr))
//	//	{
//	//		// Show the Open dialog box.
//	//		hr = pFileSave->Show(NULL);
//
//	//		// Get the file name from the dialog box.
//	//		if (SUCCEEDED(hr))
//	//		{
//	//			IShellItem* pItem;
//	//			hr = pFileSave->GetResult(&pItem);
//	//			if (SUCCEEDED(hr))
//	//			{
//	//				PWSTR pszFilePath;
//	//				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
//
//	//				// Display the file name to the user.
//	//				if (SUCCEEDED(hr))
//	//				{
//	//					MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
//	//					CoTaskMemFree(pszFilePath);
//	//				}
//	//				pItem->Release();
//	//			}
//	//		}
//	//		pFileSave->Release();
//	//	}
//	//	CoUninitialize();
//	//}
//}
//
//void UI_BarMenu::WriteDatasIntoFile(std::string path)
//{
//	std::ofstream outfile(path, std::ofstream::out);
//
//	if (outfile.is_open())
//	{
//		constexpr int N = 255;
//		char tmpArr[N];
//		for (int i = 0; i < N; ++i)
//			tmpArr[i] = 1 + rand() % 10;
//
//		outfile.write(tmpArr, sizeof(tmpArr));
//
//		outfile.close();
//	}
//
//	/*FILE* file;
//	file = fopen(path, "w");
//
//	constexpr int N = 255;
//	int tmpArr[N];
//	for (int i = 0; i < N; ++i)
//		tmpArr[i] = 1 + rand() % 10;
//
//	fwrite(tmpArr, sizeof(tmpArr), N, file);
//	fclose(file);*/
//}