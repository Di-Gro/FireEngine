#pragma once

#include <Jolt/Jolt.h>

#include <Jolt\Core\TempAllocator.h>
#include <Jolt\Core\JobSystemThreadPool.h>

JPH_SUPPRESS_WARNINGS

class Game;
class Scene;

class Physics {
private:
	Game* m_game;

	JPH::TempAllocatorImpl* m_allocator;
	JPH::JobSystemThreadPool* m_jobSystem;

public:
	void Init(Game* game);
	void Destroy();
	void Update(Scene* scene);

};


