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

		typedef float MonoBufferx10[1][10];

	public:
		void FillBufferWithData(float* pBuffer, float pattern[], int size, int repeats)
		{

			for (int i = 0; i < repeats; i++)
			{
				for (int j = 0; j < size; j++)
				{
					pBuffer[(size * i + j)] = pattern[j];
				}
			}
		}

		void AssertBufferValues(float expected[], float* actual, int size)
		{
			for (int i = 0; i < size; i++)
			{
				Assert::AreEqual(expected[i], actual[i]);
			}
		}



		TEST_METHOD(FillBufferHelper)
		{
			float expected[] = { 0,1,2,0,1,2,0,1,2 };

			float* pBuffer = new float[10];
			FillBufferWithData(pBuffer, new float[] { 0, 1, 2 }, 3, 3);

			AssertBufferValues(expected, pBuffer, 9);
		}

		/// <summary>
		/// DESCRIPTION:
		/// When the plugin first starts receiving samples, it has not yet filled it's playback buffer.
		/// While it is filling it's internal buffer for playback, it outputs silence (0).
		/// </summary>
		TEST_METHOD(SetsInitialBufferToZero)
		{
			float expected[] = { 0,0,0,0,0,0,0,0,0,0 };

			int sampleRate = 48000;
			int channels = 1;
			int frames = 10;
			bool reverse = false;
			float pitch = 0;
			float splice = 500;//ms
			float delay = 0;
			float recycle = 0;
			float mix = 1;

			float** pBuffer = new float* [1];
			pBuffer[0] = new float[frames];
			FillBufferWithData(pBuffer[0], new float[] {0.1}, 1, frames);

			MaggiLizerFXDSP* p_DSP = new MaggiLizerFXDSP();
			p_DSP->Init(sampleRate, splice, channels);

			p_DSP->Execute(pBuffer, channels, frames, recycle, pitch, splice, delay, recycle, mix);
			
			AssertBufferValues(expected, pBuffer[0], frames);
		}


		/// <summary>
		/// Description:
		/// If Mix value is set to 0, input buffer should match output buffer
		/// </summary>
		TEST_METHOD(MixOfZeroDoesNotModifyBuffer)
		{
			float expected[] = { 1,2,3,1,2,3,1,2,3,1,2,3,1,2,3 };

			

			int sampleRate = 48000;
			int channels = 1;
			int frames = 15;
			bool reverse = false;
			float pitch = 0;
			float splice = 500;//ms
			float delay = 0;
			float recycle = 0;
			float mix = 0;

			float** pBuffer = new float* [1];
			pBuffer[0] = new float[frames];
			FillBufferWithData(pBuffer[0], new float[] {1, 2, 3}, 3, 5);
			
			MaggiLizerFXDSP* p_DSP = new MaggiLizerFXDSP();
			p_DSP->Init(sampleRate, splice, channels);

			p_DSP->Execute(pBuffer, channels, frames, recycle, pitch, splice, delay, recycle, mix);

			AssertBufferValues(expected, pBuffer[0], frames);
		}

		//TEST_METHOD(Reverse)
		//{
		//	float expected[] = { 0.5,0.4,0.3,0.2,0.1};

		//	int sampleRate = 5;
		//	int channels = 1;
		//	int frames = 5;
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
	};
}
