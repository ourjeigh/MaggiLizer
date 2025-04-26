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
#include "utilities.h"
#include "splice.h"

typedef AkReal32* AK_RESTRICT AKReal32_Restrict;

const AkReal32 k_max_splice_buffer_seconds = 1.0f;
const AkReal32 k_cache_buffer_seconds = k_max_splice_buffer_seconds * 2.25f;
const AkReal32 k_max_smoothing_ratio = 0.25f;
// TODO: Use this to verify param inputs
const AkReal32 k_max_speed = 2.0f;

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
	m_pCacheBufferMemory(nullptr),
	m_uCacheBufferSize(0),
	m_pScratchBuffer(nullptr),
	m_uSamplesPerFrame(0),
	m_pCacheBuffers(nullptr),
	m_pSplices(nullptr),
	m_pCrossfadeSplices(nullptr),
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

	AkAudioSettings audioSettings;
	in_pContext->GlobalContext()->GetAudioSettings(audioSettings);
	m_uSamplesPerFrame = audioSettings.uNumSamplesPerFrame;

	// allocate buffer memories
	const AkUInt32 uPlaybackBufferSize = k_cache_buffer_seconds * m_uSampleRate * m_uChannelCount;
	m_uCacheBufferSize = AK_ALIGN_TO_NEXT_BOUNDARY(uPlaybackBufferSize, m_uSamplesPerFrame);
	m_pCacheBufferMemory = static_cast<AkReal32*>(AK_PLUGIN_ALLOC(in_pAllocator, sizeof(AkReal32) * m_uCacheBufferSize));
	
	// scratch buffer needs to be large enough to temporarily hold a double frame buffer so that it can be pitched up into a single frame length
	m_pScratchBuffer = static_cast<AkReal32*>(AK_PLUGIN_ALLOC(in_pAllocator, sizeof(AkReal32) * m_uSamplesPerFrame * k_max_speed));

	// allocate containers
	m_pCacheBuffers = static_cast<RingBuffer*>(AK_PLUGIN_ALLOC(in_pAllocator, sizeof(RingBuffer) * m_uChannelCount));
	m_pSplices = static_cast<Splice*>(AK_PLUGIN_ALLOC(in_pAllocator, sizeof(Splice) * m_uChannelCount));
	m_pCrossfadeSplices = static_cast<Splice*>(AK_PLUGIN_ALLOC(in_pAllocator, sizeof(Splice) * m_uChannelCount));

	if (m_pCacheBufferMemory == nullptr ||
		m_pScratchBuffer == nullptr ||
		m_pCacheBuffers == nullptr ||
		m_pSplices == nullptr ||
		m_pCrossfadeSplices == nullptr)
	{
		return AK_InsufficientMemory;
	}

	AkZeroMemLarge(m_pCacheBufferMemory, sizeof(AkReal32) * m_uCacheBufferSize);
	AkZeroMemLarge(m_pScratchBuffer, sizeof(AkReal32) * m_uSamplesPerFrame);
	AkZeroMemSmall(m_pCacheBuffers, sizeof(RingBuffer) * m_uChannelCount);
	AkZeroMemSmall(m_pSplices, sizeof(Splice) * m_uChannelCount);
	AkZeroMemSmall(m_pCrossfadeSplices, sizeof(Splice) * m_uChannelCount);

	// setup containers
	for (AkUInt16 uChannelIndex = 0; uChannelIndex < m_uChannelCount; uChannelIndex++)
	{
		// assign each buffer a block of splice memory
		AkUInt32 uCacheBufferSamplesPerChannel = k_cache_buffer_seconds * m_uSampleRate;
		m_pCacheBuffers[uChannelIndex].AttachData(&m_pCacheBufferMemory[uCacheBufferSamplesPerChannel * uChannelIndex], uCacheBufferSamplesPerChannel);

		m_pSplices[uChannelIndex].SetAttachedBufferSize(uCacheBufferSamplesPerChannel);
		m_pCrossfadeSplices[uChannelIndex].SetAttachedBufferSize(uCacheBufferSamplesPerChannel);
	}

	return AK_Success;
}

AKRESULT maggilizerFX::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
	// we are NOT responsible for freeing the cached m_pParams (and Wwise will crash if we do)
	m_pParams = nullptr;

	AK_PLUGIN_FREE(in_pAllocator, m_pCacheBuffers);
	AK_PLUGIN_FREE(in_pAllocator, m_pCacheBufferMemory);
	AK_PLUGIN_FREE(in_pAllocator, m_pScratchBuffer);
	AK_PLUGIN_FREE(in_pAllocator, m_pSplices);
	AK_PLUGIN_FREE(in_pAllocator, m_pCrossfadeSplices);

	AK_PLUGIN_DELETE(in_pAllocator, this);

	return AK_Success;
}

