#include <iostream>
#include <string>

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

	auto assembly = my_domain.get_assembly(ASSEMBLY_PATH);
	auto imono = MonoInst(&my_domain, &assembly);

	InsideAssemply(&imono);
}

int main() {	
	bool useMonoDebug = true;

	if (!mono::init("mono", useMonoDebug))
		return 1;

	InsideMono();

	mono::shutdown();
	std::cout << "EXIT_OK" << std::endl;
}