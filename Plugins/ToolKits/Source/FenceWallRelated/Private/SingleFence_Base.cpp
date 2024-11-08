// Fill out your copyright notice in the Description page of Project Settings.


#include "SingleFence_Base.h"
#include "Components/TimelineComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"

ASingleFence_Base::ASingleFence_Base()
{
	// 设置此actor的tick 为false 
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComponent);
	Box->SetGenerateOverlapEvents(true);
	Box->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Box->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	Box->SetCollisionObjectType(ECC_WorldDynamic);
	Box->SetUseCCD(true);

	FenceMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FenceMesh"));
	FenceMeshComponent->SetupAttachment(RootComponent);
	FenceMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	ScaleTimeComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("ScaleTimeComponent"));
	HitTimeComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("HitTimeComponent"));
}

void ASingleFence_Base::BeginPlay()
{
	Super::BeginPlay();
	InitBase();
	InitBind();
	StartSize = FenceMeshComponent->GetComponentScale();
}

// 构造函数
void ASingleFence_Base::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	InitBase();
}

// 初始化绑定
void ASingleFence_Base::InitBind()
{
	if (!UpdateFunctionScale.IsBound())
		UpdateFunctionScale.BindDynamic(this, &ThisClass::UpdateScaleTimeComponent);
	if (!OnScaleFinishFunction.IsBound())
		OnScaleFinishFunction.BindDynamic(this, &ThisClass::OnScaleFinish);
	if (ScaleCurve)
	{
		ScaleTimeComponent->AddInterpFloat(ScaleCurve, UpdateFunctionScale);
		ScaleTimeComponent->SetTimelineFinishedFunc(OnScaleFinishFunction);
	}

	if (!UpdateFunctionHit.IsBound())
		UpdateFunctionHit.BindDynamic(this, &ThisClass::UpdateHitTimeComponent);
	if (!OnHitFinishFunction.IsBound())
		OnHitFinishFunction.BindDynamic(this, &ThisClass::OnHitFinish);
	if (HitCurve)
	{
		HitTimeComponent->AddInterpFloat(HitCurve, UpdateFunctionHit);
		HitTimeComponent->SetTimelineFinishedFunc(OnHitFinishFunction);
	}
}

// 初始化基础信息
void ASingleFence_Base::InitBase()
{
	if (FenceMesh)
	{
		FenceMeshComponent->SetStaticMesh(FenceMesh);
		float NewRadius;
		FVector NewLocation, BoxExtent;
		UKismetSystemLibrary::GetComponentBounds(FenceMeshComponent, NewLocation, BoxExtent, NewRadius);
		Box->SetWorldLocation(NewLocation);
		Box->SetBoxExtent(BoxExtent);
		FenceMeshComponent->SetVectorParameterValueOnMaterials("CampColor", FVector(CampColor.R, CampColor.G, CampColor.B));
	}
}

// 显示
void ASingleFence_Base::ShowFence_Implementation()
{
	const bool Hidden = IsHidden();
	if (Hidden)
	{
		ScalePlay();
		SetActorHiddenInGame(false);
	}
}

// 更改颜色
void ASingleFence_Base::ChangeFenceColor_Implementation(const FLinearColor NewColor)
{
	if (CampColor == NewColor) return;

	ScalePlay();

	CampColor = NewColor;
	FenceMeshComponent->SetVectorParameterValueOnMaterials("CampColor", FVector(CampColor.R, CampColor.G, CampColor.B));
}

// 命中
void ASingleFence_Base::FenceHit_Implementation(const bool CanShake, const float Angle)
{
	if (bInHit) return;
	bCanShake = CanShake;
	ShakeAngle = Angle;
	bInHit = true;
	if (HitTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(HitTimerHandle);
	}

	GetWorld()->GetTimerManager().SetTimer(HitTimerHandle, this, &ASingleFence_Base::HitPlayer, FMath::FRandRange(0.01, 0.05));
}

// 移除
void ASingleFence_Base::RemoveFence_Implementation()
{
	Destroy();
}

// 播放缩放动画
void ASingleFence_Base::ScalePlay()
{
	if (ScaleTimeComponent)
	{
		ScaleTimeComponent->SetTimelineLength(1.f);
		ScaleTimeComponent->SetPlayRate(1.f / ScaleTime);
		ScaleTimeComponent->PlayFromStart();
	}
}

// 更新缩放动画
void ASingleFence_Base::UpdateScaleTimeComponent(const float Output)
{
	FenceMeshComponent->SetWorldScale3D(StartSize * Output);
}

// 缩放动画结束
void ASingleFence_Base::OnScaleFinish()
{
}

// 播放命中动画
void ASingleFence_Base::HitPlayer()
{
	if (HitTimeComponent)
	{
		HitTimeComponent->SetTimelineLength(1.f);
		HitTimeComponent->SetPlayRate(1.f / HitTime);
		HitTimeComponent->PlayFromStart();
	}
}

// 更新命中动画
void ASingleFence_Base::UpdateHitTimeComponent(const float Output)
{
	FenceMeshComponent->SetScalarParameterValueOnMaterials("Hit", Output);
	if (bCanShake)
	{
		FenceMeshComponent->SetRelativeRotation(FRotator(0.f, 0.f, (0.f + Output * (ShakeAngle - 0.f))));
	}
}

// 命中动画结束
void ASingleFence_Base::OnHitFinish()
{
	bInHit = false;
}
