#include "MonoProcess.h"
#include "MonoDeclarations.h"
#include "../BlackBone/src/BlackBone/Process/RPC/RemoteFunction.hpp"

namespace MInjectNative
{
	template<typename T>
	inline auto MonoProcess::GetMethodFromExport(char * p_MethodName)
	{
		auto exportData = m_InnerProcess.modules().GetExport(m_MonoModule, p_MethodName);
		blackbone::RemoteFunction<T> remoteFunction{ m_InnerProcess, exportData->procAddress };

		return remoteFunction;
	}

	MonoProcess::MonoProcess(DWORD p_ProcessId)
	{
		m_InnerProcess.Attach(p_ProcessId);
		m_MonoModule = m_InnerProcess.modules().GetModule(L"mono.dll");

		m_InnerProcess.remote().CreateRPCEnvironment(blackbone::Worker_CreateNew);
		m_WorkerThread = m_InnerProcess.remote().getWorker();
	}

	intptr_t MonoProcess::GetRootDomain()
	{
		auto mono_get_root_function = GetMethodFromExport<MonoDeclarations::mono_get_root_domain>("mono_get_root_domain");
		decltype(mono_get_root_function)::CallArguments args{};

		return mono_get_root_function.Call(args, m_WorkerThread).result();
	}

	intptr_t MonoProcess::AssemblyLoadFromFull(intptr_t p_Image)
	{
		auto mono_assembly_load_from_function = GetMethodFromExport<MonoDeclarations::mono_assembly_load_from_full>("mono_assembly_load_from_full");
		int monoImageOpenStatus;

		decltype(mono_assembly_load_from_function)::CallArguments args{
			p_Image,
			nullptr,
			&monoImageOpenStatus,
			false
		};

		return mono_assembly_load_from_function.Call(args, m_WorkerThread).result();
	}

	intptr_t MonoProcess::AssemblyGetImage(intptr_t p_Assembly)
	{
		auto mono_assembly_get_image_function = GetMethodFromExport<MonoDeclarations::mono_assembly_get_image>("mono_assembly_get_image");
		decltype(mono_assembly_get_image_function)::CallArguments args{ p_Assembly };

		return mono_assembly_get_image_function.Call(args, m_WorkerThread).result();
	}

	intptr_t MonoProcess::ImageOpenFromDataFull(const std::vector<byte>& p_Image)
	{
		auto mono_image_open_from_data_export = GetMethodFromExport<MonoDeclarations::mono_image_open_from_data_full>("mono_image_open_from_data_full");

		int monoImageOpenStatus;
		auto allocatedMem = m_InnerProcess.memory().Allocate(p_Image.size(), PAGE_READWRITE);
		allocatedMem->Write(0, p_Image.size(), p_Image.data());

		decltype(mono_image_open_from_data_export)::CallArguments args{
			allocatedMem->ptr<intptr_t>(),
			static_cast<uint32_t>(p_Image.size()),
			1,
			&monoImageOpenStatus,
			0
		};

		auto callResult = mono_image_open_from_data_export.Call(args, m_WorkerThread).result();
		allocatedMem->Free();

		return callResult;
	}

	intptr_t MonoProcess::ThreadAttach(intptr_t p_Domain)
	{
		auto mono_thread_attach_function = GetMethodFromExport<MonoDeclarations::mono_thread_attach>("mono_thread_attach");
		decltype(mono_thread_attach_function)::CallArguments args{ p_Domain };

		return mono_thread_attach_function.Call(args, m_WorkerThread).result();
	}
	
	int MonoProcess::ThreadDetach(intptr_t p_Domain)
	{
		auto mono_thread_detach_function = GetMethodFromExport<MonoDeclarations::mono_thread_detach>("mono_thread_detach");
		decltype(mono_thread_detach_function)::CallArguments args{ p_Domain };

		return mono_thread_detach_function.Call(args, m_WorkerThread).result();
	}

	void MonoProcess::SecuritySetMode(int p_Mode)
	{
		auto mono_security_set_mode_function = GetMethodFromExport<MonoDeclarations::mono_security_set_mode>("mono_security_set_mode");
		decltype(mono_security_set_mode_function)::CallArguments args{ p_Mode };

		mono_security_set_mode_function.Call(args, m_WorkerThread);
	}

	intptr_t MonoProcess::ClassFromName(intptr_t p_Image, const char* p_Namespace, const char* p_ClassName)
	{
		auto mono_class_from_name_function = GetMethodFromExport<MonoDeclarations::mono_class_from_name>("mono_class_from_name");
		decltype(mono_class_from_name_function)::CallArguments args{
			p_Image,
			p_Namespace,
			p_ClassName
		};

		return mono_class_from_name_function.Call(args, m_WorkerThread).result();
	}

