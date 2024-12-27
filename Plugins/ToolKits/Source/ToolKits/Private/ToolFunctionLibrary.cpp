// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolFunctionLibrary.h"

// 有参构造函数
UToolFunctionLibrary::UToolFunctionLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 贝塞尔曲线
FVector UToolFunctionLibrary::BezierCurve(TArray<FVector> Points, const float CurveTime, const float TotalTime)
{
	if (Points.IsEmpty()) return FVector::ZeroVector;

	// 当输入的点长度小于等于一的时候，输出数组下标零
	if (Points.Num() <= 1)return Points[0];

	// 计算处于曲线的百分比
	const float Time = CurveTime / TotalTime;
	// 数组的数量
	const int32 Count = Points.Num() - 1;
	// 创建一个临时的数组点
	TArray<FVector> PointsDelta;
	// 现在数组内存大小
	PointsDelta.Reserve(Count);

	for (int i = 0; i < Count; i++)
	{
		const FVector& Start = Points[i];
		const FVector& End = Points[i + 1];
		PointsDelta.Add(Start + (End - Start) * Time);
	}

	return BezierCurve(PointsDelta, CurveTime, TotalTime);
}
