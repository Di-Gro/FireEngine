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

#define ASSEMBLY_PATH "C:/GitHub/FireEngine/x64/Debug/netstandard2.0/Mono.dll"


static void RunGame(MonoInst* imono) {
	try {
		Game game;
		game.Init(imono);
		game.Run();
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
		
	if (!mono::init("mono", true))
		return 1;

	InsideMono();

	mono::shutdown();
}