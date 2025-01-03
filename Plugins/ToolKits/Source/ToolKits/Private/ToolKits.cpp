// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToolKits.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "YC_Log.h"
#include "Logging/LogMacros.h"

#include "Misc/Paths.h"
#include "Misc/MessageDialog.h"
#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "FToolKitsModule"

// 密钥
static FString MyKey = FString::Printf(TEXT("YiChen"));

// 许可证到期日期
static FString LicenceTime = "2026.01.01-00.00.00"; // 年/月/日-时/分/秒

// 缓存的有效时间（秒）
static constexpr int32 CacheDuration = 60 * 60 * 24 * 1; // 60 * 60 * 24 * 1; // （60*60 = 1小时） 缓存1天

// 最大次数
static constexpr int32 MaxRetryCount = 9;

// 当前网络重试次数
static int32 CurrentRetryCount = 0;

void FToolKitsModule::StartupModule()
{
	LoadConfig();
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	YICHEN_LOG(Warning, "当前自定义打印的时间：%s", *FDateTime::Now().ToString());
	// 读取缓存数据
	LoadCache();
}

void FToolKitsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	// 保存缓存数据
	SaveCache();
}

// 许可证有效性检查
void FToolKitsModule::CheckLicenseValidity()
{
	// 检查缓存是否有效
	if (CachedLicenseValid && FDateTime::Now() - LastCheckTime < FTimespan::FromSeconds(CacheDuration))
	{
		YICHEN_LOG(Display, "缓存有效！！！");
		Authorization(IsLicenseValid(LastCheckTime));
		return;
	}
	else
	{
		YICHEN_LOG(Warning, "缓存无效！！！");
	}

	// 创建一个HTTP请求对象 这里使用的是UE的HTTP模块
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	// 设置请求的URL。这个URL指向一个可以返回当前UTC时间的API
	Request->SetURL(TEXT("https://worldtimeapi.org/api/timezone/Etc/UTC"));

	// 设置请求的方法为GET，表示这是一个获取数据的请求。
	Request->SetVerb(TEXT("GET"));

	// 绑定一个回调函数，这个函数会在HTTP请求完成时被调用。
	// BindRaw需要两个参数，第一个是要绑定的对象，第二个是成员函数指针。
	// 当请求完成时，无论成功还是失败，OnTimeResponseReceived都会被调用。
	Request->OnProcessRequestComplete().BindRaw(this, &FToolKitsModule::OnTimeResponseReceived);

	// 发送请求。这个调用是异步的，意味着它会立即返回，而请求的结果会在稍后通过回调函数返回。
	Request->ProcessRequest();
}

// HTTP请求完成时的回调函数
void FToolKitsModule::OnTimeResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	YICHEN_LOG(Log, "正在进行(%d/%d)验证..", CurrentRetryCount, MaxRetryCount);
	if (bWasSuccessful && Response.IsValid())
	{
		// 解析响应中的时间
		TSharedPtr<FJsonObject> JsonObject;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			const FString DateTimeString = JsonObject->GetStringField("datetime"); // 根据API的响应格式获取时间字符串

			// 解析时间字符串(实际是网络的需要单独解析)
			FDateTime NewTime;
			if (FDateTime::ParseIso8601(*DateTimeString, NewTime))
			{
				// 比较网络时间和许可证到期时间
				CachedLicenseValid = true;
				LastCheckTime = NewTime.UtcNow() + FTimespan(8, 0, 0); // 加8时区
				Authorization(IsLicenseValid(LastCheckTime));
			}
			else
			{
				RerequestNetwork();
			}
		}
	}
	else
	{
		RerequestNetwork();
	}
}

// 检查许可证是否有效
bool FToolKitsModule::IsLicenseValid(FDateTime CurrentDate)
{
	// 定义一个FDateTime变量来存储解析后的到期日期
	FDateTime ExpiryDate;

	// 将存储在LicenceTime字符串中的到期日期解析到ExpiryDate变量中
	FDateTime::Parse(LicenceTime, ExpiryDate);

	// 如果当前日期小于或等于到期日期，则返回true（表示许可证有效）
	// 否则返回false（表示许可证无效）
	return CurrentDate <= ExpiryDate;
}

// 授权
void FToolKitsModule::Authorization(bool bValid)
{
	// 判断授权
	if (bValid)
	{
		YICHEN_LOG(Display, "插件授权有效！！！");
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT(""
			                     "\tPlease contact the plug author for permission！！！\n"
			                     "\t(请联系插头作者获得许可！！！) \n"
			                     "\t2394439184@qq.com")));
		YICHEN_LOG(Error, "插件授权无效");

		FGenericPlatformMisc::RequestExit(false);
	}
}

