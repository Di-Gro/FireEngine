#pragma once
#include <unordered_map>

#include "UI_Hierarchy.h"
#include "UI_Inspector.h"
#include "UI_Editor.h"
#include "UI_BarMenu.h"
#include "UI_UpperMenu.h"
#include "UI_PlayGame.h"
#include "UI_ContentBrowser.h"
#include "UI_Console.h"
#include "../imgui/imgui.h"
#include "../Actor.h"
#include "../SimpleMath.h"

#include "../ImageResource.h"
#include "../TextureResource.h"
#include "../ShaderResource.h"

class Game;
class SceneWindow;

class UserInterface {
	friend class UI_Editor;

public:
	struct Callbacks {
		void (*onDrawComponent)(CsRef); 
		void (*requestComponentName)(CsRef);
		void (*onDrawActorTags)(CsRef);
		void (*drawAssetEditor)();
	};

private:
	static bool opt_fullscreen;
	static bool dockspaceOpen;
	static bool opt_padding;

public:
	Callbacks _callbacks;

	ShaderResource icMove;
	ShaderResource icRotate;
	ShaderResource icScale;
	ShaderResource icPickup;
	ShaderResource icPickupActor;
	ShaderResource icPickupComponent;
	ShaderResource icPickupAsset;
	ShaderResource icSoundOn;
	ShaderResource icSoundOff;

	bool isSceneHovered = false;

private: /// For friends
	//Vector2 f_mouseViewportPosition;
	//Vector2 f_viewportPosition;

private:
	Game* _game;
	Actor* _actor;
	Scene* m_selectedScene = nullptr;
	int m_selectedAsset = 0;

	bool _isActorActive = false;

	/// Context -> 
	CppRef m_groupSceneRef = RefCpp(0);
	float m_rectWidth = 0;
	size_t m_groupId;
	int m_subGroupId;
	int m_groupAssetIdHash;
	/// <-

	UI_Hierarchy uiHierarchy;
	UI_Editor uiEditor;
	UI_BarMenu uiBarMenu;
	UI_UpperMenu uiUpperMenu;
	UI_PlayGame uiPlayGame;
	UI_ContentBrowser uiContentBrowser;
	UI_Console uiConsole;
	UI_Inspector uiInspector;
	//UI_AssetEditor m_assetEditor;

	static ImGuiDockNodeFlags dockspace_flags;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	std::unordered_map<size_t, SceneWindow*> m_sceneWindows;

	ImageResource m_imgMove;
	ImageResource m_imgRotate;
	ImageResource m_imgScale;
	ImageResource m_imgPickup;
	ImageResource m_imgPickupActor;
	ImageResource m_imgPickupComponent;
	ImageResource m_imgPickupAsset;
	ImageResource m_imgSoundOn;
	ImageResource m_imgSoundOff;

	TextureResource m_texMove;
	TextureResource m_texRotate;
	TextureResource m_texScale;
	TextureResource m_texPickup;
	TextureResource m_texPickupActor;
	TextureResource m_texPickupComponent;
	TextureResource m_texPickupAsset;
	TextureResource m_texSoundOn;
	TextureResource m_texSoundOff;

public:
	UserInterface();
	~UserInterface();

	void Draw();
	void Init(Game* game);
	void Destroy();

	UI_Inspector* inspector() { return &uiInspector; }
	//UI_AssetEditor* assetEditor() { return &m_assetEditor; };

	Scene* selectedScene() { return m_selectedScene; }
	void selectedScene(Scene* scene) { m_selectedScene = scene; }

	void SelectedActor(Actor* actor);
	void SetActorActive();

	int SelectedAsset() { return m_selectedAsset; }
	void SelectedAsset(int assetIdHash) { m_selectedAsset = assetIdHash; }

	inline Actor* GetActor() const { return _actor; }

	bool HasActor();
	bool isActive();

	float rectWidth() { return m_rectWidth; };
	void rectWidth(float value) { m_rectWidth = value; };

	CppRef groupSceneRef() { return m_groupSceneRef; }
	void groupSceneRef(CppRef ref) { m_groupSceneRef = ref; }

	size_t groupId() { return m_groupId; };
	void groupId(size_t value) { m_groupId = value; };

	int subGroupId() { return m_subGroupId; };
	void subGroupId(int value) { m_subGroupId = value; };

	int groupAssetIdHash() { return m_groupAssetIdHash; };
	void groupAssetIdHash(int value) { m_groupAssetIdHash = value; };

	void SetCallbacks(const Callbacks&);

	bool HasSceneWindow(const std::string& sceneId);
	SceneWindow* GetSceneWindow(const std::string& sceneId);
	SceneWindow* CreateSceneWindow(const std::string& sceneId);
	void RemoveSceneWindow(const std::string& sceneId);
		

private:
	//void InitMono();
	void InitDockSpace();
	void m_InitStyles();
	void m_InitIcons();

	size_t m_StringHash(const std::string& str) { return std::hash<std::string>()(str); }
};

FUNC(UserInterface, SetCallbacks2, void)(CppRef cppRef, const UserInterface::Callbacks& callbacks);

PROP_GETSET(UserInterface, int, SelectedAsset);
PROP_GETSET(UserInterface, CppRef, groupSceneRef);
PROP_GETSET(UserInterface, float, rectWidth);
PROP_GETSET(UserInterface, size_t, groupId);
PROP_GETSET(UserInterface, int, subGroupId);
PROP_GETSET(UserInterface, int, groupAssetIdHash);

FUNC(UserInterface, SelectedScene, CppRef)(CppRef uiRef);