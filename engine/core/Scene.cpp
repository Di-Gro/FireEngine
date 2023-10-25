#include "Scene.h"

#include "Game.h"
#include "Assets.h"
#include "Physics.h"
#include "Actor.h"
#include "CameraComponent.h"
#include "EditorCamera.h"
#include "LinedPlain.h"
#include "Audio.h"
#include "Sound.h"
#include "PhysicsScene.h"

unsigned int Scene::m_objectCount = 0;

mono::mono_method_invoker<CppRef()> Scene::mono_create;

bool Scene::mono_inited = false;

static const char* MissingPushPopMsg = "You are trying to manipulate the scene without calling PushScene() / PopScene() pair.";


void Scene::Init(Game* game, bool isEditor) {
	pointerForDestroy(this);

	m_game = game;
	m_isEditor = isEditor;

	m_InitMono();

	renderer.Init(game, this);

	m_physicsScene = new PhysicsScene();
	m_physicsScene->Init();

	m_audio = new Audio();
	m_audio->Init(m_game, this);
}

void Scene::Start() {
	m_isStarted = true;

	renderer.Start();

	linedPlain = CreateActor("Lined Plain")->AddComponent<LinedPlain>();
	linedPlain->localPosition({ 0, -5, 0 });
	linedPlain->actor()->isSelectable = false;

	editorCamera = CreateActor("Editor Camera")->AddComponent<EditorCamera>();
	editorCamera->localPosition(m_initCameraPos);
	editorCamera->localRotationQ(m_initCameraRot);
	editorCamera->speed = m_initCameraSpeed;

	editorCamera->Attach();

	m_MoveToStatic(linedPlain->actor());
	m_MoveToStatic(editorCamera->actor());
}

void Scene::m_InitMono() {
	if (mono_inited)
		return;

	auto type_Actor = m_game->mono()->GetType("Engine", "Actor");
	mono_create = mono::make_method_invoker<CppRef()>(type_Actor, "cpp_Create");
}

void Scene::Release() {
	m_game->DestroyScene(this);
}

void Scene::mainCamera(CameraComponent* camera) {
	m_mainCamera = camera;
	if (m_mainCamera == nullptr)
		m_mainCamera = editorCamera;
}

void Scene::f_Update() {
	m_game->PushScene(this);

	if (!m_isStarted)
		throw std::exception("Scene not started");

	m_UpdateActors(&m_staticActors);
	m_UpdateActors(&m_actors);

	m_audio->Update();

	m_game->PopScene();
}

void Scene::f_FixedUpdate() {
	m_game->PushScene(this);

	if (!m_isStarted)
		throw std::exception("Scene not started");

	if (!m_isEditor) {
		m_game->physics()->Update(this);

		m_FixedUpdate(&m_staticActors);
		m_FixedUpdate(&m_actors);
	}
	m_game->PopScene();
}

void Scene::Destroy() {
	assert(!IsDestroyed());

	m_game->PushScene(this);

	m_DestroyActors(&m_actors);
	m_DestroyActors(&m_staticActors);

	renderer.Destroy();
	m_physicsScene->Destroy();

	delete m_physicsScene;
	m_physicsScene = nullptr;

	m_audio->Destroy();
	delete m_audio;
	m_audio = nullptr;

	m_game->PopScene();
}

void Scene::m_UpdateActors(std::list<Actor*>* list) {
	auto it = list->begin();
	while (!IsDestroyed() && it != list->end()) {

		auto actor = (*it);
		if (actor->IsDestroyed()) {
			actor->timeToDestroy--;
			if (actor->timeToDestroy <= 0)
				it = m_EraseActor(it, list);
			continue;
		}
		actor->f_Update();
		it++;
	}
}

void Scene::m_FixedUpdate(std::list<Actor*>* list) {
	for (auto it = list->begin(); !IsDestroyed() && it != list->end(); it++) {
		auto actor = (*it);
		if (!actor->IsDestroyed())
			actor->f_FixedUpdate();
	}
}

void Scene::m_DestroyActors(std::list<Actor*>* list) {
	auto it = list->begin();
	while (!IsDestroyed() && it != list->end()) {
		DestroyActor(*it);
		it = m_EraseActor(it, list);
	}
}

