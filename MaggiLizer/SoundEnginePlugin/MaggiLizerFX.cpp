/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the
"Apache License"); you may not use this file except in compliance with the
Apache License. You may obtain a copy of the Apache License at
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Copyright (c) 2024 Audiokinetic Inc.
*******************************************************************************/

#include "maggilizerFX.h"
#include "../maggilizerConfig.h"

#include <AK/AkWwiseSDKVersion.h>
#include "buffer_utilities.h"
#include "splice.h"

typedef AkReal32* AK_RESTRICT AKReal32_Restrict;

const AkReal32 k_playback_buffer_seconds = 20.0f;
const AkReal32 k_max_splice_buffer_seconds = 2.0f;
const AkUInt16 k_crossfade_frames = 1024; // this is good down to ~40Hz so we may need to add HPF

AK::IAkPlugin* CreatemaggilizerFX(AK::IAkPluginMemAlloc* in_pAllocator)
{
	return AK_PLUGIN_NEW(in_pAllocator, maggilizerFX());
}

AK::IAkPluginParam* CreatemaggilizerFXParams(AK::IAkPluginMemAlloc* in_pAllocator)
{
	return AK_PLUGIN_NEW(in_pAllocator, maggilizerFXParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(maggilizerFX, AkPluginTypeEffect, maggilizerConfig::CompanyID, maggilizerConfig::PluginID)

maggilizerFX::maggilizerFX() :
	m_pParams(nullptr),
	m_uChannelCount(0),
	m_uSampleRate(0),
	m_pSpliceBufferMemory(nullptr),
	m_uSpliceBufferSize(0),
	m_pPlaybackBufferMemory(nullptr),
	m_uPlaybackBufferSize(0),
	m_pSplices(nullptr),
	m_pPlaybacks(nullptr),
	m_uTailPosition(0)
{
}

maggilizerFX::~maggilizerFX()
{
}

AKRESULT maggilizerFX::Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkEffectPluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat)
{
	m_pParams = static_cast<maggilizerFXParams*>(in_pParams);
	m_uSampleRate = in_rFormat.uSampleRate;
	m_uChannelCount = in_rFormat.GetNumChannels();

	// allocate buffer memories
	const AkUInt32 uPlaybackBufferSize = k_playback_buffer_seconds * m_uSampleRate * m_uChannelCount;
	m_uPlaybackBufferSize = AK_ALIGN_TO_NEXT_BOUNDARY(uPlaybackBufferSize, in_rFormat.GetBlockAlign());
	m_pPlaybackBufferMemory = static_cast<AkReal32*>(AK_PLUGIN_ALLOC(in_pAllocator, sizeof(AkReal32) * m_uPlaybackBufferSize));


	// I think we need to add *1x or 2x* k_crossfade_frames to the splice buffer size (per channel)
	const AkUInt32 uSpliceBufferSize = k_max_splice_buffer_seconds * m_uSampleRate * m_uChannelCount;
	m_uSpliceBufferSize = AK_ALIGN_TO_NEXT_BOUNDARY(uSpliceBufferSize, in_rFormat.GetBlockAlign());
	m_pSpliceBufferMemory = static_cast<AkReal32*>(AK_PLUGIN_ALLOC(in_pAllocator, sizeof(AkReal32) * m_uSpliceBufferSize));

	// allocate containers
	m_pSplices = static_cast<Splice*>(AK_PLUGIN_ALLOC(in_pAllocator, sizeof(Splice) * m_uChannelCount));
	m_pPlaybacks = static_cast<RingBuffer*>(AK_PLUGIN_ALLOC(in_pAllocator, sizeof(RingBuffer) * m_uChannelCount));

	if (m_pPlaybackBufferMemory == nullptr ||
		m_pSpliceBufferMemory == nullptr ||
		m_pSplices == nullptr ||
		m_pPlaybacks == nullptr)
	{
		return AK_InsufficientMemory;
	}

	AkZeroMemLarge(m_pPlaybackBufferMemory, sizeof(AkReal32) * m_uPlaybackBufferSize);
	AkZeroMemLarge(m_pSpliceBufferMemory, sizeof(AkReal32) * m_uSpliceBufferSize);
	AkZeroMemSmall(m_pSplices, sizeof(Splice) * m_uChannelCount);
	AkZeroMemSmall(m_pPlaybacks, sizeof(RingBuffer) * m_uChannelCount);

	// setup containers
	for (AkUInt16 uChannelIndex = 0; uChannelIndex < m_uChannelCount; uChannelIndex++)
	{
		// assign each splice buffer a block of splice memory
		AkUInt32 uSpliceSamplesPerChannel = k_max_splice_buffer_seconds * m_uSampleRate;
		m_pSplices[uChannelIndex].AttachData(&m_pSpliceBufferMemory[uSpliceSamplesPerChannel * uChannelIndex], uSpliceSamplesPerChannel);

		AkUInt32 uPlaybackSamplesPerChannel = k_playback_buffer_seconds * m_uSampleRate;
		m_pPlaybacks[uChannelIndex].AttachData(&m_pPlaybackBufferMemory[uPlaybackSamplesPerChannel * uChannelIndex], uPlaybackSamplesPerChannel);
	}

	return AK_Success;
}

AKRESULT maggilizerFX::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
	// we are NOT responsible for freeing the cached m_pParams (and Wwise will crash if we do)
	m_pParams = nullptr;

	AK_PLUGIN_FREE(in_pAllocator, m_pSplices);
	AK_PLUGIN_FREE(in_pAllocator, m_pPlaybacks);
	AK_PLUGIN_FREE(in_pAllocator, m_pPlaybackBufferMemory);
	AK_PLUGIN_FREE(in_pAllocator, m_pSpliceBufferMemory);

	AK_PLUGIN_DELETE(in_pAllocator, this);

	return AK_Success;
}

