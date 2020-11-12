#include "pch.h"
#include "CppUnitTest.h"
#include "../SoundEnginePlugin/MaggiLizerFX.h"
#include <array>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace MaggiLizerTests
{
	
	TEST_CLASS(MaggiLizerTests)
	{
		typedef unsigned int uint;
		typedef float** buffer;
		typedef float* buffer_single;
		
		const float assert_tolerance = 0.0001;

	public:

		/// Helpers
		// 
		void FillBufferSingleWithData(buffer_single pBuffer, const float pattern[], uint patternSize, uint repeats)
		{
			pBuffer = new float[(long long) patternSize * (long long) repeats];

			for (uint i = 0; i < repeats; i++)
			{
				for (uint j = 0; j < patternSize; j++)
				{
					uint pos = patternSize * i + j;
					pBuffer[pos] = pattern[j];
				}
			}
		}

		void FillBufferWithData(buffer pBuffer, uint channels, const float pattern[], uint patternSize, uint repeats)
		{
			pBuffer = new buffer_single[channels];

			for (uint i = 0; i < channels; i++)
			{
				FillBufferSingleWithData(pBuffer[i], pattern, patternSize, repeats);
			}
		}

		void AssertBufferSingleValues(const float expected[], buffer_single actual, uint size)
		{
			for (uint i = 0; i < size; i++)
			{
				Assert::AreEqual(expected[i], actual[i], assert_tolerance);
			}
		}
		//
		/// \Helpers

		/// Helper Tests
		// 
		TEST_METHOD(Helper_FillBufferSingle)
		{
			float expected[9] = { 0,1,2,0,1,2,0,1,2 };

			buffer_single pBuffer;
			FillBufferSingleWithData(pBuffer, new float[] { 0, 1, 2 }, 3, 3);

			AssertBufferSingleValues(expected, pBuffer, 9);
		}

		TEST_METHOD(Helper_FillBuffer)
		{
			const float pattern[] = { 0,1,2,0,1,2,0,1,2 };
			const uint patternSize = 9;
			const uint channels = 2;

			float expected[channels][patternSize] = { { 0,1,2,0,1,2,0,1,2 } , { 0,1,2,0,1,2,0,1,2 } };

			buffer pBuffer;
			FillBufferWithData(pBuffer, channels, pattern, patternSize, 1);

			for (uint i = 0; i < channels; i++)
			{
				AssertBufferSingleValues(pattern, pBuffer[i], patternSize);
			}
		}
		//
		/// \Helper Tests

		/// Method Tests
		//

		
		TEST_METHOD(Method_SetBufferValue)
		{
			Assert::Fail();
		}

		TEST_METHOD(Method_ClearBufferSingle)
		{
			Assert::Fail();
		}

		TEST_METHOD(Method_SwapBufferValues)
		{
			Assert::Fail();
		}

		TEST_METHOD(Method_ReverseBuffer)
		{
			Assert::Fail();
		}

		TEST_METHOD(Method_ApplyReverseBufferSingle)
		{
			Assert::Fail();
		}

		TEST_METHOD(Method_ApplySpeedBufferSingle)
		{
			Assert::Fail();
		}

		TEST_METHOD(Method_CaclulateBufferSampleSize)
		{
			Assert::Fail();
		}

		TEST_METHOD(Method_CalculateBufferSizeChangeFromSpeed)
		{
			Assert::Fail();
		}

		TEST_METHOD(Method_GetBufferValue)
		{
			Assert::Fail();
		}

		TEST_METHOD(Method_CalculateWetDryMix)
		{
			Assert::Fail();
		}

		TEST_METHOD(Method_CalculateSpeed)
		{
			Assert::Fail();
		}


		/// <summary>
		/// DESCRIPTION:
		/// When the plugin first starts receiving samples, it has not yet filled it's playback buffer.
		/// While it is filling it's uinternal buffer for playback, it outputs silence (0).
		/// </summary>
		TEST_METHOD(SetsInitialBufferToZero)
		{
			float expected[] = { 0,0,0,0,0,0,0,0,0,0 };

			uint sampleRate = 48000;
			uint channels = 1;
			uint bufferBlockSize = 10;
			bool reverse = false;
			float pitch = 0;
			float splice = 500;//ms
			float delay = 0;
			float recycle = 0;
			float mix = 1;

			buffer pBuffer = new float* [1];
			FillBufferWithData(pBuffer, channels, new float[] {0.1}, 1, bufferBlockSize);

			MaggiLizerFXDSP* p_DSP = new MaggiLizerFXDSP();
			p_DSP->Init(sampleRate, splice, channels);

			p_DSP->Execute(pBuffer, channels, bufferBlockSize, recycle, pitch, splice, delay, recycle, mix);
			
			AssertBufferSingleValues(expected, pBuffer[0], bufferBlockSize);
		}


		/// <summary>
		/// Description:
		/// If Mix value is set to 0, input buffer should match output buffer
		/// </summary>
		TEST_METHOD(MixOfZeroDoesNotModifyBuffer)
		{
			float expected[] = { 1,2,3,1,2,3,1,2,3,1,2,3,1,2,3 };

			uint sampleRate = 48000;
			uint channels = 1;
			uint bufferBlockSize = 15;
			bool reverse = false;
			float pitch = 0;
			float splice = 500;//ms
			float delay = 0;
			float recycle = 0;
			float mix = 0;

			buffer pBuffer = new float* [1];
			pBuffer[0] = new float[bufferBlockSize];
			FillBufferSingleWithData(pBuffer[0], new float[] {1, 2, 3}, 3, 5);
			
			MaggiLizerFXDSP* p_DSP = new MaggiLizerFXDSP();
			p_DSP->Init(sampleRate, splice, channels);

			p_DSP->Execute(pBuffer, channels, bufferBlockSize, recycle, pitch, splice, delay, recycle, mix);

			AssertBufferSingleValues(expected, pBuffer[0], bufferBlockSize);
		}

		//TEST_METHOD(Reverse)
		//{
		//	float expected[] = { 0.5,0.4,0.3,0.2,0.1};

		//	uint sampleRate = 5;
		//	uint channels = 1;
		//	uint frames = 5;
		//	bool reverse = true;
		//	float pitch = 0;
		//	float splice = 1000;//ms
		//	float delay = 0;
		//	float recycle = 0;
		//	float mix = 1;

		//	float** pBuffer = new float* [1];
		//	pBuffer[0] = new float[frames];

		//	MaggiLizerFXDSP* p_DSP = new MaggiLizerFXDSP();
		//	p_DSP->Init(sampleRate, splice, channels);

		//	FillBufferWithData(pBuffer[0], new float[] {0.1, 0.2, 0.3, 0.4, 0.5}, 5, 1); 
		//	p_DSP->Execute(pBuffer, channels, frames, recycle, pitch, splice, delay, recycle, mix);
		//	FillBufferWithData(pBuffer[0], new float[] {0.1, 0.2, 0.3, 0.4, 0.5}, 5, 1);
		//	p_DSP->Execute(pBuffer, channels, frames, recycle, pitch, splice, delay, recycle, mix);
		//	FillBufferWithData(pBuffer[0], new float[] {0.1, 0.2, 0.3, 0.4, 0.5}, 5, 1);
		//	p_DSP->Execute(pBuffer, channels, frames, recycle, pitch, splice, delay, recycle, mix);

		//	AssertBufferValues(expected, pBuffer[0], frames);
		//}

		//
		/// \Method Tests
	};
}
