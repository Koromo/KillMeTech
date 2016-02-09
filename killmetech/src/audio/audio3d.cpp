#include "audio3d.h"
#include "sourcevoice.h"
#include "xaudiosupport.h"
#include "../core/exception.h"
#include "../core/utility.h"
#include <Windows.h>

namespace killme
{
    Audio3D audio3D;

    template <>
    X3DAUDIO_VECTOR to<X3DAUDIO_VECTOR, Vector3>(const Vector3& v)
    {
        return{ v.x, v.y, v.z };
    }

    void Audio3D::startup(unsigned channelMask, size_t numSrcChannels)
    {
        numSrcCannels_ = numSrcChannels;
        numDestCannels_ = 2; /// TODO:
        levelMatrix_.resize(numSrcCannels_ * numDestCannels_, 0);

        enforce<XAudioException>(
            SUCCEEDED(X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, x3DAudio_)),
            "Failed to initialize the X3DAudio.");

        ZeroMemory(&listener_, sizeof(listener_));
    }

    void Audio3D::shutdown()
    {
    }

    void Audio3D::setListener(const ListenerParams& params)
    {
        listener_.OrientFront = to<X3DAUDIO_VECTOR>(params.orientation * Vector3::UNIT_Z);
        listener_.OrientTop = to<X3DAUDIO_VECTOR>(params.orientation * Vector3::UNIT_Y);
        listener_.Position = to<X3DAUDIO_VECTOR>(params.position);
        listener_.Velocity = to<X3DAUDIO_VECTOR>(params.velocity);
    }

    void Audio3D::calculate(const EmitterParams& params)
    {
        X3DAUDIO_EMITTER emitter;
        ZeroMemory(&emitter, sizeof(emitter));
        emitter.OrientFront = to<X3DAUDIO_VECTOR>(params.orientation * Vector3::UNIT_Z);
        emitter.OrientTop = to<X3DAUDIO_VECTOR>(params.orientation * Vector3::UNIT_Y);
        emitter.Position = to<X3DAUDIO_VECTOR>(params.position);
        emitter.Velocity = to<X3DAUDIO_VECTOR>(params.velocity);
        emitter.InnerRadius = 30;
        emitter.ChannelCount = 1; /// TODO:
        emitter.pVolumeCurve = const_cast<X3DAUDIO_DISTANCE_CURVE*>(&X3DAudioDefault_LinearCurve);
        emitter.CurveDistanceScaler = 500;
        emitter.DopplerScaler = 1;

        X3DAUDIO_DSP_SETTINGS dsp;
        dsp.pMatrixCoefficients = levelMatrix_.data();
        dsp.pDelayTimes = NULL;
        dsp.SrcChannelCount = numSrcCannels_;
        dsp.DstChannelCount = numDestCannels_;

        X3DAudioCalculate(x3DAudio_, &listener_, &emitter, X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER, &dsp);

        params.voice->applyFrequencyRatio(dsp.DopplerFactor);
        params.voice->applyOutputMatrix(dsp.SrcChannelCount, dsp.DstChannelCount, dsp.pMatrixCoefficients);
    }
}