// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FenceSpline.generated.h"

class USplineComponent; // 样条线
class UHierarchicalInstancedStaticMeshComponent; // 静态网格实例化
class ASingleFence_Base; // 单一围栏

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
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "围栏类"))
	TSubclassOf<ASingleFence_Base> SingleFenceClass;

	// 默认显示
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "默认显示"))
	uint8 bDefaultDisplay : 1;

	// 显示模型
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "显示的模型"))
	TArray<UStaticMesh*> DisplayModels;

	// 显示数量
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "显示数量"))
	int32 DisplayNum = 1;

	// 实例化网格
	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent*> InstancedStaticMeshComponents;

	// 模型大小
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "模型大小"))
	float Size = 1.f;

	// 间距
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "间距"))
	float Interval = 2.f;

	// 颜色
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "颜色"))
	FLinearColor CampColor = FLinearColor::Green;

	// 所有围栏
	UPROPERTY(BlueprintReadOnly, Category="默认")
	TArray<TObjectPtr<ASingleFence_Base>> AllSingleFences;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	/**
	* 初始化围栏组件
	* @param Component	围栏组件
	* @param NewStaticMesh 新的静态网格
	*/
	void InitializeComponent(TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Component, TObjectPtr<UStaticMesh> NewStaticMesh);

private:
	// 获取模型长度
	FVector GetMeshLength(int32 Index);

	// 获取临时变换
	TArray<FTransform> GetTempTransforms();

	// 添加显示模型
	void AddDisplayModel();

public:
	// 生成围栏
	UFUNCTION(BlueprintCallable, Category="默认")
	void GeneratingFences();

	// 获取所有围栏
	FORCEINLINE TArray<TObjectPtr<ASingleFence_Base>> GetAllSingleFences() const { return AllSingleFences; };
};