// 重新请求网络
void FToolKitsModule::RerequestNetwork()
{
	// 重新请求
	if (CurrentRetryCount < MaxRetryCount)
	{
		CurrentRetryCount++;
		YICHEN_LOG(Warning, "请求失败，正在重试...（%d/%d）", CurrentRetryCount, MaxRetryCount);
		CheckLicenseValidity();
	}
	else
	{
		YICHEN_LOG(Error, "无法获取网络时间");
		Authorization(IsLicenseValid());
	}
}

// 简单的XOR加密/解密函数
FString XorEncryptDecrypt(const FString& Input, const FString& Key)
{
	FString Output;
	for (int32 i = 0; i < Input.Len(); ++i)
	{
		// 获取当前字符
		const TCHAR InputChar = Input[i];
		// 获取对应的密钥字符，循环使用密钥
		const TCHAR KeyChar = Key[i % Key.Len()];
		// 进行XOR操作
		const TCHAR EncryptedChar = InputChar ^ KeyChar;
		// 将结果添加到输出字符串
		Output += EncryptedChar;
	}
	return Output;
}

// 写入加密数据
void FToolKitsModule::EncryptingData() const
{
	// 将布尔值和时间转换为字符串
	FString ValidStr = CachedLicenseValid ? TEXT("1") : TEXT("0");
	FString TimeStr = LastCheckTime.ToString();

	YICHEN_LOG(Display, "保存的时间：%s", *LastCheckTime.ToString());

	// 将字符串连接起来
	FString Data = ValidStr + TEXT("\n") + TimeStr;
	// 使用XOR加密
	FString EncryptedData = XorEncryptDecrypt(Data, MyKey);

	// 将加密后的数据保存到文件
	FFileHelper::SaveStringToFile(EncryptedData, *FilePath);
}

// 保存缓存数据
void FToolKitsModule::SaveCache() const
{
	// 确保目录存在
	if (!FPaths::FileExists(FilePath))
	{
		FFileHelper::SaveStringToFile(TEXT(""), *FilePath, FFileHelper::EEncodingOptions::ForceUTF8);
		YICHEN_LOG(Log, "%s\t缓存文件创建成功: %s", *FDateTime::Now().ToString(), *FilePath);
		EncryptingData();
	}
	else
	{
		EncryptingData();
	}
}

// 加载缓存数据
void FToolKitsModule::LoadCache()
{
	// 保存路径
	Path = FPaths::Combine(FPaths::ProjectPluginsDir(),TEXT("ToolKits/Saved"));
	// 缓存文件名称
	CacheFileName = TEXT("ToolKitsCache.ini");
	// 缓存文件路径
	FilePath = FPaths::Combine(Path, CacheFileName);

	if (FPaths::FileExists(FilePath))
	{
		// 从文件中读取加密后的数据
		FString EncryptedData;
		if (FFileHelper::LoadFileToString(EncryptedData, *FilePath))
		{
			// 使用XOR解密
			FString DecryptedData = XorEncryptDecrypt(*EncryptedData, MyKey);

			// 解析数据
			TArray<FString> Parts;
			DecryptedData.ParseIntoArray(Parts,TEXT("\n"), true);
			if (Parts.Num() == 2)
			{
				CachedLicenseValid = Parts[0] == TEXT("1");

				FDateTime::Parse(*Parts[1], LastCheckTime);

				YICHEN_LOG(Display, "读取的时间：%s\t当前时间%s", *LastCheckTime.ToString(), *FDateTime::Now().ToString());
			}
		}
	}
	// 检查许可证有效性
	CheckLicenseValidity();
}

/*// 创建文件
void FToolKitsModule::CreateFiles(const FString& Path, const FString& FileName)
{
	if (FPaths::FileExists(Path + "/" + FileName))
	{
		UE_LOG(YiChenLog, Display, TEXT("文件有效！！！"));
		return;
	}
	else
	{
		// 创建文件
		FString FilePath = FPaths::Combine(Path, FileName);

		// 创建文件
		FFileHelper::SaveStringToFile(TEXT(""), *FilePath);
		YICHEN_LOG( Display, "创建成功！！！");
		return;
	}
}*/

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FToolKitsModule, ToolKits)
