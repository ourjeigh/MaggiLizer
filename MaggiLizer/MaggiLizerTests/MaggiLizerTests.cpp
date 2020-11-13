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
		buffer_single FillBufferSingleWithData(const float pattern[], uint patternSize, uint repeats = 1)
		{
			buffer_single pBuffer = new float[(long long) patternSize * (long long)repeats];

			for (uint i = 0; i < repeats; i++)
			{
				for (uint j = 0; j < patternSize; j++)
				{
					uint pos = patternSize * i + j;
					pBuffer[pos] = pattern[j];
					pBuffer[pos] = pBuffer[pos];

				}
			}
			return pBuffer;
		}

		buffer FillBufferWithData(uint channels, const float pattern[], uint patternSize, uint repeats = 1)
		{
			buffer pBuffer = new buffer_single[channels];
			
			for (uint i = 0; i < channels; i++)
			{
				pBuffer[i] = FillBufferSingleWithData(pattern, patternSize, repeats);
				//FillBufferSingleWithData(pBuffer[i], pattern, patternSize, repeats);
			}
			return pBuffer;
		}

		void AssertBufferSingleValues(const float expected[], buffer_single actual, uint size)
		{
			for (uint i = 0; i < size; i++)
			{
				Assert::AreEqual(expected[i], actual[i], assert_tolerance);
			}
		}

		void AssertBufferSingleValues(buffer_single expected, buffer_single actual, uint size)
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

			buffer_single pBuffer = FillBufferSingleWithData(expected, 9);

			AssertBufferSingleValues(expected, pBuffer, 9);
		}

		TEST_METHOD(Helper_FillBuffer)
		{
			const float pattern[] = { 0,1,2,0,1,2,0,1,2 };
			const uint patternSize = 9;
			const uint channels = 2;

			float expected[channels][patternSize] = { { 0,1,2,0,1,2,0,1,2 } , { 0,1,2,0,1,2,0,1,2 } };

			buffer pBuffer = FillBufferWithData(channels, pattern, 9);
			
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
			MaggiLizerFXDSP* dsp = new MaggiLizerFXDSP;

			uint channels = 2;

			buffer pBuffer = FillBufferWithData(channels, new float[] {0}, 1, 10);

			uint channelIndex = 1;
			uint bufferSamplePosition = 7;
			float value = 0.021;

			dsp->SetBufferValue(pBuffer, channelIndex, bufferSamplePosition, value);
		
			Assert::AreEqual(pBuffer[channelIndex][bufferSamplePosition], value);
		}

		TEST_METHOD(Method_ClearBufferSingle)
		{

			float expected[] = { 0,0,0,0,0,0,0,0,0,0 };
			
			// SETUP
			uint bufferSize = 10;

			MaggiLizerFXDSP* dsp = new MaggiLizerFXDSP;

			buffer_single pBuffer = FillBufferSingleWithData(new float[] {0.1}, 1, bufferSize);

			// TEST
			dsp->ClearBufferSingle(pBuffer, bufferSize);

			// VALIDATE
			AssertBufferSingleValues(expected, pBuffer, bufferSize);
		}

		TEST_METHOD(Method_SwapBufferValues_EvenSizeBuffer)
		{
			float input[] = {0, 1, 2, 3, 4, 5, 6, 7};
			float expected[]{ 0, 1, 5, 3, 4, 2, 6, 7 };

			//SETUP
			uint leftPos = 2;
			uint rightPos = 5;
			uint bufferSize = 8;

			MaggiLizerFXDSP* dsp = new MaggiLizerFXDSP;

			buffer_single pBuffer = FillBufferSingleWithData(input, bufferSize);

			//TEST
			dsp->SwapBufferValues(&pBuffer[leftPos], &pBuffer[rightPos]);

			//VALIDATE
			AssertBufferSingleValues(expected, pBuffer, bufferSize);
		}

		TEST_METHOD(Method_SwapBufferValues_OddSizeBuffer)
		{
			float input[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
			float expected[]{ 0, 1, 6, 3, 4, 5, 2, 7, 8 };

			//SETUP
			uint leftPos = 2;
			uint rightPos = 6;
			uint bufferSize = 9;

			MaggiLizerFXDSP* dsp = new MaggiLizerFXDSP;

			buffer_single pBuffer = FillBufferSingleWithData(input, bufferSize);

			//TEST
			dsp->SwapBufferValues(&pBuffer[leftPos], &pBuffer[rightPos]);

			//VALIDATE
			AssertBufferSingleValues(expected, pBuffer, bufferSize);
		}

		TEST_METHOD(Method_ApplyReverseBufferSingle)
		{
			float input[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
			float expected[] = { 8, 7, 6, 5, 4, 3, 2, 1, 0};

			//SETUP
			uint bufferSize = 9;
			bool reverse = true;

			MaggiLizerFXDSP* dsp = new MaggiLizerFXDSP;

			buffer_single pBuffer = FillBufferSingleWithData(input, bufferSize);
			
			//TEST
			dsp->ApplyReverseBufferSingle(pBuffer, bufferSize, reverse);

			//VALIDATE
			AssertBufferSingleValues(expected, pBuffer, bufferSize);
		}

		TEST_METHOD(Method_ApplySpeedBufferSingle_Double)
		{
			float input[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
			float expected[] = { 0, 2, 4, 6, 8, 0, 0, 0, 0, 0};
			
			// SETUP
			uint bufferSize = 10;
			float speed = 2.0;

			MaggiLizerFXDSP* dsp = new MaggiLizerFXDSP;

			buffer_single in_pBuffer = FillBufferSingleWithData(input, bufferSize);
			buffer_single out_pBuffer = FillBufferSingleWithData(new float{ 0 }, 1, bufferSize);

			//TEST
			dsp->ApplySpeedBufferSingle(in_pBuffer, out_pBuffer, bufferSize, speed);

			//VALIDATE
			AssertBufferSingleValues(expected, out_pBuffer, bufferSize);
		}

		TEST_METHOD(Method_ApplySpeedBufferSingle_Half)
		{
			float input[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
			float expected[] = { 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5 };

			// SETUP
			uint bufferSize = 10;
			float speed = 0.5;

			MaggiLizerFXDSP* dsp = new MaggiLizerFXDSP;

			buffer_single in_pBuffer = FillBufferSingleWithData(input, bufferSize);
			buffer_single out_pBuffer = FillBufferSingleWithData(new float{ 0 }, 1, bufferSize);

			//TEST
			dsp->ApplySpeedBufferSingle(in_pBuffer, out_pBuffer, bufferSize, speed);

			//VALIDATE
			AssertBufferSingleValues(expected, out_pBuffer, bufferSize);
		}

		TEST_METHOD(Method_CaclulateBufferSampleSize)
		{
			uint expected = 76800;
			uint bufferSampleSize = 0; //to be set by test

			//SETUP
			uint sampleRate = 48000;
			float splice = 1600; //ms
			MaggiLizerFXDSP* dsp = new MaggiLizerFXDSP;

			//TEST
			bufferSampleSize = dsp->CaclulateBufferSampleSize(sampleRate, splice);

			//VALIDATE
			Assert::AreEqual(expected, bufferSampleSize);
		}

		TEST_METHOD(Method_CalculateBufferSizeChangeFromSpeed)
		{
			uint expected = 48000;
			uint newBufferSpeed = 0; //to be set by test

			//SETUP
			uint bufferSize = 96000;
			float speed = 2;

			MaggiLizerFXDSP* dsp = new MaggiLizerFXDSP;

			//TEST
			newBufferSpeed = dsp->CalculateBufferSizeChangeFromSpeed(bufferSize, speed);

			//VALIDATE
			Assert::AreEqual(expected, newBufferSpeed);
		}

		TEST_METHOD(Method_GetBufferValue)
		{
			float input[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
			float expected = 4;
			float outputValue = -1; //to be set by test

			//SETUP
			uint channels = 2;
			uint bufferSize = 10;
			uint channelIndex = 0;
			uint bufferPosition = 4;

			MaggiLizerFXDSP* dsp = new MaggiLizerFXDSP;

			buffer pBuffer = FillBufferWithData(channels, input, bufferSize);

			//TEST
			outputValue = dsp->GetBufferValue(pBuffer, channelIndex, bufferPosition);

			//VALIDATE
			Assert::AreEqual(expected, outputValue);
		}

		TEST_METHOD(Method_CalculateWetDryMix)
		{
			float expected = 0.5;
			float mix = 0; //to be set by test

			//SETUP
			float input = 1;
			float output = 0;
			float mixPercent = 0.5;
			MaggiLizerFXDSP* dsp = new MaggiLizerFXDSP;

			//TEST
			mix = dsp->CalculateWetDryMix(input, output, mixPercent);

			//VALIDATE
			Assert::AreEqual(expected, mix);

		}

		TEST_METHOD(Method_CalculateSpeed)
		{
			float expected = 0.5;
			float speed = 0;

			//SETUP
			float pitch = -1200;

			MaggiLizerFXDSP* dsp = new MaggiLizerFXDSP;

			//TEST
			speed = dsp->CalculateSpeed(pitch);

			//VALIDATE
			Assert::AreEqual(expected, speed);
		}


		/// <summary>
		/// DESCRIPTION:
		/// When the plugin first starts receiving samples, it has not yet filled it's playback buffer.
		/// While it is filling it's uinternal buffer for playback, it outputs silence (0).
		/// </summary>
		TEST_METHOD(Execute_SetsInitialBufferToZero)
		{
			float expected[] = { 0,0,0,0,0,0,0,0,0,0 };
			float input[] = { 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1 };

			uint sampleRate = 48000;
			uint channels = 1;
			uint bufferBlockSize = 10;
			bool reverse = false;
			float pitch = 0;
			float splice = 500;//ms
			float delay = 0;
			float recycle = 0;
			float mix = 1;

			buffer pBuffer = FillBufferWithData(channels, input, bufferBlockSize);

			MaggiLizerFXDSP* p_DSP = new MaggiLizerFXDSP();
			p_DSP->Init(sampleRate, splice, channels);

			p_DSP->Execute(pBuffer, channels, bufferBlockSize, recycle, pitch, splice, delay, recycle, mix);
			
			AssertBufferSingleValues(expected, pBuffer[0], bufferBlockSize);
		}


		/// <summary>
		/// Description:
		/// If Mix value is set to 0, input buffer should match output buffer
		/// </summary>
		TEST_METHOD(Execute_MixOfZeroDoesNotModifyBuffer)
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

			buffer pBuffer = FillBufferWithData(channels, new float[]{1, 2, 3}, 3, 5);
			
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
