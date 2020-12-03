#include "pch.h"
#include "CppUnitTest.h"
#include "../SoundEnginePlugin/MonoBuffer.h"
#include "../SoundEnginePlugin/MonoBufferUtilities.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace MaggiLizerTests
{
	TEST_CLASS(MaggiLizerTests)
	{
		typedef unsigned int uint;
		const float assert_tolerance = 0.0001;

		TEST_METHOD(MBU_CopyLastWrittenBufferBlock_NoWrap)
		{
			float input[] = { 1, 3, 5, 7, 9, 2, 4, 6, 8, 0.1 };
			uint bufferSize = 10;
			uint lastBlockSize = 4;
			float expected[] = { 4, 6, 8, 0.1 };
			MonoBuffer* output = new LinearMonoBuffer(lastBlockSize);

			MonoBuffer* buffer = new CircularMonoBuffer(bufferSize);
			buffer->WriteNextBufferBlock(input, bufferSize);
			MonoBufferUtilities::CopyLastWrittenBufferBlock(buffer, output, lastBlockSize);

			for (uint i = 0; i < lastBlockSize; i++)
			{
				float bufferVal = 0;
				output->ReadNextBufferValue(bufferVal);
				Assert::AreEqual(expected[i], bufferVal);
			}
		}

		TEST_METHOD(MBU_CopyLastWrittenBufferBlock_Wrap)
		{
			float input[] = { 1, 3, 5, 7, 9, 2, 4, 6, 8, 0.1 };
			uint bufferSize = 10;
			uint lastBlockSize = 4;
			float expected[] = {8, 0.1, 2.5, 1.7 };
			MonoBuffer* output = new LinearMonoBuffer(lastBlockSize);

			MonoBuffer* buffer = new CircularMonoBuffer(bufferSize);
			buffer->WriteNextBufferBlock(input, bufferSize);
			buffer->WriteNextBufferValue(2.5);
			buffer->WriteNextBufferValue(1.7);
			MonoBufferUtilities::CopyLastWrittenBufferBlock(buffer, output, lastBlockSize);

			for (uint i = 0; i < lastBlockSize; i++)
			{
				float bufferVal = 0;
				output->ReadNextBufferValue(bufferVal);
				Assert::AreEqual(expected[i], bufferVal);
			}
		}

		TEST_METHOD(MBU_ApplySpeedBufferSingle_Double)
		{
			float input[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
			float expected[] = { 0, 2, 4, 6, 8, 0, 0, 0, 0, 0};
						
			// SETUP
			uint bufferSize = 10;
			float speed = 2.0;
			
			MonoBuffer* in_pBuffer = new LinearMonoBuffer(bufferSize);
			MonoBuffer* out_pBuffer = new CircularMonoBuffer(bufferSize * 2);

			in_pBuffer->PrefillBuffer(input, bufferSize);
			
			//TEST
			MonoBufferUtilities::ApplySpeedBufferSingle(in_pBuffer, out_pBuffer, speed);
			
			//VALIDATE
			for (uint i = 0; i < bufferSize; i++)
			{
				float outVal = 0;
				out_pBuffer->ReadNextBufferValue(outVal);

				Assert::AreEqual(expected[i], outVal);
			}
		}
			
		TEST_METHOD(MBU_ApplySpeedBufferSingle_Half)
		{
			float input[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
			float expected[] = { 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5 };
			
			// SETUP
			uint bufferSize = 10;
			float speed = 0.5;
			
			MonoBuffer* in_pBuffer = new LinearMonoBuffer(bufferSize);
			MonoBuffer* out_pBuffer = new CircularMonoBuffer(bufferSize * 2);

			in_pBuffer->PrefillBuffer(input, bufferSize);

			//TEST
			MonoBufferUtilities::ApplySpeedBufferSingle(in_pBuffer, out_pBuffer, speed);
			
			//VALIDATE
			for (uint i = 0; i < bufferSize; i++)
			{
				float outVal = 0;
				out_pBuffer->ReadNextBufferValue(outVal);

				Assert::AreEqual(expected[i], outVal);
			}
		}

		TEST_METHOD(MBU_ApplyReverseBufferSingle)
		{
			float input[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
			float expected[] = { 8, 7, 6, 5, 4, 3, 2, 1, 0};
			
			//SETUP
			uint bufferSize = 9;
			bool reverse = true;
			
			MonoBuffer* buffer = new LinearMonoBuffer(bufferSize);
			buffer->PrefillBuffer(input, bufferSize);

			//TEST
			MonoBufferUtilities::ApplyReverseBufferSingle(buffer);
						
			//VALIDATE
			for (uint i = 0; i < bufferSize; i++)
			{
				float outVal = 0;
				buffer->ReadNextBufferValue(outVal);

				Assert::AreEqual(expected[i], outVal);
			}
		}

		TEST_METHOD(MBU_ApplySmoothingAtIndex)
		{
			float input[] = { -1, 0, 0, 0, 0.5, 0.5, 0, 0, 0, 0 };
			float expected[] = { -1, 0, 0, 0, 0.5, 0.5, 0.300000012, 0.199999988, 0.0999999940, 0 };
			float startValue = 4;
			uint bufferSize = 10;
			uint smoothWindow = 5;
			
			MonoBuffer* buffer = new CircularMonoBuffer(bufferSize);
			buffer->PrefillBuffer(input, bufferSize);
			
			//TEST
			MonoBufferUtilities::ApplySmoothingAtIndex(buffer, startValue, smoothWindow);
			
			//VALIDATE
			for (uint i = 0; i < bufferSize; i++)
			{
				float outVal = 0;
				buffer->ReadNextBufferValue(outVal);
				Assert::AreEqual(expected[i], outVal);
			}
		}
	};
}