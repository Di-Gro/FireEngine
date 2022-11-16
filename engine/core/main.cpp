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

#define ASSEMBLY_PATH "C:/GitHub/FireEngine/engine/mono/bin/Debug/netstandard2.0/Mono.dll"


static void RunGame(MonoInst* imono) {
	Game game;
	game.Init(imono);
	game.Run();
}

static void InsideAssemply(MonoInst* imono) {

	//Ref<ClassInfo> classInfo;
	//classInfo.obj = ClassInfo::Create<CppClass>();

	//CSLinked<CppClass> cppObj(imono);

	//cppObj.Link("EngineMono", "CSClass", classInfo.RefId());

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