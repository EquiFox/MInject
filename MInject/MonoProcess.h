#pragma once

#include "../BlackBone/src/BlackBone/Process/Process.h"

namespace MInjectNative
{
	class MonoProcess
	{
	public:
		MonoProcess(DWORD p_ProcessId);
		~MonoProcess() = default;

		//Domain
		intptr_t GetRootDomain();

		//Assembly
		intptr_t AssemblyLoadFromFull(intptr_t p_Image);
		intptr_t AssemblyGetImage(intptr_t p_Assembly);

		//Images
		intptr_t ImageOpenFromDataFull(const std::vector<byte>& p_Image);

		//Threads
		intptr_t ThreadAttach(intptr_t p_Domain);
		int ThreadDetach(intptr_t p_Domain);

		//SecurityManager
		void SecuritySetMode(int p_Mode);

		//Class
		intptr_t ClassFromName(intptr_t p_Image, const char* p_Namespace, const char* p_ClassName);
		intptr_t ClassGetMethodFromName(intptr_t p_Class, const char* p_MethodName);

		//Utilities			
		intptr_t RuntimeInvoke(intptr_t p_Method);
		bool DisableAssemblyLoadCallback();
		bool EnableAssemblyLoadCallback();
		bool HideLastAssembly(intptr_t p_Domain);

	private:
		blackbone::Process m_InnerProcess;
		blackbone::ThreadPtr m_WorkerThread;
		blackbone::ModuleDataPtr m_MonoModule;

		blackbone::ptr_t m_AssemblyLoadPtr = NULL;
		blackbone::ptr_t m_OriginalAssemblyLoadPtrVal = NULL;

		template<typename T>
		auto GetMethodFromExport(char* p_MethodName);
	};
}
