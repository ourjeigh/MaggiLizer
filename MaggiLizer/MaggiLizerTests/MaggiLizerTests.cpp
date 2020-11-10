#include "pch.h"
#include "CppUnitTest.h"
#include "../SoundEnginePlugin/MaggiLizerFX.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace MaggiLizerTests
{
	TEST_CLASS(MaggiLizerTests)
	{
	public:
		
		TEST_METHOD(TestOneEquals)
		{
			Assert::AreEqual(1, 1);
		}
	};
}
