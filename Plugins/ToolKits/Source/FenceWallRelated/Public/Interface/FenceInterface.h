// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FenceInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UFenceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 围栏城墙相关的接口 
 */
class FENCEWALLRELATED_API IFenceInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * 显示围栏
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="YC|城墙围栏相关")
	void ShowFence();

	/**	
	 * 更改围栏颜色
	 * @param NewColor 新的颜色
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="YC|城墙围栏相关")
	void ChangeFenceColor(const FLinearColor NewColor);

	/**
	 * 围栏被攻击
	 * @param CanShake	可以抖动
	 * @param Angle		角度
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="YC|城墙围栏相关")
	void FenceHit(const bool CanShake, const float Angle);

	// 移除围栏
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="YC|城墙围栏相关")
	void RemoveFence();
};
