#include "Physics.h"

#include <iostream>
#include <cstdarg>
#include <thread>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "Game.h"
#include "Scene.h"
#include "PhysicsScene.h"

using namespace JPH;

static void TraceImpl(const char* inFMT, ...);
static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine);


void Physics::Init(Game* game) {
	m_game = game;

	// Register allocation hook
	RegisterDefaultAllocator();

	// Install callbacks
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

	// Create a factory
	Factory::sInstance = new Factory();

	// Register all Jolt physics types
	RegisterTypes();
		
	// We need a temp allocator for temporary allocations during the physics update. 
	// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
	// malloc / free.
	m_allocator = new TempAllocatorImpl(10 * 1024 * 1024);

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.
	m_jobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

} 

void Physics::Destroy() {
	delete m_jobSystem;
	m_jobSystem = nullptr;

	delete m_allocator;
	m_allocator = nullptr;

	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;
}

void Physics::Update(Scene* scene) {
	auto physicsScene = scene->physicsScene();
	auto physicsSystem = physicsScene->physicsSystem();

	// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
	const int cCollisionSteps = 1;

	// If you want more accurate step results you can do multiple sub steps within a collision step. Usually you would set this to 1.
	const int cIntegrationSubSteps = 1;
	
	physicsScene->BeginUpdate();

	// Step the world
	physicsSystem->Update(m_game->deltaFixedTime(), cCollisionSteps, cIntegrationSubSteps, m_allocator, m_jobSystem);
}


// Callback for traces, connect this to your own trace function if you have one
static void TraceImpl(const char* inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Print to the TTY
	std::cout << buffer << std::endl;
}

// Callback for asserts, connect this to your own assert handler if you have one
static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
{
	// Print to the TTY
	std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << std::endl;

	// Breakpoint
	return true;
};