#pragma once
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(YiChenLog, Log, All);

// 全局Log启用开关，默认关闭
extern bool bYiChenLogEnable;

/**
 * YICHEN_LOG 宏定义用于条件性地记录日志消息。
 * 它根据 bYiChenLogEnable 标志决定是否记录日志。
 * 这个宏只在 bYiChenLogEnable 为真时执行日志记录操作。
 * 
 * @param Verbosity 日志的详细级别，决定了日志的重要性。
 * @param Format 格式字符串，描述了日志消息的结构。
 * @param ... 可变参数列表，包含格式字符串中的替换项。
 */
#define YICHEN_LOG(Verbosity, Format, ...) \
	do{\
		if(bYiChenLogEnable){\
			UE_LOG(YiChenLog,Verbosity,TEXT(Format),##__VA_ARGS__);\
		}\
	}while(0)

// 读取配置文件
void LoadConfig();
