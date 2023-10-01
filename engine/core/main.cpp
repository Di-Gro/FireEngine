#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>

#include "monopp/mono_jit.h"
#include "monopp/mono_domain.h"
#include "monopp/mono_assembly.h"
#include "monopp/mono_type.h"
#include "monopp/mono_method_invoker.h"

#include "CSLinked.h"
#include "CppClass.h"
#include "ClassInfo.h"
#include "MonoInst.h"

#include "Game.h"
#include "Bridge.h"
#include "smono/SMono.h"

namespace fs = std::filesystem;

#define ASSEMBLY_PATH "../../engine/mono/bin/Debug/netstandard2.1/Mono.dll"

static void RunGame(MonoInst* imono) {
	try {
		Game game;
		game.Init(imono);

		try {
			game.Run();
		}
		catch (std::exception ex) {
			std::cout << "+: Game Exception: \n" << ex.what() << "'\n";
		}
	}
	catch (mono::mono_thunk_exception ex) {
		std::cout << "+: mono_exception: \n" << ex.what() << "'\n";
	}
}

static void InsideAssemply(MonoInst* imono) {
	RunGame(imono);
}

static void InsideMono() {
	mono::mono_domain my_domain("fire_engine_domain");
	mono::mono_domain::set_current_domain(my_domain);

	MonoInst* imono = nullptr;
	
	try {
		auto assembly = my_domain.get_assembly(ASSEMBLY_PATH);
		auto monoInst = MonoInst(&my_domain, &assembly);
		imono = &monoInst;
	} 
	catch (mono::mono_exception ex) {
		std::cout << "+: mono_exception: \n" << ex.what() << "'\n";
	}

	if (imono == nullptr)
		return;

	InsideAssemply(imono);
}

enum class FireMode { None, WaitDebugger, };

FireMode GetFireModeArg(int argc, char* argv[]) {
	std::string fireModeTag = "FireMode:";

	for (int i = 1; i < argc; i++) {
		auto arg = std::string(argv[i]);

		if (arg.starts_with(fireModeTag)) {

			auto tagSize = fireModeTag.size();
			auto value = arg.substr(tagSize, arg.size() - tagSize);

			if (value == "WaitDebugger")
				return FireMode::WaitDebugger;
		}
	}
	return FireMode::None;
}

int main(int argc, char* argv[]) {
	auto mode = GetFireModeArg(argc, argv);

	bool useMonoDebug = mode == FireMode::WaitDebugger;

	if(useMonoDebug)
		std::cout << "Waiting for debagger" << std::endl;

	if (!mono::init("mono", useMonoDebug))
		return 1;

	InsideMono();

	mono::shutdown();
	std::cout << "EXIT_OK" << std::endl;
}