AKRESULT maggilizerFX::Reset()
{
	m_uTailPosition = 0;

	AkZeroMemLarge(m_pPlaybackBufferMemory, sizeof(AkReal32) * m_uPlaybackBufferSize);
	AkZeroMemLarge(m_pSpliceBufferMemory, sizeof(AkReal32) * m_uSpliceBufferSize);

	for (AkUInt16 uChannelIndex = 0; uChannelIndex < m_uChannelCount; uChannelIndex++)
	{
		m_pSplices[uChannelIndex].Reset();
		m_pPlaybacks[uChannelIndex].Reset();
	}

	return AK_Success;
}

AKRESULT maggilizerFX::GetPluginInfo(AkPluginInfo& out_rPluginInfo)
{
	out_rPluginInfo.eType = AkPluginTypeEffect;
	out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.bCanProcessObjects = false;
	out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
	return AK_Success;
}

void maggilizerFX::Execute(AkAudioBuffer* io_pBuffer)
{
	// Check state first because the tail handler with add valid samples and update the state
	const bool bNoMoreData = io_pBuffer->eState == AK_NoMoreData;

	// Set constants
	const AkUInt32 uChannelCount = io_pBuffer->NumChannels();

	const bool bReverse = m_pParams->m_rtpcs.bReverse;
	const AkReal32 fDelay = m_pParams->m_rtpcs.fDelay;
	const AkReal32 fRecycle = m_pParams->m_rtpcs.fRecycle;
	const AkReal32 fMix = m_pParams->m_rtpcs.fMix;

	const AkReal32 fSpeed = CalculateSpeed(m_pParams->m_rtpcs.fPitch);
	const AkUInt32 uSpliceSize = ConvertMillisecondsToSamples(m_uSampleRate, m_pParams->m_rtpcs.fSplice);
	const AkUInt32 uDelaySize = ConvertMillisecondsToSamples(m_uSampleRate, m_pParams->m_rtpcs.fDelay);

	// Update Tail Handler
	const AkUInt32 uMaxTailSamples = ((m_uSampleRate * k_max_splice_buffer_seconds) * (1 / CalculateSpeed(m_pParams->m_rtpcs.fPitch))) * (1 + m_pParams->m_rtpcs.fRecycle);
	m_TailHandler.HandleTail(io_pBuffer, uMaxTailSamples);

	// Set buffer size after tail handler because it will adjust the buffer frames in tail mode
	const AkUInt32 uBufferSize = io_pBuffer->uValidFrames;
	AkReal32 fTailMix = 1.0f;

	if (bNoMoreData)
	{
		fTailMix = CalculateWetDryMix(1.0f, 0.0f, static_cast<AkReal32>(m_uTailPosition) / uMaxTailSamples);
		m_uTailPosition += uBufferSize;
	}

	// temp hack
	AkReal32 scratchBuffer[512];
	AkZeroMemLarge(&scratchBuffer[0], sizeof(AkReal32) * 512);

	for (AkUInt32 uChannelndex = 0; uChannelndex < uChannelCount; uChannelndex++)
	{
		AKReal32_Restrict pBuffer = static_cast<AKReal32_Restrict>(io_pBuffer->GetChannel(uChannelndex));
		Splice* pSplice = &m_pSplices[uChannelndex];
		RingBuffer* pPlayback = &m_pPlaybacks[uChannelndex];

		if (pSplice->IsEmpty())
		{
			pSplice->UpdateSettings(
				bReverse,
				fSpeed,
				uSpliceSize,
				//uDelaySize,
				//fMix,
				fRecycle);
		}

		AkUInt32 uSpliceSamplesToWrite = AkMin(uBufferSize, pSplice->FreeSpace());

		if (uSpliceSamplesToWrite > 0)
		{
			// pre-fill splice with recycled data if needed
			if (fRecycle > 0.0f && pPlayback->HasData())
			{
				pSplice->SetOutptReadPosition(
					pPlayback->PeekBlock(
						&pSplice->GetData()[pSplice->m_uWritePosition],
						uSpliceSamplesToWrite,
						pSplice->GetOutputReadPosition()));

				//pPlayback->PeekReadBlock(scratchBuffer, uSpliceWrite);
			}

			// mix input to splice
			pSplice->MixInBlock(pBuffer, uSpliceSamplesToWrite);
		}

		if (pSplice->IsFull())
		{
			// handle delay by advancing the write position by the delay amount before writing splice data in
			pPlayback->AdvanceWriteHead(uDelaySize);

			// write the processed splice into the playback buffer
			AkUInt32 uFramesWritten = pSplice->PushToBuffer(*pPlayback, k_crossfade_frames);
			
			// TODO: now backtrack the write buffer by crossfade frames / 2

			pSplice->Reset();
		}

		AkUInt32 uRemainingSpliceSamplesToWrite = uBufferSize - uSpliceSamplesToWrite;

		if (uRemainingSpliceSamplesToWrite > 0)
		{
			pSplice->UpdateSettings(
				bReverse,
				fSpeed,
				uSpliceSize,
				//uDelaySize,
				//fMix,
				fRecycle);

			// write remaining input to splice
			pSplice->MixInBlock(pBuffer, uRemainingSpliceSamplesToWrite);
		}

		// read playback into the scratch buffer
		if (pPlayback->HasData())
		{
			pPlayback->ReadBlock(&scratchBuffer[0], uBufferSize);
		}

		// mix scratch buffer with input
		for (AkUInt32 frame = 0; frame < uBufferSize; frame++)
		{
			pBuffer[frame] = (((1 - fMix) * pBuffer[frame]) + scratchBuffer[frame] * fMix) * fTailMix;
		}
	}
}

