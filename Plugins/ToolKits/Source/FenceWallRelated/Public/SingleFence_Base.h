// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "Interface/FenceInterface.h"
#include "SingleFence_Base.generated.h"

class UBoxComponent;
class UTimelineComponent;
class FOnTimelineFloat;
class UCurveFloat;
class FOnTimelineEvent;

/**
 *	单个围栏基类
 */
UCLASS()
class FENCEWALLRELATED_API ASingleFence_Base : public AActor, public IFenceInterface
{
	GENERATED_BODY()

public:
	ASingleFence_Base();

	// 初始化绑定
	void InitBind();

	// 初始化基本参数
	void InitBase();

protected:
	virtual void BeginPlay() override;

	// 重写，用于在构造函数中设置属性
	virtual void OnConstruction(const FTransform& Transform) override;

	// 显示围栏
	virtual void ShowFence_Implementation() override;

	// 改变围栏颜色
	virtual void ChangeFenceColor_Implementation(const FLinearColor NewColor) override;

	// 围栏被命中
	virtual void FenceHit_Implementation(const bool CanShake, const float Angle) override;

	// 移除围栏
	virtual void RemoveFence_Implementation() override;

	// 碰撞盒
	UPROPERTY(VisibleDefaultsOnly, Category=Component)
	TObjectPtr<UBoxComponent> Box;

	// 围栏组件
	UPROPERTY(VisibleDefaultsOnly, Category=Component)
	TObjectPtr<UStaticMeshComponent> FenceMeshComponent;

	// 围栏模型
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "模型"))
	TObjectPtr<UStaticMesh> FenceMesh;
	// 阵营颜色
	UPROPERTY(EditAnywhere, Category="默认", meta=(DisplayName = "颜色"))
	FLinearColor CampColor = FLinearColor::Green;

	// 是否设置绿膜材质
	UPROPERTY(EditDefaultsOnly, Category="默认")
	uint8 bSetGreenFilm : 1;
	// 默认材质
	UPROPERTY()
	TArray<UMaterialInterface*> DefaultMaterials;
	// 绿膜材质
	UPROPERTY(EditDefaultsOnly, Category="默认")
	UMaterialInterface* GreenFilmMaterial;

	// 绿膜材质计时器
	FTimerHandle GreenFilmTimerHandle;

	// 开启绿膜材质
	void StartGreenFilm();

	// 绿膜材质结束
	UFUNCTION()
	void GreenFilmFinish();

	// 绿膜材质持续时间
	UPROPERTY(EditDefaultsOnly, Category="默认")
	float GreenFilmTime = 0.15f;

	// 是否可以震动
	uint8 bCanShake : 1;

	// 震动角度
	float ShakeAngle = 0.f;

	// 是否在命中
	uint8 bInHit : 1;

private:
	// 初始大小
	UPROPERTY()
	FVector StartSize;

	// 缩放时间
	UPROPERTY(EditDefaultsOnly, Category="默认")
	float ScaleTime = 0.3f;

	// 缩放曲线
	UPROPERTY(EditDefaultsOnly, Category="默认")
	TObjectPtr<UCurveFloat> ScaleCurve;
	// 缩放时间轴
	UPROPERTY()
	TObjectPtr<UTimelineComponent> ScaleTimeComponent;
	// 更新时间浮点轨道的代理
	UPROPERTY()
	FOnTimelineFloat UpdateFunctionScale;
	// 缩放完成事件的代理
	UPROPERTY()
	FOnTimelineEvent OnScaleFinishFunction;
	// 更新缩放
	UFUNCTION()
	void UpdateScaleTimeComponent(const float Output);
	// 缩放完成
	UFUNCTION()
	void OnScaleFinish();

	// 缩放播放
	void ScalePlay();

	// 击中时间
	UPROPERTY(EditDefaultsOnly, Category="默认")
	float HitTime = 0.3f;

	// 击中曲线
	UPROPERTY(EditDefaultsOnly, Category="默认")
	TObjectPtr<UCurveFloat> HitCurve;
	// 击中时间轴
	UPROPERTY()
	TObjectPtr<UTimelineComponent> HitTimeComponent;
	// 更新时间浮点轨道的代理
	UPROPERTY()
	FOnTimelineFloat UpdateFunctionHit;
	// 击中完成事件的代理
	UPROPERTY()
	FOnTimelineEvent OnHitFinishFunction;
	// 更新击中
	UFUNCTION()
	void UpdateHitTimeComponent(const float Output);
	// 击中完成
	UFUNCTION()
	void OnHitFinish();
	// 击中播放
	UFUNCTION()
	void HitPlayer();

	// 击中计时器
	FTimerHandle HitTimerHandle;

public:
	// 设置围栏模型
	FORCEINLINE void SetFenceMesh(const TObjectPtr<UStaticMesh> NewStaticMesh)
	{
		FenceMesh = NewStaticMesh;
	}

	// 获取围栏模型
	FORCEINLINE TObjectPtr<UStaticMesh> GetFenceMesh() const { return FenceMesh; }

	// 设置阵营颜色
	FORCEINLINE void SetCampColor(const FLinearColor NewColor)
	{
		CampColor = NewColor;
	}

	/**
	 * 设置缩放曲线
	 * @param NewCurve 缩放曲线
	 */
	FORCEINLINE void SetScaleFloat(const TObjectPtr<UCurveFloat> NewCurve) { if (NewCurve)ScaleCurve = *NewCurve; };
	/**
	 * 设置击中曲线
	 * @param NewCurve 击中曲线
	 */
	FORCEINLINE void SetHitFloat(const TObjectPtr<UCurveFloat> NewCurve) { if (NewCurve)HitCurve = *NewCurve; };
};