	intptr_t MonoProcess::ClassGetMethodFromName(intptr_t p_Class, const char* p_MethodName)
	{
		auto mono_class_get_method_function = GetMethodFromExport<MonoDeclarations::mono_class_get_method_from_name>("mono_class_get_method_from_name");
		decltype(mono_class_get_method_function)::CallArguments args{
			p_Class,
			p_MethodName,
			0
		};

		return mono_class_get_method_function.Call(args, m_WorkerThread).result();
	}

	intptr_t MonoProcess::RuntimeInvoke(intptr_t p_Method)
	{
		auto mono_runtime_invoke_function = GetMethodFromExport<MonoDeclarations::mono_runtime_invoke>("mono_runtime_invoke");
		decltype(mono_runtime_invoke_function)::CallArguments args{
			p_Method,
			nullptr,
			nullptr,
			nullptr
		};

		return mono_runtime_invoke_function.Call(args, m_InnerProcess.threads().getMain()).result();
	}
	
	bool MonoProcess::DisableAssemblyLoadCallback()
	{
		/*
		*	Ref: https://github.com/Unity-Technologies/mono/blob/unity-staging/mono/metadata/assembly.c
		*
		*	=> AssemblyLoadHook *assembly_load_hook = NULL;
		*	AssemblyLoadHook is a global variable that holds all the callbacks that needs to be fired when an assembly is loaded.
		*	
		*	Plan is to get an hold of it, null the pointer and restore it after loading our injected assembly.
		*/

		if (m_OriginalAssemblyLoadPtrVal == NULL)
		{
			if (m_AssemblyLoadPtr == NULL)
			{
				auto exportData = m_InnerProcess.modules().GetExport(m_MonoModule, "mono_assembly_invoke_load_hook");
				auto assemblyLoadHookRefAddr = exportData->procAddress + 6;
				m_AssemblyLoadPtr = m_InnerProcess.memory().Read<int>(assemblyLoadHookRefAddr).result();
			}

			//Keep the original pointer value so we can restore it.
			m_OriginalAssemblyLoadPtrVal = m_InnerProcess.memory().Read<int>(m_AssemblyLoadPtr).result();

			//Null the pointer so it doesn't points to existing callback list anymore
			m_InnerProcess.memory().Write<int>(m_AssemblyLoadPtr, 0);

			return true;
		}

		return false;
	}

	bool MonoProcess::EnableAssemblyLoadCallback()
	{
		if (m_OriginalAssemblyLoadPtrVal != NULL)
		{
			m_InnerProcess.memory().Write<int>(m_AssemblyLoadPtr, m_OriginalAssemblyLoadPtrVal);
			m_OriginalAssemblyLoadPtrVal = NULL;

			return true;
		}

		return false;
	}

	bool MonoProcess::HideLastAssembly(intptr_t p_Domain)
	{
		/*
		*	Ref: https://github.com/Unity-Technologies/mono/blob/unity-staging/mono/metadata/domain-internals.h
		*
		*	MonoDomain has a 'domain_assemblies' field that hold the loaded assemblies.
		*	The most recent loaded assembly will be the first in the list.
		*	Goal is to remove it :)
		*/

		auto assembler = blackbone::AsmFactory::GetAssembler(m_InnerProcess.modules().GetMainModule()->type);
		auto& asmHelper = *assembler;

		asmHelper.GenPrologue();

		//Save EDI, ECX regs
		asmHelper->push(asmHelper->zdi);
		asmHelper->push(asmHelper->zcx);

		//MOV EDI, p_Domain
		asmHelper->mov(asmHelper->zdi, p_Domain);

		//ADD EDI, 0x70 (Offset of domain_assemblies)
		asmHelper->add(asmHelper->zdi, 0x70);

		//MOV ECX, DWORD PTR DS[EDI]
		//Store domain_assemblies into a temp variable
		asmHelper->mov(asmHelper->zcx, asmHelper->intptr_ptr(asmHelper->zdi));

		//MOV ECX, 0x4
		//ECX now points to 2nd assembly in list
		asmHelper->add(asmHelper->zcx, 0x4);

		//MOV ECX, DWORD PTR DS[ECX]
		asmHelper->mov(asmHelper->zcx, asmHelper->intptr_ptr(asmHelper->zcx));

		//MOV DWORD PTR DS[EDI], ECX
		//Replace real domain_assemblies with our temp variable
		asmHelper->mov(asmHelper->intptr_ptr(asmHelper->zdi), asmHelper->zcx);

		//Restore EDI, ECX regs
		asmHelper->pop(asmHelper->zcx);
		asmHelper->pop(asmHelper->zdi);

		m_InnerProcess.remote().AddReturnWithEvent(asmHelper);
		asmHelper.GenEpilogue();

		uint64_t callResult = NULL;
		NTSTATUS result = m_InnerProcess.remote().ExecInWorkerThread(asmHelper->make(), asmHelper->getCodeSize(), callResult);

		return NT_SUCCESS(result);
	}
}
