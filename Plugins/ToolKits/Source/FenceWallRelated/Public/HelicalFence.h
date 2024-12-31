// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HelicalFence.generated.h"

class USplineComponent; // 样条线
class UHierarchicalInstancedStaticMeshComponent; // 静态网格实例化
class ASingleFence_Base; // 单一围栏
class AFenceSpline; // 围栏样条线

/**
 * 螺旋围栏样条线
 */
UCLASS()
class FENCEWALLRELATED_API AHelicalFence : public AActor
{
	GENERATED_BODY()

public:
	AHelicalFence();

	// 样条线
	UPROPERTY(VisibleDefaultsOnly, Category=Component)
	TObjectPtr<USplineComponent> Spline;

	// 围栏样条线
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "围栏样条线"))
	TObjectPtr<AFenceSpline> FenceSpline;

	// 单一围栏
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "围栏类"))
	TSubclassOf<ASingleFence_Base> SingleFenceClass;

	// 显示模型
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "显示的模型"))
	TArray<TObjectPtr<UStaticMesh>> DisplayModels;

	// 显示数量
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "显示数量"))
	int32 DisplayNum = 10;

	// 实例化网格
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="默认", meta=(DisplayName = "实例化网格"))
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> InstancedStaticMeshComponents;

	// 模型大小
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "模型大小"))
	float Size = 1.f;

	// 间距
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "间距"))
	float Interval = 2.f;

	// 点在左还是右
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "右"))
	uint8 Around : 1;

	// 获取绕线方向
	float GeAround() const
	{
		if (Around) return 1.f;
		else return -1.f;
	}

	// 顺时针
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "顺时针旋转"))
	uint8 Clockwise : 1;

	// 线段点间距
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "点间距"))
	float PointInterval = 20.f;

	// 实际线段点间距
	float GetPointInterval() const;

	// 螺旋线到中心的距离
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "中心距离"))
	float CentreDistance = 10.f;

	// 获取螺旋线到中心的距离
	float GetCentreDistance() const;

	// 螺旋线的间距
	UPROPERTY(EditAnywhere, meta=(UIMin = 0.f, UIMax = 1.f), Category="默认", DisplayName = "螺旋线间距")
	float HelicalInterval = 0.1f;

	// 颜色
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "颜色"))
	FLinearColor CampColor = FLinearColor::Green;

	// 进度
	UPROPERTY(EditAnywhere, Interp, meta=(UIMin = 0.f, UIMax = 1.f), Category="默认", DisplayName = "进度")
	float Progress = 0.f;

	// 开始
	UPROPERTY(EditAnywhere, Interp, Category="默认", DisplayName = "开始运行")
	uint8 bStart : 1;

	// 实际长度
	UPROPERTY(VisibleAnywhere, Category="默认", meta=(DisplayName = "实际长度"))
	float ActualLength = 0.f;

	// 所有点
	UPROPERTY()
	TArray<FVector> Points = TArray<FVector>();

protected:
	virtual void BeginPlay() override;
	// 构造
	virtual void OnConstruction(const FTransform& Transform) override;

	/**
	 * 初始化围栏组件
	 * @param Component	围栏组件
	 * @param NewStaticMesh 新的静态网格
	 */
	void InitializeComponent(TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Component, TObjectPtr<UStaticMesh> NewStaticMesh);

public:
	virtual void Tick(float DeltaTime) override;

	/**
	* 获取模型长度
	* 
	* @param Index 围栏组件的索引
	* @return 返回围栏组件的网格长度，如果指定的围栏组件不存在，则返回零向量
	*/
	FVector GetMeshLength(int32 Index);

	// 获取临时变换
	TArray<FTransform> GetTempTransforms();
	// 设置样条线位置
	void SetSplineLocation();

	// 添加显示模型
	void AddDisplayModel();

	// 所有围栏
	UPROPERTY(BlueprintReadOnly, Category="默认", meta=(DisplayName = "拥有的所有围栏"))
	TArray<TObjectPtr<ASingleFence_Base>> AllSingleFences;

	// 生成围栏
	UFUNCTION(BlueprintCallable, Category="默认")
	void GeneratingFences();

	// 围栏显示隐藏
	void FenceHidden();
};
