#include "SMono.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/mono-debug.h"

#include "d3d11.h"
#include "../monopp/mono_build_config.h"

#define ASSEMBLY_PATH "C:/GitHub/FireEngine/x64/Debug/netstandard2.0/Mono.dll"



char* ReadFile(const std::filesystem::path& assemblyPath, uintmax_t& fileSize)
{
	std::fstream file;
	file.open(assemblyPath, std::ios_base::in | std::ios::binary);

	if (!file.is_open()) {
		// Lets talk about it!
		return nullptr;
	}
	fileSize = std::filesystem::file_size(assemblyPath);

	char* fileData = new char[fileSize];
	file.read(fileData, fileSize);

	return fileData;
}

static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath)
{
	uintmax_t fileSize;


	auto fileData = ReadFile(assemblyPath, fileSize);

	// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
	MonoImageOpenStatus status;
	MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

	if (status != MONO_IMAGE_OK) {
		const char* errorMessage = mono_image_strerror(status);
		// Log some error message using the errorMessage data
		return nullptr;
	}

#ifdef _DEBUG
	std::filesystem::path pdbPath = assemblyPath;
	pdbPath.replace_extension("pdb");

	if (std::filesystem::exists(pdbPath)) {
		uintmax_t pdbFileSize;
		auto pdb_data = ReadFile(pdbPath, pdbFileSize);

		mono_debug_open_image_from_memory(image, (const mono_byte*)pdb_data, pdbFileSize);

		delete[] pdb_data;
	}
#endif

	std::string pathString = assemblyPath.string();
	MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.data(), &status, 0);
	mono_image_close(image);

	delete[] fileData;

	return assembly;
}



SMono::SMono()
{
	Init();
}


void SMono::Init()
{
	mono_set_assemblies_path(INTERNAL_MONO_ASSEMBLY_DIR);
	//mono_set_assemblies_path("../../misc/mono/lib"); 
	//mono_set_dirs(".", ".");


//#if defined(_DEBUG) && defined(MONO_DEBUG)
	const char* params[2] = {
		"--debugger-agent=transport=dt_socket,server=y,address=127.0.0.1:55555",
		"--use-fallback-tls"
	};
	mono_jit_parse_options(2, (char**)params);
	mono_debug_init(MONO_DEBUG_FORMAT_MONO);
//#endif

	RootDomain = mono_jit_init("GameFrameworkJIT");

	AppDomain = mono_domain_create_appdomain(const_cast<char*>("GameDomain"), nullptr);
	mono_domain_set(AppDomain, true);

	CoreAssembly = LoadMonoAssembly(ASSEMBLY_PATH);
	CoreAssemblyImage = mono_assembly_get_image(CoreAssembly);

	PrintAssemblyTypes(CoreAssembly);
}


void SMono::Reload()
{
	mono_domain_set(RootDomain, false);
	mono_domain_unload(AppDomain);

	AppDomain = mono_domain_create_appdomain(const_cast<char*>("GameDomain"), nullptr);
	mono_domain_set(AppDomain, true);

	CoreAssembly		= LoadMonoAssembly(ASSEMBLY_PATH);
	CoreAssemblyImage	= mono_assembly_get_image(CoreAssembly);
}


void SMono::PrintAssemblyTypes(MonoAssembly* assembly)
{
	MonoImage* image = mono_assembly_get_image(assembly);
	const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
	int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

	for (int32_t i = 0; i < numTypes; i++)
	{
		uint32_t cols[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

		const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
		const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

		std::cout << nameSpace << "::" << name << "\n";
	}
}
