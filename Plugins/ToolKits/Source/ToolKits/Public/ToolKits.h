// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Modules/ModuleManager.h"

class FToolKitsModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	// 网络检查许可证是否有效
	void CheckLicenseValidity();

	// 许可证是否有效
	static bool IsLicenseValid(FDateTime CurrentDate = FDateTime::Now());

	/**
	 * @brief						及时收到回应
	 * @param Request				请求
	 * @param Response				响应
	 * @param bWasSuccessful		是否成功
	 */
	void OnTimeResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// 授权
	static void Authorization(bool bValid);

	// 重新请求网络
	void RerequestNetwork();

private:
	// 缓存许可证是否有效
	bool CachedLicenseValid = false;
	// 上次检查时间
	FDateTime LastCheckTime = FDateTime::MinValue();

	// 加载缓存
	void LoadCache();

	// 保存缓存
	void SaveCache() const;

	/**
	 * 创建文件函数
	 * 
	 * 该函数用于在指定路径下创建一个文件，主要用于生成配置缓存文件
	 * 其目的是为了保存程序的配置信息，以便在程序重新启动时能够恢复到之前的状态
	 * 
	 * @param Path 文件路径，表示文件将要被创建在哪个目录下
	 * @param FileName 文件名，默认为"ConfigCache.ini"，这是为了提供一个默认的配置缓存文件名
	 *                  当调用者没有提供文件名时，将使用这个默认值
	 */
	// static void CreateFiles(const FString& Path, const FString& FileName = FString("ConfigCache.ini"));
};
