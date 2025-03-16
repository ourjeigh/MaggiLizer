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

#include "maggilizerFXParams.h"

#include <AK/Tools/Common/AkBankReadHelpers.h>

maggilizerFXParams::maggilizerFXParams()
{
    m_rtpcs.clear();
}

maggilizerFXParams::~maggilizerFXParams()
{
}

maggilizerFXParams::maggilizerFXParams(const maggilizerFXParams& in_rParams)
{
    m_rtpcs = in_rParams.m_rtpcs;
    m_paramChangeHandler.SetAllParamChanges();
}

AK::IAkPluginParam* maggilizerFXParams::Clone(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, maggilizerFXParams(*this));
}

AKRESULT maggilizerFXParams::Init(AK::IAkPluginMemAlloc* in_pAllocator, const void* in_pParamsBlock, AkUInt32 in_ulBlockSize)
{
    if (in_ulBlockSize == 0)
    {
        m_rtpcs.clear();
        m_paramChangeHandler.SetAllParamChanges();
        return AK_Success;
    }

    return SetParamsBlock(in_pParamsBlock, in_ulBlockSize);
}

AKRESULT maggilizerFXParams::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT maggilizerFXParams::SetParamsBlock(const void* in_pParamsBlock, AkUInt32 in_ulBlockSize)
{
    AKRESULT eResult = AK_Success;
    AkUInt8* pParamsBlock = (AkUInt8*)in_pParamsBlock;

    // Read bank data here
    // test this!
    m_rtpcs.bReverse = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize) != 0;
    m_rtpcs.fPitch = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    m_rtpcs.fSplice = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    m_rtpcs.fDelay = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    m_rtpcs.fRecycle = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize) / 100.0f;
    m_rtpcs.fMix = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize) / 100.0f; 
    CHECKBANKDATASIZE(in_ulBlockSize, eResult);
    m_paramChangeHandler.SetAllParamChanges();

    return eResult;
}

AKRESULT maggilizerFXParams::SetParam(AkPluginParamID in_paramID, const void* in_pValue, AkUInt32 in_ulParamSize)
{
    AKRESULT eResult = AK_Success;

    // Handle parameter change here
    switch (in_paramID)
    {
    case param_id_reverse:
        m_rtpcs.bReverse = (*static_cast<const AkReal32*>(in_pValue)) != 0; // bools are conveyed as 0 false, !=0 true
        m_paramChangeHandler.SetParamChange(param_id_reverse);
        break;
    case param_id_pitch:
        m_rtpcs.fPitch = *static_cast<const AkReal32*>(in_pValue);
        m_paramChangeHandler.SetParamChange(param_id_pitch);
        break;
    case param_id_delay:
        // TODO: Implement Delay
        // -- issue https://github.com/rjmattingly/MaggiLizer/projects/1#card-49020915
        m_rtpcs.fDelay = *static_cast<const AkReal32*>(in_pValue);
        m_paramChangeHandler.SetParamChange(param_id_delay);
        break;
    case param_id_splice:
        m_rtpcs.fSplice = *static_cast<const AkReal32*>(in_pValue);
        m_paramChangeHandler.SetParamChange(param_id_splice);
        break;
    case param_id_recycle:
        m_rtpcs.fRecycle = *static_cast<const AkReal32*>(in_pValue) / 100.0f; // convert from 0-100% to 0-1
        m_paramChangeHandler.SetParamChange(param_id_recycle);
        break;
    case param_id_mix:
        m_rtpcs.fMix = *static_cast<const AkReal32*>(in_pValue) / 100.0f; // convert from 0-100% to 0-1
        m_paramChangeHandler.SetParamChange(param_id_mix);
        break;
    default:
        eResult = AK_InvalidParameter;
        break;
    }

    return eResult;
}
