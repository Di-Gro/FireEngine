#include "ContextMenu.h"

#include "Game.h"
#include "Scene.h"
#include "Assets.h"
#include "AssetStore.h"
#include "UI\UserInterface.h"

#include "DirectionLight.h"
#include "AmbientLight.h"


void SceneMenu::AddActor(Scene* scene) {
	auto actor = scene->CreateActor();

	auto game = scene->game();
	game->assets()->MakeDirty(game->currentScene()->assetIdHash());
}

void SceneMenu::AddLight(Scene* scene) {
	auto actor = scene->CreateActor();
	actor->name("Light");

	actor->localPosition({ 0, 0, 300 });
	actor->localRotation({ rad(-45), rad(45 + 180), 0 });

	if (scene->directionLight == nullptr)
		actor->AddComponent<DirectionLight>();

	if (scene->ambientLight == nullptr)
		actor->AddComponent<AmbientLight>();

	auto game = scene->game();
	game->assets()->MakeDirty(scene->assetIdHash());
}

bool SceneMenu::CanCreate(Game* game, const std::string& name, bool inEditorDir) {
	if (name == "")
		return false;

	auto path = game->assetStore()->assetsPath() + "\\" + name + ".scene";
	if (inEditorDir)
		path = game->assetStore()->editorPath() + "\\" + name + ".scene";

	if (fs::exists(path))
		return false;

	return true;
}

int SceneMenu::Create(Game* game, const std::string& name, bool inEditorDir) {
	if (!CanCreate(game, name))
		return 0;

	auto path = game->assetStore()->assetsPath() + "\\" + name + ".scene";
	if(inEditorDir)
		path = game->assetStore()->editorPath() + "\\" + name + ".scene";

	return game->callbacks().createSceneAsset((size_t)path.c_str());
}

bool SceneMenu::CanRename(Scene* scene, const std::string& name) {
	if (name == "")
		return false;

	auto game = scene->game();

	bool hasAsset = game->callbacks().hasAssetInStore(scene->assetIdHash());
	if (!hasAsset)
		return false;

	auto path = game->assetStore()->assetsPath() + "/" + name + ".scene";

	if (fs::exists(path))
		return false;

	return true;
}

void SceneMenu::Rename(Scene* scene, const std::string& name) {
	if (!CanRename(scene, name))
		return;
	
	auto game = scene->game();
	fs::path path = game->assetStore()->assetsPath() + "/" + name + ".scene";

	bool res = game->callbacks().renameSceneAsset(scene->assetIdHash(), (size_t)path.string().c_str());

	if (res)
		scene->name(path.stem().string());
}

bool SceneMenu::CanSave(Scene* scene) {
	auto game = scene->game();
	return scene->IsAsset() && game->assets()->IsDirty(scene->assetIdHash());
}

void SceneMenu::Save(Scene* scene) {
	if (!CanSave(scene))
		return;

	auto game = scene->game();
	game->assets()->Save(scene->assetIdHash());
}

bool SceneMenu::CanSetAsStartup(Scene* scene) {
	if (scene == nullptr)
		return false;

	auto game = scene->game();

	int startupSceneIdHash = game->editorSettings.startupSceneIdHash;
	if (startupSceneIdHash == scene->assetIdHash())
		return false;

	return true;
}

void SceneMenu::SetAsStartup(Scene* scene) {
	if (!CanSetAsStartup(scene))
		return;

	auto game = scene->game();
	auto sceneRef = CppRefs::GetRef(scene);

	game->callbacks().setStartupScene(sceneRef);
}

void ActorMenu::AddChild(Actor* actor) {
	auto child = actor->scene()->CreateActor(actor);

	auto game = actor->game();
	game->assets()->MakeDirty(game->currentScene()->assetIdHash());
}

void ActorMenu::Remove(Actor* actor) {
	if (actor == nullptr)
		return;

	auto game = actor->game();

	if (game->ui()->GetActor() == actor)
		game->ui()->SelectedActor(nullptr);

	game->assets()->MakeDirty(actor->scene()->assetIdHash());
	actor->Destroy();
}

void ActorMenu::Copy(Actor* actor) {
	if (actor == nullptr)
		return;

	auto game = actor->game();

	if (!game->ui()->HasActor())
		return;

	game->callbacks().pushClipboard(actor->csRef());
}

