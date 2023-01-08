#include "Scene.h"

#include "Game.h"
#include "Actor.h"
#include "CameraComponent.h"
#include "EditorCamera.h"
#include "LinedPlain.h"

unsigned int Scene::m_objectCount = 0;

mono::mono_method_invoker<CppRef()> Scene::mono_create;

bool Scene::mono_inited = false;

static const char* MissingPushPopMsg = "You are trying to manipulate the scene without calling PushScene() / PopScene() pair.";


void Scene::Init(Game* game, bool _isEditor) {
	m_game = game;
	m_isEditor = _isEditor;

	m_InitMono();

	renderer.Init(game, this);
}

void Scene::Start() {
	m_isStarted = true;

	renderer.Start();

	linedPlain = CreateActor("Lined Plain")->AddComponent<LinedPlain>();
	linedPlain->localPosition({ 0, -5, 0 });

	editorCamera = CreateActor("Editor Camera")->AddComponent<EditorCamera>();
	editorCamera->localPosition({ 350, 403, -20 });
	editorCamera->localRotation({ -0.803, 1.781, 0 });

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

void Scene::mainCamera(CameraComponent* camera) {
	m_mainCamera = camera;
	if (m_mainCamera == nullptr)
		m_mainCamera = editorCamera;
}

void Scene::f_Update() {
	m_game->PushScene(this);

	if (!m_isStarted)
		throw std::exception("Scene not started"); // ������ ������� scene.Start()

	m_Update(&m_staticActors);
	m_Update(&m_actors);

	m_game->PopScene();
}

void Scene::f_Destroy() {
	m_game->PushScene(this);

	m_Destroy(&m_actors);
	m_Destroy(&m_staticActors);

	renderer.Destroy();
	m_game->PopScene();
}

void Scene::m_Update(std::list<Actor*>* list) {
	auto it = list->begin();
	while (it != list->end()) {

		auto actor = (*it);
		if (actor->IsDestroyed()) {
			actor->friend_timeToDestroy--;
			if (actor->friend_timeToDestroy <= 0)
				it = m_EraseActor(it, list);
			continue;
		}
		actor->f_Update();
		it++;
	}
}

void Scene::m_Destroy(std::list<Actor*>* list) {
	auto it = list->begin();
	while (it != list->end()) {
		DestroyActor(*it);
		it = m_EraseActor(it, list);
	}
}

void Scene::f_RemoveActor(Actor* actor) {
	//TODO: �������� ����� �� ������ ��� ����������
}

void Scene::f_AddActor(Actor* actor) {
	//TODO: �������� ��� ������������ �����
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
	if (actor->ActorBase::friend_CanDestroy()) {
		actor->ActorBase::friend_StartDestroy();

		actor->f_Destroy();

		if (CppRefs::IsValid(actor->f_ref)) {
			CppRefs::Remove(actor->f_ref);
		}
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

void Scene::AttachGameCamera() {
	for (auto camera : m_cameras) {
		if (camera != editorCamera)
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
