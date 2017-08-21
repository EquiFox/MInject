#include "MonoProcess.h"

using namespace System;
using namespace System::Diagnostics;
using namespace System::Runtime::InteropServices;

namespace MInject {

	public ref class MonoProcess
	{
	public:
		//Domain
		IntPtr GetRootDomain();

		//Assembly
		IntPtr AssemblyLoadFromFull(IntPtr p_ImageAddress);
		IntPtr AssemblyGetImage(IntPtr p_AssemblyAddress);

		//Images
		IntPtr ImageOpenFromDataFull(array<System::Byte>^ p_AssemblyBytes);

		//Threads
		IntPtr ThreadAttach(IntPtr p_DomainAddress);
		int ThreadDetach(IntPtr p_DomainAddress);

		//SecurityManager
		void SecuritySetMode(int p_Mode);

		//Class
		IntPtr ClassFromName(IntPtr p_ImageAddress, String^ p_Namespace, String^ p_ClassName);
		IntPtr ClassGetMethodFromName(IntPtr p_ImageAddress, String^ p_MethodName);

		//Utilities	
		IntPtr RuntimeInvoke(IntPtr p_MethodAddress);
		bool DisableAssemblyLoadCallback();
		bool EnableAssemblyLoadCallback();
		bool HideLastAssembly(IntPtr p_DomainAddress);

		static bool Attach(Process^ p_Process, [OutAttribute] MonoProcess^% p_MonoProcess);

	protected: 
		!MonoProcess() {
			delete m_Impl;
		}

	private:
		MInjectNative::MonoProcess* m_Impl;

		MonoProcess(int p_ProcessId) : m_Impl(new MInjectNative::MonoProcess(p_ProcessId)) {}
		~MonoProcess() {
			delete m_Impl;
		}
	};
}
