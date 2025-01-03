// Fill out your copyright notice in the Description page of Project Settings.


#include "HelicalFence.h"

#include "FenceSpline.h"
#include "SingleFence_Base.h"
#include "YCTArray.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"


AHelicalFence::AHelicalFence():
	Around(true)
{
	// 开启Tick
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(RootComponent);
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(RootComponent);
}


// 获取间隔
float AHelicalFence::GetPointInterval() const
{
	if (Around)
	{
		if (Clockwise) return PointInterval * -1.f;
		else return PointInterval;
	}
	else
	{
		if (Clockwise) return PointInterval;
		else return PointInterval * -1.f;
	}
}

// 获取中心距离
float AHelicalFence::GetCentreDistance() const
{
	if (Around)
	{
		if (Clockwise) return CentreDistance * -1.f;
		else return CentreDistance;
	}
	else
	{
		if (Clockwise) return CentreDistance * -1.f;
		else return CentreDistance;
	}
}

void AHelicalFence::BeginPlay()
{
	Super::BeginPlay();
	GeneratingFences();
}

void AHelicalFence::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (FenceSpline && !FenceSpline->DisplayModels.IsEmpty() && FenceSpline->SingleFenceClass)
	{
		DisplayModels = FenceSpline->DisplayModels;
		DisplayNum = FenceSpline->DisplayNum;
		SingleFenceClass = FenceSpline->SingleFenceClass;
	}
	AddDisplayModel();
}

// 初始化实例组件
void AHelicalFence::InitializeComponent(TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Component, TObjectPtr<UStaticMesh> NewStaticMesh)
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
}

void AHelicalFence::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetSplineLocation();
	if (bStart)
	{
		FenceHidden();
	}
	else
	{
		if (!AllSingleFences.IsEmpty())
		{
			for (auto& SingleFences : AllSingleFences)
			{
				if (SingleFences)
				{
					SingleFences->SetActorHiddenInGame(IsHidden()); // 隐藏
				}
			}
		}
	}
}

/**
 * 根据索引获取围栏组件的网格长度
 * 
 * @param Index 围栏组件的索引
 * @return 返回围栏组件的网格长度，如果指定的围栏组件不存在，则返回零向量
 */
FVector AHelicalFence::GetMeshLength(int32 Index)
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

TArray<FTransform> AHelicalFence::GetTempTransforms()
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
			FTransform ATransforms = Spline->GetTransformAtDistanceAlongSpline((Spline->GetSplineLength() - CurrentDistance), ESplineCoordinateSpace::World, true);
			// 设置缩放
			ATransforms.SetScale3D(ATransforms.GetScale3D() * Size);
			// 旋转
			FRotator TempRotation = ATransforms.Rotator() + FRotator(0.f, 180.f, 0.f);
			// 设置旋转
			ATransforms.SetRotation(TempRotation.Quaternion());
			// 添加到数组
			TempTransforms.Add(ATransforms);
		}
	}, TStatId(), nullptr, ENamedThreads::Type::AnyThread);
	// 等待任务完成
	FTaskGraphInterface::Get().WaitUntilTaskCompletes(TransformsTask);

	// 返回临时坐标数组
	return TempTransforms;
}

// 设置样条线位置
void AHelicalFence::SetSplineLocation()
{
	float NewTime = 1 + Progress * (0 - 1);
	FVector NewTimeLocation = Spline->GetLocationAtTime(NewTime, ESplineCoordinateSpace::Local, true);
	float NewLocationX = FMath::Sqrt(FMath::Square(NewTimeLocation.X) + FMath::Square(NewTimeLocation.Y)) * GeAround();
	float NewRotationYay = (180.0) / UE_DOUBLE_PI * FMath::Atan2(NewTimeLocation.Y, NewTimeLocation.X) * -1.f + (Around ? 180.f : 0.f);
	Spline->SetRelativeLocationAndRotation(FVector(NewLocationX, 0.f, 0.f), FRotator(0.f, NewRotationYay, 0.f));
}