Actor* ActorMenu::Paste(Game* game) {
	auto actorTypeId = game->assetStore()->actorTypeIdHash;
	bool isActor = game->callbacks().clipboardIsSameType(actorTypeId);

	if (!isActor)
		return nullptr;

	auto cppRef = game->callbacks().peekClipboard();
	auto actor = CppRefs::GetPointer<Actor>(cppRef);

	if (actor != nullptr) {
		game->ui()->SelectedActor(actor);
		//actor->name(actor->name() + " ("+ std::to_string(actor->Id()) + ")");
	}

	return actor;
}

void ComponentMenu::Remove(Component* component) {
	auto game = component->game();
	game->assets()->MakeDirty(component->scene()->assetIdHash());

	component->Destroy();
}

void ComponentMenu::Copy(Component* component) {
	if (component == nullptr)
		return;

	auto game = component->game();
	game->callbacks().pushClipboard(component->csRef());
}

bool ComponentMenu::CanPaste(Game* game) {
	auto commponentType = game->assetStore()->componentTypeIdHash;
	bool isComponent = game->callbacks().clipboardIsAssignable(commponentType);

	return isComponent;
}

Component* ComponentMenu::Paste(Actor* actor) {
	if (actor == nullptr)
		return nullptr;

	auto game = actor->game();

	if (!CanPaste(game))
		return nullptr;

	game->callbacks().clipboardSetActor(actor->csRef());

	auto cppRef = game->callbacks().peekClipboard();
	auto commponent = CppRefs::GetPointer<Component>(cppRef);

	return commponent;
}

bool PrefabMenu::CanCreate(Actor* actor, const std::string& name) {
	if (actor == nullptr || name == "")
		return false;

	auto game = actor->game();
	auto path = game->assetStore()->assetsPath() + "/" + name + ".prefab";

	if (fs::exists(path))
		return false;

	return true;
}

void PrefabMenu::Create(Actor* actor, const std::string& name) {
	if (!CanCreate(actor, name))
		return;

	auto game = actor->game();
	auto path = game->assetStore()->assetsPath() + "/" + name + ".prefab";

	auto prefabIdHash = game->callbacks().createPrefab(actor->csRef(), (size_t)path.c_str());

	game->callbacks().setPrefabId(actor->csRef(), prefabIdHash);
}

bool PrefabMenu::CanSave(Actor* actor) {
	if (actor == nullptr || actor->prefabId() == "")
		return false;

	auto game = actor->game();
	auto prefabIdHash = game->assets()->GetCsAssetIDHash(actor->prefabId());

	bool hasAsset = game->callbacks().hasAssetInStore(prefabIdHash);
	if (!hasAsset)
		return false;

	return true;
}

void PrefabMenu::Save(Actor* actor) {
	if (!CanSave(actor))
		return;

	auto game = actor->game();
	auto prefabIdHash = game->assets()->GetCsAssetIDHash(actor->prefabId());

	game->callbacks().updatePrefab(actor->csRef(), prefabIdHash);
}

bool PrefabMenu::CanLoad(Actor* actor) {
	if (actor == nullptr || actor->prefabId() == "")
		return false;

	auto game = actor->game();
	auto prefabIdHash = game->assets()->GetCsAssetIDHash(actor->prefabId());

	bool hasAsset = game->callbacks().hasAssetInStore(prefabIdHash);
	if (!hasAsset)
		return false;

	return true;
}

void PrefabMenu::Load(Actor* actor) {
	if (!CanLoad(actor))
		return;

	auto game = actor->game();
	auto prefabIdHash = game->assets()->GetCsAssetIDHash(actor->prefabId());

	auto position = actor->localPosition();
	auto rotation = actor->localRotationQ();
	auto scale = actor->localScale();
	auto prefabId = actor->prefabId();

	actor->Clear();

	game->PushScene(actor->scene());
	game->callbacks().loadPrefab(prefabIdHash, actor->csRef());
	game->PopScene();

	actor->prefabId(prefabId);
	actor->localPosition(position);
	actor->localRotationQ(rotation);
	actor->localScale(scale);
}