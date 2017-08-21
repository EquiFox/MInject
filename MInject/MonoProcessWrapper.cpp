#include "MonoProcessWrapper.h"

IntPtr MInject::MonoProcess::GetRootDomain()
{
	return IntPtr(m_Impl->GetRootDomain());
}

IntPtr MInject::MonoProcess::AssemblyLoadFromFull(IntPtr p_ImageAddress)
{
	return IntPtr(m_Impl->AssemblyLoadFromFull(reinterpret_cast<intptr_t>(p_ImageAddress.ToPointer())));
}

IntPtr MInject::MonoProcess::AssemblyGetImage(IntPtr p_AssemblyAddress)
{
	return IntPtr(m_Impl->AssemblyGetImage(reinterpret_cast<intptr_t>(p_AssemblyAddress.ToPointer())));
}

IntPtr MInject::MonoProcess::ImageOpenFromDataFull(array<System::Byte>^ p_AssemblyBytes)
{
	std::vector<byte> bytes(p_AssemblyBytes->Length);
	{
		pin_ptr<byte> pin(&p_AssemblyBytes[0]);
		byte *first(pin), *last(pin + p_AssemblyBytes->Length);
		std::copy(first, last, bytes.begin());
	}

	return IntPtr(m_Impl->ImageOpenFromDataFull(bytes));
}

IntPtr MInject::MonoProcess::ThreadAttach(IntPtr p_DomainAddress)
{
	return IntPtr(m_Impl->ThreadAttach(reinterpret_cast<intptr_t>(p_DomainAddress.ToPointer())));
}

int MInject::MonoProcess::ThreadDetach(IntPtr p_DomainAddress)
{
	return m_Impl->ThreadDetach(reinterpret_cast<intptr_t>(p_DomainAddress.ToPointer()));
}

void MInject::MonoProcess::SecuritySetMode(int p_Mode)
{
	m_Impl->SecuritySetMode(p_Mode);
}

IntPtr MInject::MonoProcess::ClassFromName(IntPtr p_ImageAddress, String^ p_Namespace, String^ p_ClassName)
{
	intptr_t imageAddress = reinterpret_cast<intptr_t>(p_ImageAddress.ToPointer());
	const char* targetNamespace = (const char*)(Marshal::StringToHGlobalAnsi(p_Namespace)).ToPointer();
	const char* targetClass = (const char*)(Marshal::StringToHGlobalAnsi(p_ClassName)).ToPointer();

	return IntPtr(m_Impl->ClassFromName(imageAddress, targetNamespace, targetClass));
}

IntPtr MInject::MonoProcess::ClassGetMethodFromName(IntPtr p_ImageAddress, String^ p_MethodName)
{
	intptr_t imageAddress = reinterpret_cast<intptr_t>(p_ImageAddress.ToPointer());
	const char* targetMethod = (const char*)(Marshal::StringToHGlobalAnsi(p_MethodName)).ToPointer();

	return IntPtr(m_Impl->ClassGetMethodFromName(imageAddress, targetMethod));
}

IntPtr MInject::MonoProcess::RuntimeInvoke(IntPtr p_MethodAddress)
{
	return IntPtr(m_Impl->RuntimeInvoke(reinterpret_cast<intptr_t>(p_MethodAddress.ToPointer())));
}

bool MInject::MonoProcess::DisableAssemblyLoadCallback()
{
	return m_Impl->DisableAssemblyLoadCallback();
}

bool MInject::MonoProcess::EnableAssemblyLoadCallback()
{
	return m_Impl->EnableAssemblyLoadCallback();
}

bool MInject::MonoProcess::HideLastAssembly(IntPtr p_DomainAddress)
{
	return m_Impl->HideLastAssembly(reinterpret_cast<intptr_t>(p_DomainAddress.ToPointer()));
}

bool MInject::MonoProcess::Attach(Process^ p_Process, [OutAttribute] MonoProcess^% p_MonoProcess)
{
	for each(ProcessModule^ mod in p_Process->Modules)
	{
		if (mod->FileName->Contains("mono.dll"))
		{
			p_MonoProcess = gcnew MonoProcess(p_Process->Id);
			return true;
		}
	}
	return false;
}