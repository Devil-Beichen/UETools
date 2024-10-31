#include "YC_Log.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

DEFINE_LOG_CATEGORY(YiChenLog);

bool bYiChenLogEnable = false;

// 读取配置
void LoadConfig()
{
	// 保存路径
	FString Path = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("ToolKits/Config"));
	// 文件名称
	FString FileName = TEXT("YiChenLog.ini");
	// 文件路径
	FString FilePath = FPaths::Combine(Path, FileName);

	// 键
	FString Key = TEXT("bYiChenLogEnable");

	// 确保目录存在
	if (!FPaths::DirectoryExists(Path))
	{
		if (!FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*Path))
		{
			UE_LOG(YiChenLog, Error, TEXT("无法创建文件夹: %s"), *Path);
			return;
		}
	}

	// 如果文件不存在，则创建文件并写入默认内容
	if (!FPaths::FileExists(FilePath))
	{
		// 默认内容
		FString DefaultContent = TEXT("; true ? false 是否开启YiChenLog打印 \nbYiChenLogEnable = true\n");
		if (FFileHelper::SaveStringToFile(DefaultContent, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8))
		{
			UE_LOG(YiChenLog, Log, TEXT("%s\t配置文件创建成功: %s"), *FDateTime::Now().ToString(), *FilePath);
		}
		else
		{
			UE_LOG(YiChenLog, Error, TEXT("无法创建文件: %s"), *FilePath);
			return;
		}
	}
	else
	{
		// 读取文件内容
		FString FileContent;
		if (FFileHelper::LoadFileToString(FileContent, *FilePath))
		{
			// 解析配置内容
			TMap<FString, FString> ConfigMap;
			TArray<FString> Lines;
			FileContent.ParseIntoArray(Lines,TEXT("\n"), true);

			for (const FString& Line : Lines)
			{
				FString TempKey;
				FString TempValue;

				if (Line.Split(TEXT("="), &TempKey, &TempValue))
				{
					// 清理键值对，并放入 Map 中
					ConfigMap.Add(Key.TrimStartAndEnd(), TempValue.TrimStartAndEnd());
				}
			}

			// 读取配置
			bYiChenLogEnable = ConfigMap.FindRef(Key) == TEXT("true") ? true : false;
			// UE_LOG(YiChenLog, Log, TEXT("读取到的值是:%d"), bYiChenLogEnable);
		}
	}
}
