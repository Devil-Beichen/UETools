#include "FogOfWar.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/RenderCore/Public/ShaderCore.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/FileManager.h"

#define LOCTEXT_NAMESPACE "FFogOfWarModule"

void FFogOfWarModule::StartupModule()
{
	InitShadersPath();
}

void FFogOfWarModule::ShutdownModule()
{
}

// 初始化着色器路径
void FFogOfWarModule::InitShadersPath()
{
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*PluginsShaderDirectory);
	AddShaderSourceDirectoryMapping(TEXT("/ToolKitsMaterial"), PluginsShaderDirectory);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFogOfWarModule, FogOfWar)
