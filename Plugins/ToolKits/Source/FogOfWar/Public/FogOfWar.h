#pragma once

#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

class FFogOfWarModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	// 插件Shader路径
	const FString PluginsShaderDirectory = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("ToolKits/Shaders"));

	// 初始化材质路径
	void InitShadersPath();
};