AKRESULT maggilizerFX::Reset()
{
	m_uTailPosition = 0;

	AkZeroMemLarge(m_pCacheBufferMemory, sizeof(AkReal32) * m_uCacheBufferSize);

	for (AkUInt16 uChannelIndex = 0; uChannelIndex < m_uChannelCount; uChannelIndex++)
	{
		m_pCacheBuffers[uChannelIndex].Reset();
		m_pSplices[uChannelIndex].Reset();
		m_pCrossfadeSplices[uChannelIndex].Reset();

		const SpliceSettings settings = GetSettings();

		// Backtrack the initial splice into a silent portion of the cache buffer so that when the first splice is ready,
		// it will advance into the filled portion of the buffer. This is a bit of a hack since we haven't even set splice
		// settings yet, we're just waiting for read to wrap back around to the initial end position of 0.
		m_pSplices[uChannelIndex].SetInitialPosition(m_pCacheBuffers[uChannelIndex].GetSize() - settings.uSpliceSamples);
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

SpliceSettings maggilizerFX::GetSettings()
{
	SpliceSettings settings;

	settings.bReverse = m_pParams->m_rtpcs.bReverse;
	settings.fRecycle = m_pParams->m_rtpcs.fRecycle;
	settings.fMix = m_pParams->m_rtpcs.fMix;
	settings.fSpeed = CalculateSpeed(m_pParams->m_rtpcs.fPitch);

	const AkReal32 fSplice = m_pParams->m_rtpcs.fSplice;
	const AkReal32 fDelay = m_pParams->m_rtpcs.fDelay;
	settings.uSpliceSamples = AK_ALIGN_TO_NEXT_BOUNDARY(ConvertMillisecondsToSamples(m_uSampleRate, m_pParams->m_rtpcs.fSplice), m_uSamplesPerFrame);
	settings.uDelaySamples = AK_ALIGN_TO_NEXT_BOUNDARY(ConvertMillisecondsToSamples(m_uSampleRate, m_pParams->m_rtpcs.fDelay), m_uSamplesPerFrame);

	settings.uSmoothingSamples = static_cast<AkUInt32>(settings.uSpliceSamples * k_max_smoothing_ratio * m_pParams->m_rtpcs.fSmoothing);

	return settings;
}

void maggilizerFX::Execute(AkAudioBuffer* io_pBuffer)
{
	// Check state first because the tail handler will add valid samples and update the state
	const bool bNoMoreData = io_pBuffer->eState == AK_NoMoreData;

	// Set constants
	const AkUInt32 uChannelCount = io_pBuffer->NumChannels();
	const SpliceSettings settings = GetSettings();

	// Update Tail Handler
	const AkUInt32 uMaxTailSamples = AkMax(settings.uSpliceSamples, (m_uSampleRate * k_cache_buffer_seconds) * m_pParams->m_rtpcs.fRecycle);
	m_TailHandler.HandleTail(io_pBuffer, uMaxTailSamples);

	// Set buffer size after tail handler because it will adjust the buffer frames in tail mode
	const AkUInt32 uBufferSize = io_pBuffer->uValidFrames;
	AkReal32 fTailMix = 1.0f;

	if (bNoMoreData)
	{
		// Move this into ProcessChannel, just passing in the tail position instead so that we can use 
		// CalculateEqualPowerFadeOut
		fTailMix = CalculateWetDryMix(1.0f, 0.0f, static_cast<AkReal32>(m_uTailPosition) / uMaxTailSamples);
		m_uTailPosition += uBufferSize;
	}

	for (AkUInt32 uChannelndex = 0; uChannelndex < uChannelCount; uChannelndex++)
	{
		AKReal32_Restrict pBuffer = static_cast<AKReal32_Restrict>(io_pBuffer->GetChannel(uChannelndex));
		Splice* pSplice = &m_pSplices[uChannelndex];
		Splice* pCrossfadeSplice = &m_pCrossfadeSplices[uChannelndex];
		RingBuffer* pCacheBuffer = &m_pCacheBuffers[uChannelndex];

		ProcessChannel(
			pBuffer,
			uBufferSize,
			pCacheBuffer,
			pSplice,
			pCrossfadeSplice,
			settings,
			fTailMix);
	}
}

inline void maggilizerFX::ProcessChannel(
	AkReal32* pBuffer,
	AkUInt32 uBufferSize,
	RingBuffer* pCacheBuffer,
	Splice* pSplice,
	Splice* pCrossfadeSplice,
	const SpliceSettings& settings,
	AkReal32 fTailMix)
{
	AkZeroMemLarge(m_pScratchBuffer, sizeof(AkReal32) * m_uSamplesPerFrame);
	
	if (pSplice->IsProcessingComplete())
	{
		pSplice->PrepareNextSplice(settings);
	}

	// write the splice output into the scratch buffer so it can be mixed 
	pSplice->Process(pCacheBuffer, uBufferSize, m_pScratchBuffer, pBuffer);

	// Write the input buffer to the cache buffer
	pCacheBuffer->WriteBlock(pBuffer, uBufferSize);

	// BAD: If recycle has mixed output back into pBuffer above, it will invalidate a 0-mixed output here
	MixBufferBIntoA(pBuffer, m_pScratchBuffer, uBufferSize, settings.fMix);

	// Handle tail
	// TODO: Smooth this out, we're setting the same mix value for the entire buffer
	if (fTailMix < 1.0f)
	{
		for (AkUInt32 uBufferIndex = 0; uBufferIndex < uBufferSize; uBufferIndex++)
		{
			pBuffer[uBufferIndex] *= fTailMix;
		}
	}
}

AKRESULT maggilizerFX::TimeSkip(AkUInt32 in_uFrames)
{
	return AK_DataReady;
}
