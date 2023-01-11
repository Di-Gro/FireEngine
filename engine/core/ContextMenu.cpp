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