Actor* Scene::CreateActor(Actor* parent, std::string name) {
	if (m_game->currentScene() != this)
		throw std::exception(MissingPushPopMsg);

	auto cppRef = mono_create();

	Actor* actor = CppRefs::ThrowPointer<Actor>(cppRef);
	if (name != "")
		actor->name(name);

	if (parent != nullptr) {
		actor->parent(parent);
		actor->localScale({ 1, 1, 1 });
	}
	return actor;
}


GameObjectInfo Scene::m_CreateActorFromCs(CsRef csRef, CppRef parentRef) {
	auto classInfoRef = CppRefs::Create(ClassInfo::Get<Actor>());

	Actor* actor = new Actor();
	m_actors.insert(m_actors.end(), actor);

	actor->f_actorID = ++m_objectCount;
	actor->f_actorIDStr = std::to_string(actor->f_actorID);
	actor->f_ref = CppRefs::Create(actor);
	actor->f_cppRef = actor->f_ref.cppRef();
	actor->f_csRef = csRef;

	actor->f_Init(m_game, this);

	if (parentRef.value != 0) {
		auto parent = CppRefs::ThrowPointer<Actor>(parentRef);
		actor->parent(parent);
		actor->localScale({ 1, 1, 1 });
	}
	GameObjectInfo objectInfo;
	objectInfo.classRef = RefCpp(classInfoRef.cppRef());
	objectInfo.objectRef = actor->cppRef();

	return objectInfo;
}

void Scene::DestroyActor(Actor* actor) {
	if (actor->ActorBase::BeginDestroy()) {

		actor->f_Destroy();

		if (CppRefs::IsValid(actor->f_ref)) 
			CppRefs::Remove(actor->f_ref);

		if (actor->csRef().value > 0)
			m_game->callbacks().removeCsRef(actor->csRef());

		actor->ActorBase::EndDestroy();
	}
}

std::list<Actor*>::iterator Scene::m_EraseActor(std::list<Actor*>::iterator it, std::list<Actor*>* list) {
	auto* actor = *it;
	auto next = list->erase(it);
	delete actor;
	return next;
}


void Scene::m_MoveToStatic(Actor* actor) {
	for (auto it = m_actors.rbegin(); it != m_actors.rend(); it++) {
		if (*it == actor) {
			m_actors.erase(std::next(it).base());
			m_staticActors.insert(m_staticActors.end(), actor);
			return;
		}
	}
}

CameraIter Scene::AddCamera(CameraComponent* camera) {
	return m_cameras.insert(m_cameras.begin(), camera);
}

void Scene::RemoveCamera(CameraIter iter) {
	m_cameras.erase(iter);
}

std::list<Actor*>::iterator Scene::GetNextRootActors(const std::list<Actor*>::iterator& iter) {

	for (auto it = iter; it != m_actors.end(); ++it) {
		auto actor = *it;
		if (!actor->IsDestroyed() && !actor->HasParent())
			return it;
	}
	return m_actors.end();
}

int Scene::GetRootActorsCount() {
	if (m_game->currentScene() != this)
		throw std::exception(MissingPushPopMsg);

	int count = 0;
	for (auto actor : m_actors) {
		if (!actor->HasParent())
			count++;
	}
	return count;
}

void Scene::WriteRootActorsRefs(CsRef* refs) {
	if (m_game->currentScene() != this)
		throw std::exception(MissingPushPopMsg);

	CsRef* ptr = refs;
	for (auto actor : m_actors) {
		if (!actor->HasParent()) {
			*ptr = actor->csRef();
			ptr++;
		}
	}
}

void Scene::AttachPlayerCamera() {
	for (auto camera : m_cameras) {
		if (camera != editorCamera && camera->isPlayerCamera)
			return camera->Attach();
	}
}

