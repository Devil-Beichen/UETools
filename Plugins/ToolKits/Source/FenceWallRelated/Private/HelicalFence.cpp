// Fill out your copyright notice in the Description page of Project Settings.


#include "HelicalFence.h"

#include "SingleFence_Base.h"
#include "YCTArray.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"


AHelicalFence::AHelicalFence():
	Around(true)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(RootComponent);
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(RootComponent);
}


void AHelicalFence::BeginPlay()
{
	Super::BeginPlay();
}

void AHelicalFence::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	AddDisplayModel();
}

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

void AHelicalFence::UpdateComponent(TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Component, TObjectPtr<UStaticMesh> NewStaticMesh)
{
	// 为网格组件设置静态网格，即定义其外观
	Component->SetStaticMesh(NewStaticMesh);
	// 设置网格组件的材质参数，此处为营地颜色
	Component->SetVectorParameterValueOnMaterials("CampColor", FVector(CampColor.R, CampColor.G, CampColor.B));
}


void AHelicalFence::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	if (DisplayModel[Index])
	{
		// 如果存在，计算并返回网格长度
		return DisplayModel[Index]->GetBounds().BoxExtent * 2.f * Size;
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
	if (DisplayModel.Num() <= 0 || DisplayNum <= 0) return TArray<FTransform>();
	// 获取曲线
	if (!Spline) return TArray<FTransform>();
	// 模型数量
	int32 ModelNum = DisplayModel.Num();
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
	// 如果显示模型数组为空或显示数量小于等于0，则直接返回
	if (DisplayModel.IsEmpty() || DisplayNum <= 0 || Spline == nullptr) return;

	// 模型数量
	int32 ModelNum = DisplayModel.Num();

	// 清空实例静态网格组件数组
	if (InstancedStaticMeshComponents.Num() != ModelNum)
	{
		for (auto* StaticMeshComponent : InstancedStaticMeshComponents)
		{
			// 添加空指针检查,清除实例并清空数组
			if (StaticMeshComponent != nullptr)
			{
				StaticMeshComponent->ClearInstances();
				StaticMeshComponent->UnregisterComponent();
				StaticMeshComponent->DestroyComponent();
			}
		}
		InstancedStaticMeshComponents.Empty();
	}

	// 遍历模型数组
	for (int32 i = 0; i < ModelNum; i++)
	{
		if (DisplayModel[i] != nullptr)
		{
			if (InstancedStaticMeshComponents.IsEmpty())
			{
				// 实例化网格组件
				UHierarchicalInstancedStaticMeshComponent* HISMComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
				if (HISMComponent)
				{
					InitializeComponent(HISMComponent, DisplayModel[i]);
				}
			}
			else if (InstancedStaticMeshComponents[i] != nullptr)
			{
				UpdateComponent(InstancedStaticMeshComponents[i], DisplayModel[i]);
			}
		}
	}

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
}

// 生成围栏
void AHelicalFence::GeneratingFences()
{
	if (DisplayModel.IsEmpty()) return;
	TArray<FTransform> TempTransforms = GetTempTransforms();
	UWorld* World = GetWorld();
	if (TempTransforms.IsEmpty() || World == nullptr) return;
	// 模型数量
	int32 ModelNum = DisplayModel.Num();
	int index = 0;
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 遍历临时变换数组，用于在特定位置生成单个围栏对象
	for (auto SpawnTransform : TempTransforms)
	{
		// 如果单个围栏类为空，则跳出循环，防止无效操作
		if (SingleFenceClass == nullptr) break;

		// 在指定的位置和参数下生成单个围栏对象
		if (ASingleFence_Base* SingleFence_Base = World->SpawnActor<ASingleFence_Base>(SingleFenceClass, SpawnTransform, SpawnParameters))
		{
			// 将生成的围栏对象附加到当前对象上，保持其在世界中的变换
			SingleFence_Base->AttachToComponent(Spline, FAttachmentTransformRules::KeepWorldTransform);

			// 设置围栏对象的显示模型，根据索引选择合适的模型
			SingleFence_Base->SetFenceMesh(DisplayModel[index % ModelNum]);

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
