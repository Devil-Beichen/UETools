#pragma once
#include "Containers/Array.h"

// 反转数组
template <typename T>
void ReverseTArray(TArray<T>& Array)
{
	// 获取数组的长度
	int32 Length = Array.Num();

	if (Length == 0) return;

	// 循环遍历数组的前半部分，交换元素
	for (int32 i = 0; i < Length / 2; ++i)
	{
		// 临时变量存储当前元素
		T Temp = Array[i];

		// 交换元素
		Array[i] = Array[Length - 1 - i];
		Array[Length - 1 - i] = Temp;
	}
}