void Scene::Stat() {
	int actorsCount = 0;
	int csComponents = 0;
	int cppComponents = 0;

	for (auto it = BeginActor(); it != EndActor(); it++) {
		auto actor = *it;
		actorsCount++;

		int count = actor->GetComponentsCount();
		size_t* refs = new size_t[count]();
		actor->WriteComponentsRefs(refs);

		for (int i = 0; i < count; i++) {
			if (refs[i] == 0)
				cppComponents++;
			else
				csComponents++;
		}

		delete refs;
	}

	std::cout << "+ Game.Stat() -> " << std::endl;
	std::cout << "+ actorsCount: " << actorsCount << std::endl;
	std::cout << "+ csComponents: " << csComponents << std::endl;
	std::cout << "+ cppComponents: " << cppComponents << std::endl;
	std::cout << "+ <- " << std::endl;
}

void Scene::MoveActor(Actor* from, Actor* to, bool isPastBefore)
{
	bool isFrom = false;
	bool isTo = false;

	std::list<Actor*>::iterator newIt = m_actors.end();

	// Throw Exception
	auto it = m_actors.begin();
	while (it != m_actors.end())
	{
		if (*it == from)
		{
			auto iter = m_actors.erase(it);
			it = iter;
			isFrom = true;
			continue;
		}

		if (*it == to)
		{
			newIt = it;
			isTo = true;
		}

		if (isFrom && isTo)
			break;

		it++;
	}

	if (newIt == m_actors.end())
		return;

	if(isPastBefore)
		m_actors.insert(newIt, from);
	else
		m_actors.insert(++newIt, from);
}

void Scene::editorCameraPos(const Vector3& position) {
	if (editorCamera == nullptr)
		m_initCameraPos = position;
	else
		editorCamera->localPosition(position);
}

void Scene::editorCameraRot(const Quaternion& rotation) {
	if (editorCamera == nullptr)
		m_initCameraRot = rotation;
	else
		editorCamera->localRotationQ(rotation);
}

Vector3 Scene::editorCameraPos() {
	if (editorCamera == nullptr)
		return m_initCameraPos;

	return editorCamera->localPosition();
}

Quaternion Scene::editorCameraRot() {
	if (editorCamera == nullptr)
		return m_initCameraRot;

	return editorCamera->localRotationQ();
}

void Scene::editorCameraSpeed(float value) {
	if (editorCamera == nullptr)
		m_initCameraSpeed = value;
	else
		editorCamera->speed = value;
}

float Scene::editorCameraSpeed() {
	if (editorCamera == nullptr)
		return m_initCameraSpeed;

	return editorCamera->speed;
}

std::string Scene::ToSceneAssetId(const std::string& assetId) {
	auto sceneRef = CppRefs::GetRef(this);
	return "scene_" + sceneRef.ToString() +"_" + assetId;
}

DEF_FUNC(Game, CreateGameObjectFromCS, GameObjectInfo)(CppRef sceneRef, CsRef csRef, CppRef parentRef) {
	return CppRefs::ThrowPointer<Scene>(sceneRef)->m_CreateActorFromCs(csRef, parentRef);
}

DEF_FUNC(Game, GetRootActorsCount, int)(CppRef sceneRef) {
	if (sceneRef.value == 0)
		throw std::exception(MissingPushPopMsg);

	return CppRefs::ThrowPointer<Scene>(sceneRef)->GetRootActorsCount();
}

DEF_FUNC(Game, WriteRootActorsRefs, void)(CppRef sceneRef, CsRef* refs) {
	if (sceneRef.value == 0)
		throw std::exception(MissingPushPopMsg);

	CppRefs::ThrowPointer<Scene>(sceneRef)->WriteRootActorsRefs(refs);
}

DEF_PUSH_ASSET(Scene);

DEF_PROP_GETSET_STR(Scene, name);

DEF_PROP_GETSET(Scene, Vector3, editorCameraPos);
DEF_PROP_GETSET(Scene, Quaternion, editorCameraRot);
DEF_PROP_GETSET(Scene, float, editorCameraSpeed);

FUNC(Scene, CreateSound, CppRef)(CppRef sceneRef, CsRef soundRef) {
	auto* scene = CppRefs::ThrowPointer<Scene>(sceneRef);

	auto* sound = new Sound(scene, soundRef);
	return sound->f_cppRef;
}

FUNC(Scene, RemoveSound, void)(CppRef soundRef) {
	auto* sound = CppRefs::ThrowPointer<Sound>(soundRef);

	delete sound;
}
