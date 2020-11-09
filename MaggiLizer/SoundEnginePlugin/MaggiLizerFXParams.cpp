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

  Copyright (c) 2020 Audiokinetic Inc.
*******************************************************************************/

#include "MaggiLizerFXParams.h"

#include <AK/Tools/Common/AkBankReadHelpers.h>

MaggiLizerFXParams::MaggiLizerFXParams()
{
}

MaggiLizerFXParams::~MaggiLizerFXParams()
{
}

MaggiLizerFXParams::MaggiLizerFXParams(const MaggiLizerFXParams& in_rParams)
{
    RTPC = in_rParams.RTPC;
    //NonRTPC = in_rParams.NonRTPC;
    m_paramChangeHandler.SetAllParamChanges();
}

AK::IAkPluginParam* MaggiLizerFXParams::Clone(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, MaggiLizerFXParams(*this));
}

AKRESULT MaggiLizerFXParams::Init(AK::IAkPluginMemAlloc* in_pAllocator, const void* in_pParamsBlock, AkUInt32 in_ulBlockSize)
{
    if (in_ulBlockSize == 0)
    {
        // Initialize default parameters here
        RTPC.bReverse = false;
        RTPC.fPitch = 0.0f;
        RTPC.fSplice = 0.0f;
        RTPC.fDelay = 0.0f;
        RTPC.fRecycle = 0.0f;
        RTPC.fMix = 0.0f;
        m_paramChangeHandler.SetAllParamChanges();
        return AK_Success;
    }

    return SetParamsBlock(in_pParamsBlock, in_ulBlockSize);
}

AKRESULT MaggiLizerFXParams::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT MaggiLizerFXParams::SetParamsBlock(const void* in_pParamsBlock, AkUInt32 in_ulBlockSize)
{
    AKRESULT eResult = AK_Success;
    AkUInt8* pParamsBlock = (AkUInt8*)in_pParamsBlock;

    // Read bank data here
    RTPC.bReverse = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.fPitch = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.fSplice = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.fDelay = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.fRecycle = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.fMix = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    CHECKBANKDATASIZE(in_ulBlockSize, eResult);
    m_paramChangeHandler.SetAllParamChanges();

    return eResult;
}

AKRESULT MaggiLizerFXParams::SetParam(AkPluginParamID in_paramID, const void* in_pValue, AkUInt32 in_ulParamSize)
{
    AKRESULT eResult = AK_Success;

    // Handle parameter change here
    switch (in_paramID)
    {
    case PARAM_REVERSE_ID:
        RTPC.bReverse = (*(AkReal32*)(in_pValue)) != 0; // bools are conveyed as 0 false, !=0 true
        m_paramChangeHandler.SetParamChange(PARAM_REVERSE_ID);
        break;
    case PARAM_PITCH_ID:
        RTPC.fPitch = *(AkReal32*)(in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_PITCH_ID);
        break;
    // TODO: Implement Delay
    // -- issue https://github.com/rjmattingly/MaggiLizer/projects/1#card-49020915
    case PARAM_DELAY_ID:
        RTPC.fDelay = *(AkReal32*)(in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_DELAY_ID);
        break;
    case PARAM_SPLICE_ID:
        RTPC.fSplice = *(AkReal32*)(in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_SPLICE_ID);
        break;
    // TODO: Implement Recycyle
    // -- issue https://github.com/rjmattingly/MaggiLizer/projects/1#card-49020926
    case PARAM_RECYCLE_ID:
        RTPC.fRecycle = *(AkReal32*)(in_pValue) / 100; //convert from 0-100% to 0-1
        m_paramChangeHandler.SetParamChange(PARAM_RECYCLE_ID);
        break;
    case PARAM_MIX_ID:
        RTPC.fMix = *(AkReal32*)(in_pValue)/100; //convert from 0-100% to 0-1
        m_paramChangeHandler.SetParamChange(PARAM_MIX_ID);
    default:
        eResult = AK_InvalidParameter;
        break;
    }

    return eResult;
}
