// Fill out your copyright notice in the Description page of Project Settings.


#include "FenceSpline.h"

#include "SingleFence_Base.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "YCTArray.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

// Sets default values
AFenceSpline::AFenceSpline(): bDefaultDisplay(true)
{
	// 关闭Tick
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(RootComponent);

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(RootComponent);
}

void AFenceSpline::BeginPlay()
{
	Super::BeginPlay();
	GeneratingFences();
}

void AFenceSpline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	AddDisplayModel();
}

// 初始化网格组件
void AFenceSpline::InitializeComponent(TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Component, TObjectPtr<UStaticMesh> NewStaticMesh)
{
	// 注册网格组件
	Component->RegisterComponent();
	// 设置网格组件的附着关系，使其附着到根组件
	Component->AttachToComponent(Spline, FAttachmentTransformRules::KeepWorldTransform);
	// 禁用碰撞，因为此组件不需要与其他物体发生物理碰撞
	Component->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	// 在游戏中显示网格组件
	Component->SetHiddenInGame(false);
	// 设置网格组件不接收贴花，简化渲染处理
	Component->bReceivesDecals = false;
	// 为网格组件设置静态网格，即定义其外观
	Component->SetStaticMesh(NewStaticMesh);
	// 设置网格组件的材质参数，此处为营地颜色
	Component->SetVectorParameterValueOnMaterials("CampColor", FVector(CampColor.R, CampColor.G, CampColor.B));

	// 像用CreateDefaultSubobject一样显示网格组件
}

/**
 * 根据索引获取围栏组件的网格长度
 * 
 * @param Index 围栏组件的索引
 * @return 返回围栏组件的网格长度，如果指定的围栏组件不存在，则返回零向量
 */
FVector AFenceSpline::GetMeshLength(int32 Index)
{
	// 检查索引对应的围栏组件是否存在
	if (DisplayModels[Index])
	{
		// 如果存在，计算并返回网格长度
		return DisplayModels[Index]->GetBounds().BoxExtent * 2.f * Size;
	}
	else
	{
		// 如果不存在，返回零向量
		return FVector();
	}
}

// 获取临时变换数组
TArray<FTransform> AFenceSpline::GetTempTransforms()
{
	// 模型数量为0 或者 显示数量为0
	if (DisplayModels.Num() <= 0 || DisplayNum <= 0) return TArray<FTransform>();
	// 获取曲线
	if (!Spline) return TArray<FTransform>();
	// 模型数量
	int32 ModelNum = DisplayModels.Num();
	// 临时坐标数组
	TArray<FTransform> TempTransforms = TArray<FTransform>();
	// 当前所在的临时距离
	float CurrentDistance = 0.f;
	// 缓存模型长度
	TArray<float> ModelLengths;
	for (int32 i = 0; i < ModelNum; ++i)
	{
		ModelLengths.Add(GetMeshLength(i).X);
	}

	// 创建一个异步任务，处理坐标数组
	FGraphEventRef TransformsTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this,ModelNum,ModelLengths,&CurrentDistance,&TempTransforms]()
	{
		// 根据显示数量生成临时坐标
		for (int i = 0; i <= DisplayNum - 1; i++)
		{
			// 余数
			int32 TempIndex = i % ModelNum;
			// 实际编号
			int32 ActualNumber = TempIndex == 0 ? ModelNum - 1 : TempIndex - 1;
			// 实际间隔
			float ActualInterval = i == 0 ? 0.f : Interval;
			// 当前模型长度
			float IntervalModelLength = i == 0 ? 0.f : ModelLengths[ActualNumber];
			// 当前距离
			CurrentDistance += ActualInterval + IntervalModelLength;
			// 临时坐标
			FTransform ATransforms = Spline->GetTransformAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World, true);
			// 设置缩放
			ATransforms.SetScale3D(ATransforms.GetScale3D() * Size);
			// 添加到数组
			TempTransforms.Add(ATransforms);
		}
	}, TStatId(), nullptr, ENamedThreads::Type::AnyThread);
	// 等待任务完成
	FTaskGraphInterface::Get().WaitUntilTaskCompletes(TransformsTask);

	// 返回临时坐标数组
	return TempTransforms;
}

