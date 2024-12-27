// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToolFunctionLibrary.generated.h"

/**
 * 工具函数库
 */
UCLASS()
class TOOLKITS_API UToolFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
	/**								贝塞尔曲线						
	 * @param Points				所有的点
	 * @param CurveTime				曲线的当前时间
	 * @param TotalTime				曲线的总时长
	 */
	UFUNCTION(BlueprintPure, BlueprintCallable, meta=(DisplayName = "GetBezierCurve", Keywords = "贝塞尔曲线"), Category="ToolKits|FunctionLibrary")
	static FVector BezierCurve(TArray<FVector> Points, const float CurveTime, const float TotalTime = 1.f);
};
