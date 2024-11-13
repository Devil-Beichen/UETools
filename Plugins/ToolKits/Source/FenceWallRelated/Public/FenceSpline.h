// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FenceSpline.generated.h"

class USplineComponent; // 样条线
class UHierarchicalInstancedStaticMeshComponent; // 静态网格实例化

/**
 * 围栏样条线
 */
UCLASS()
class FENCEWALLRELATED_API AFenceSpline : public AActor
{
	GENERATED_BODY()

public:
	AFenceSpline();

	// 样条线
	UPROPERTY(VisibleDefaultsOnly, Category=Component)
	TObjectPtr<USplineComponent> Spline;

	// 单一围栏
	UPROPERTY(EditAnywhere, Category="默认")
	TSubclassOf<class ASingleFence_Base> SingleFenceClass;

	// 显示模型
	UPROPERTY(EditAnywhere, Category="默认")
	TArray<UStaticMesh*> DisplayModel;

	// 显示数量
	UPROPERTY(EditAnywhere, Category="默认")
	int32 DisplayNum = 1;

	// 实例化网格
	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent*> InstancedStaticMeshComponents;

	// 模型大小
	UPROPERTY(EditAnywhere, Category="默认")
	float Size = 1.f;

	// 间距
	UPROPERTY(EditAnywhere, Category="默认")
	float Interval = 2.f;

	// 颜色
	UPROPERTY(EditAnywhere, Category="默认")
	FLinearColor CampColor = FLinearColor::Green;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	// 获取模型长度
	FVector GetMeshLength(int32 Index);

	// 获取临时变换
	TArray<FTransform> GetTempTransforms();

	// 添加显示模型
	void AddDisplayModel();

	// 所有围栏
	UPROPERTY()
	TArray<TObjectPtr<ASingleFence_Base>> AllSingleFences;

public:
	// 生成围栏
	UFUNCTION(BlueprintCallable, Category="默认")
	void GeneratingFences();
};