// old

#if 0
void maggilizerFX::ProcessSingleFrame(
	float* io_pBuffer,
	MonoBuffer* io_pSpliceBuffer,
	MonoBuffer* io_pPlaybackBuffer,
	const AkUInt32& in_uFrame,
	const bool& in_bReverse,
	const float& in_fPitch,
	const float& in_fSplice,
	const float& in_fDelay,
	const float& in_fRecycle,
	const float& in_fMix)
{
	float input = io_pBuffer[in_uFrame];

	// Delay
	// TODO: this is wrong, it just delays the first splice playback
	// Move this to the splice filled block and write the new splice into the playback buffer with the delay offset
	if (in_fDelay > 0)
	{
		const AkUInt32 delaySampleSize = MonoBufferUtilities::ConvertMillisecondsToSamples(m_uSampleRate, in_fDelay);
		io_pPlaybackBuffer->SetReadDelay(delaySampleSize);
	}

	// Splice
	if (io_pSpliceBuffer->IsFilled())
	{
		// Reverse 
		if (in_bReverse)
		{
			MonoBufferUtilities::ApplyReverseBufferSingle(io_pSpliceBuffer);
		}

		// Pitch
		const AkUInt32 playbackWritePosition = io_pPlaybackBuffer->WritePosition();
		const float fSpeed = MonoBufferUtilities::CalculateSpeed(in_fPitch);
		const AkUInt32 filledPlaybackSampleSize = MonoBufferUtilities::ApplySpeedBufferSingle(io_pSpliceBuffer, io_pPlaybackBuffer, fSpeed);

		//MonoBufferUtilities::ApplySmoothingAtIndex(io_pPlaybackBuffer, playbackWritePosition, m_cSmoothWindowSize);
		const AkUInt32 uSpliceSampleSize = MonoBufferUtilities::ConvertMillisecondsToSamples(m_uSampleRate, in_fSplice);
		io_pSpliceBuffer->SetBufferSize(uSpliceSampleSize);

		//Recyle
		MonoBufferUtilities::CopyLastWrittenBufferBlock(io_pPlaybackBuffer, io_pSpliceBuffer, filledPlaybackSampleSize);
	}

	float recycled = 0;
	io_pSpliceBuffer->ReadNextBufferValue(recycled);
	recycled = input + recycled * in_fRecycle;

	io_pSpliceBuffer->WriteNextBufferValue(recycled);

	float output = 0;
	if (io_pPlaybackBuffer->HasData())
	{
		io_pPlaybackBuffer->ReadNextBufferValue(output);
	}

	float mix = MonoBufferUtilities::CalculateWetDryMix(input, output, in_fMix);

	io_pBuffer[in_uFrame] = mix;
}
#endif

AKRESULT maggilizerFX::TimeSkip(AkUInt32 in_uFrames)
{
	return AK_DataReady;
}
