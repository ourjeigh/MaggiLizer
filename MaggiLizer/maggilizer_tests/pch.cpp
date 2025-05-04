//
// pch.cpp
//

#include "pch.h"

void FillBufferWithRandomData(float* pBuffer, unsigned long uSize)
{
	for (unsigned long i = 0; i < uSize; i++)
	{
		// -1.0 to 1.0
		pBuffer[i] = -1.0f + 2 * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	}
}