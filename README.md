# MInject
Mono Framework Interaction / Injection Library for .NET (C++/CLI)

## Features
- Core is in pure C++ for reliabilty and performance
- Native MonoProcess class is exposed to .NET throught a custom managed wrapper
- Allow easy interaction with the Mono namespace
- Allow easy injection into Mono namespace
- Provides experimental safety mechanisms to hide the injected assembly from `AssemblyLoad` callbacks and `GetAssemblies()`

## Download
You can find the most recent releases here: https://github.com/EquiFox/MInject/releases  
  
You don't have time to waste with coding ? No problem !  
Just grab my pre-compiled injector (based on this library) here: https://github.com/EquiFox/MInjector

## Usage
Add `MInject.dll` to your .NET project references.
Don't forget `using MInject;`

```c#
//Grab the target process by its name
Process targetProcess = Process.GetProcessesByName("Game")[0];
MonoProcess monoProcess;

//Try to attach to targetProcess Mono module
if (MonoProcess.Attach(targetProcess, out monoProcess))
{
    byte[] assemblyBytes = File.ReadAllBytes("TestInjection.dll");

    IntPtr monoDomain = monoProcess.GetRootDomain();
    monoProcess.ThreadAttach(monoDomain);
    monoProcess.SecuritySetMode(0);
    
    //Disable AssemblyLoad callbacks before injection
    monoProcess.DisableAssemblyLoadCallback();

    IntPtr rawAssemblyImage = monoProcess.ImageOpenFromDataFull(assemblyBytes);
    IntPtr assemblyPointer = monoProcess.AssemblyLoadFromFull(rawAssemblyImage);
    IntPtr assemblyImage = monoProcess.AssemblyGetImage(assemblyPointer);
    IntPtr classPointer = monoProcess.ClassFromName(assemblyImage, "TestInjection", "Loader");
    IntPtr methodPointer = monoProcess.ClassGetMethodFromName(classPointer, "Init");
    
    //Remove the last loaded assembly (Ours) from domain_assemblies field
    monoProcess.HideLastAssembly(monoDomain);
    
    //Finally invoke the TestInjection.Loader.Init method
    monoProcess.RuntimeInvoke(methodPointer);

    //Restore the AssemblyLoad callbacks to avoid weird behaviours
    monoProcess.EnableAssemblyLoadCallback();    
    
    //You MUST dispose the MonoProcess instance when finished
    monoProcess.Dispose();
}
```

## Safety Mechanisms
These features are currently only supported for `x86 processes`.  
Tested and confirmed working on `Albion Online` as of August 21, 2017.  
  
Since multiple variants of Mono exists, using these features on any other Mono process might result in a crash.

## Compile Yourself
- Requires Visual Studio 2017 (Toolset v141)
