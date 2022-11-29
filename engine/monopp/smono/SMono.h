#pragma once

extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoDomain MonoDomain;
}


class SMono
{
public:
	MonoDomain* RootDomain = nullptr;
	MonoDomain* AppDomain = nullptr;

	MonoAssembly* CoreAssembly = nullptr;
	MonoImage* CoreAssemblyImage = nullptr;


	SMono();

	void Init();
	void Reload();

	void PrintAssemblyTypes(MonoAssembly* assembly);
};