// 向围栏样条添加显示模型
// 本函数负责将DisplayModel数组中的模型添加到InstancedStaticMeshComponents中，并根据临时变换数组生成实例
void AFenceSpline::AddDisplayModel()
{
	// 清空实例数组
	if (!InstancedStaticMeshComponents.IsEmpty())
	{
		for (auto& StaticMeshComponent : InstancedStaticMeshComponents)
		{
			// 添加空指针检查,清除实例并清空数组
			if (StaticMeshComponent != nullptr)
			{
				// 清除实例
				StaticMeshComponent->ClearInstances();
				if (StaticMeshComponent->IsRegistered()) // 检查是否已注册
				{
					StaticMeshComponent->UnregisterComponent(); // 卸载组件
				}
				StaticMeshComponent->DestroyComponent();
			}
		}
		InstancedStaticMeshComponents.Empty();
	}

	// 如果显示模型数组为空或显示数量小于等于0，则直接返回
	if (DisplayModels.IsEmpty() || DisplayNum <= 0)return;

	// 模型数量
	int32 ModelNum = DisplayModels.Num();

	// 预分配实例数组
	InstancedStaticMeshComponents.Reserve(ModelNum);

	// 创建一个异步任务，处理实例静态网格组件
	FGraphEventRef ComponentTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this,ModelNum]()
	{
		// 遍历显示模型数组，为每个模型创建一个层级实例静态网格组件
		for (int32 i = 0; i < ModelNum; i++)
		{
			// 确保模型指针不为空
			if (DisplayModels[i] != nullptr)
			{
				FString ComponentName = FString::Printf(TEXT("HISMComponent_%d"), i);
				// 创建层级实例静态网格组件
				UHierarchicalInstancedStaticMeshComponent* HISMComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, UHierarchicalInstancedStaticMeshComponent::StaticClass(), *ComponentName);
				if (HISMComponent)
				{
					InitializeComponent(HISMComponent, DisplayModels[i]);
					InstancedStaticMeshComponents.Add(HISMComponent);
				}
			}
		}
	}, TStatId(), nullptr, ENamedThreads::Type::GameThread);
	// 等待任务完成
	FTaskGraphInterface::Get().WaitUntilTaskCompletes(ComponentTask);

	FGraphEventRef AddMeshTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this,ModelNum]()
	{
		// 获取临时变换数组
		TArray<FTransform> TempFTransforms = GetTempTransforms();

		// 临时变量，用于记录当前实例化的模型编号
		int TempNum = 0;
		// 遍历临时变换数组，为每个变换添加实例
		for (auto StaticMeshTransform : TempFTransforms)
		{
			// 确保数组索引有效
			if (InstancedStaticMeshComponents.IsValidIndex(TempNum % ModelNum))
			{
				InstancedStaticMeshComponents[TempNum % ModelNum]->AddInstance(StaticMeshTransform, true);
				TempNum++;
			}
		}
	}, TStatId(), nullptr, ENamedThreads::Type::GameThread);
	FTaskGraphInterface::Get().WaitUntilTaskCompletes(AddMeshTask);
}

// 生成围栏
void AFenceSpline::GeneratingFences()
{
	if (DisplayModels.IsEmpty() || SingleFenceClass == nullptr) return;
	TArray<FTransform> TempTransforms = GetTempTransforms();
	UWorld* World = GetWorld();
	if (TempTransforms.IsEmpty() || World == nullptr) return;

	// 创建一个异步任务，用于生成围栏对象
	FGraphEventRef SpawnTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this,World,&TempTransforms]()
	{
		// 模型数量
		int32 ModelNum = DisplayModels.Num();
		int index = 0;
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 遍历临时变换数组，用于在特定位置生成单个围栏对象
		for (auto SpawnTransform : TempTransforms)
		{
			// 在指定的位置和参数下生成单个围栏对象
			if (ASingleFence_Base* SingleFence_Base = World->SpawnActor<ASingleFence_Base>(SingleFenceClass, SpawnTransform, SpawnParameters))
			{
				// 将生成的围栏对象附加到当前对象上，保持其在世界中的变换
				SingleFence_Base->AttachToComponent(Spline, FAttachmentTransformRules::KeepWorldTransform);

				// 设置围栏对象的显示模型，根据索引选择合适的模型
				SingleFence_Base->SetFenceMesh(DisplayModels[index % ModelNum]);

				// 设置围栏对象的阵营颜色，使其与当前对象一致
				SingleFence_Base->SetCampColor(CampColor);

				// 初始化围栏对象的基础属性
				SingleFence_Base->InitBase();

				// 设置围栏对象是否可见，根据默认显示设置
				SingleFence_Base->SetActorHiddenInGame(!bDefaultDisplay);

				// 将围栏对象添加到列表中，便于后续管理
				AllSingleFences.AddUnique(SingleFence_Base);

				// 增加索引，用于选择下一个模型或颜色等
				index++;
			}
		}
	}, TStatId(), nullptr, ENamedThreads::Type::GameThread);
	// 等待任务完成
	FTaskGraphInterface::Get().WaitUntilTaskCompletes(SpawnTask);

	// 反转数组
	ReverseTArray(AllSingleFences);

	// 如果实例静态网格组件数组不为空，则清除所有实例并清空数组
	if (!InstancedStaticMeshComponents.IsEmpty())
	{
		for (auto* StaticMeshComponent : InstancedStaticMeshComponents)
		{
			// 添加空指针检查,清除实例并清空数组
			if (StaticMeshComponent != nullptr)
				StaticMeshComponent->ClearInstances();
		}
		InstancedStaticMeshComponents.Empty();
	}
}