// 添加显示模型
void AHelicalFence::AddDisplayModel()
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
	if (DisplayModels.IsEmpty() || DisplayNum <= 0 || Spline == nullptr) return;

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

	// 创建一个处理样条线的异步任务
	FGraphEventRef SplineTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this,ModelNum]()
	{
		float SplineLength = 0.f; // 样条线长度
		int32 Index = 0; // 索引
		Points.Empty(); // 清空点数组
		Spline->ClearSplinePoints(true); // 清除样条线点
		// 计算样条线长度
		for (int32 i = 0; i <= DisplayNum - 1; ++i)
		{
			int32 FenceNum = i % ModelNum == 0 ? (ModelNum - 1) : ((i % ModelNum) - 1);
			SplineLength += i == 0 ? GetMeshLength(0).X : GetMeshLength(FenceNum).X;
		}
		// 设置实际长度
		ActualLength = SplineLength;

		// 循环添加点，直到总长度大于等于实际长度
		while (Spline->GetSplineLength() < ActualLength)
		{
			float Angle = GetPointInterval() * Index * GeAround();
			float TempLength = Angle * HelicalInterval + GetCentreDistance();
			float X = TempLength * FMath::Cos(UE_DOUBLE_PI / (180.f) * Angle);
			float Y = TempLength * FMath::Sin(UE_DOUBLE_PI / (180.f) * Angle);
			Points.Add(FVector(X, Y, 0.f));
			Spline->SetSplinePoints(Points, ESplineCoordinateSpace::Local, true);
			Index++;
		}
	}, TStatId(), nullptr, ENamedThreads::Type::AnyThread);
	// 等待任务完成
	FTaskGraphInterface::Get().WaitUntilTaskCompletes(SplineTask);

	// 创建一个处理样条线的异步任务
	FGraphEventRef SolineSetPoint = FFunctionGraphTask::CreateAndDispatchWhenReady([this]()
	{
		SetSplineLocation();
	}, TStatId(), nullptr, ENamedThreads::Type::AnyThread);
	// 等待任务完成
	FTaskGraphInterface::Get().WaitUntilTaskCompletes(SolineSetPoint);

	// 添加实例
	FGraphEventRef AddMeshTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this,ModelNum]()
	{
		// 获取临时变换数组
		TArray<FTransform> TempFTransforms = GetTempTransforms();

		// 临时变量，用于记录当前实例化的模型编号
		int TempNum = 0;
		// 遍历临时变换数组，为每个变换添加实例
		for (auto& StaticMeshTransform : TempFTransforms)
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
void AHelicalFence::GeneratingFences()
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
		for (auto& SpawnTransform : TempTransforms)
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
		for (auto& StaticMeshComponent : InstancedStaticMeshComponents)
		{
			// 添加空指针检查,清除实例并清空数组
			if (StaticMeshComponent != nullptr)
				StaticMeshComponent->ClearInstances();
		}
		InstancedStaticMeshComponents.Empty();
	}
}

// 隐藏围栏
void AHelicalFence::FenceHidden()
{
	if (!FenceSpline) return;
	if (FenceSpline->AllSingleFences.IsEmpty()) return;

	// 创建一个异步任务，用于隐藏围栏对象
	FGraphEventRef HiddenTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this]()
	{
		float Distance = 0.f;
		int32 Index = 0;

		for (auto& SingleFences : AllSingleFences)
		{
			if (SingleFences && SingleFences->GetFenceMesh())
			{
				// 计算当前围栏对象与下一个围栏对象之间的距离
				float CurrentDistance = Interval + Distance + SingleFences->GetFenceMesh()->GetBounds().BoxExtent.X * 2.f * Size;
				bool Hidden = CurrentDistance > (Spline->GetSplineLength() * (1.f - Progress));
				SingleFences->SetActorHiddenInGame(Hidden);
				if (FenceSpline->AllSingleFences[Index])
				{
					FenceSpline->AllSingleFences[Index]->SetActorHiddenInGame(!Hidden);
				}

				Distance = CurrentDistance;
				Index++;
			}
		}
	}, TStatId(), nullptr, ENamedThreads::Type::AnyThread);
	// 等待任务完成
	FTaskGraphInterface::Get().WaitUntilTaskCompletes(HiddenTask);
}
