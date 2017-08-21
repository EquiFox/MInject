#pragma once

#include <stdint.h>

namespace MInjectNative
{
	namespace MonoDeclarations
	{
		using mono_get_root_domain = intptr_t(__cdecl*)();
		using mono_assembly_load_from_full = intptr_t(__cdecl*)(intptr_t, int*, int*, bool);
		using mono_assembly_get_image = intptr_t(__cdecl*)(intptr_t);
		using mono_image_open_from_data_full = intptr_t(__cdecl*)(intptr_t, uint32_t, int, int*, int);
		using mono_thread_attach = intptr_t(__cdecl*)(intptr_t);
		using mono_thread_detach = int(__cdecl*)(intptr_t);
		using mono_security_set_mode = void(__cdecl*)(int);				
		using mono_class_from_name = intptr_t(__cdecl*)(intptr_t, const char*, const char*);
		using mono_class_get_method_from_name = intptr_t(__cdecl*)(intptr_t, const char*, int);
		using mono_runtime_invoke = intptr_t(__cdecl*)(intptr_t, void*, void**, int**);
	}
}
