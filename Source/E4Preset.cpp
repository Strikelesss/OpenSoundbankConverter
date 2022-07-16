#include "Header/E4Preset.h"
#include "Header/VoiceDefinitions.h"
#include <cmath>

float E4Voice::GetChorusWidth() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_chorusWidth);
}

float E4Voice::GetChorusAmount() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_chorusAmount);
}

uint16_t E4Voice::GetFilterFrequency() const
{
	return VoiceDefinitions::ConvertByteToFilterFrequency(m_filterFrequency);
}

double E4Voice::GetFineTune() const
{
	return VoiceDefinitions::ConvertByteToFineTune(m_fineTune);
}

double E4Voice::GetFilterQ() const
{
	return VoiceDefinitions::ConvertByteToFilterQ(m_filterQ);
}

std::string_view E4Voice::GetFilterType() const
{
	return VoiceDefinitions::GetFilterTypeFromByte(m_filterType);
}

float E4Envelope::GetAttack1Level() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_attack1Level);
}

float E4Envelope::GetAttack2Level() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_attack2Level);
}

float E4Envelope::GetDecay1Level() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_decay1Level);
}

float E4Envelope::GetDecay2Level() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_decay2Level);
}

float E4Envelope::GetRelease1Level() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_release1Level);
}

float E4Envelope::GetRelease2Level() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_release2Level);
}

[[nodiscard]] double GetTimeFromCurve(const uint8_t b)
{
	//const auto time(SEC_A * std::pow(SEC_B, static_cast<double>(b)));
	//return time + std::fmod(time, 3.); // fmod gets somewhat closer to the real number, although far off for lower numbers

	const auto percent(static_cast<double>(b) * 14. / 127.);
	const auto sec((std::pow(2., percent) * 10.) / 1000.);
	return sec + std::fmod(sec, 2.);
}

double E4Envelope::GetAttack1Sec() const
{
	//return E4BVariables::envelopeValues[std::clamp(m_ampEnvelope.GetAttack1Sec(), 0ui8, 126ui8)];
	if(m_attack1Sec > 0) { return GetTimeFromCurve(m_attack1Sec); }
	return 0.;
}

double E4Envelope::GetAttack2Sec() const
{
	//return E4BVariables::envelopeValues[std::clamp(m_ampEnvelope.GetAttack2Sec(), 0ui8, 126ui8)];
	if(m_attack2Sec > 0) { return GetTimeFromCurve(m_attack2Sec); }
	return 0.;
}

double E4Envelope::GetDecay1Sec() const
{
	//return E4BVariables::envelopeValues[std::clamp(m_ampEnvelope.GetDecay1Sec(), 0ui8, 126ui8)];
	if(m_decay1Sec > 0) { return GetTimeFromCurve(m_decay1Sec); }
	return 0.;
}

double E4Envelope::GetDecay2Sec() const
{
	//return E4BVariables::envelopeValues[std::clamp(m_ampEnvelope.GetDecay2Sec(), 0ui8, 126ui8)];
	if(m_decay2Sec > 0) { return GetTimeFromCurve(m_decay2Sec); }
	return 0.;
}

double E4Envelope::GetRelease1Sec() const
{
	//return E4BVariables::envelopeValues[std::clamp(m_ampEnvelope.GetRelease1Sec(), 0ui8, 126ui8)];

	if(m_release1Sec > 0) { return GetTimeFromCurve(m_release1Sec); }
	return 0.;
}

double E4Envelope::GetRelease2Sec() const
{
	//return E4BVariables::envelopeValues[std::clamp(m_ampEnvelope.GetRelease2Sec(), 0ui8, 126ui8)];
	if(m_release2Sec > 0) { return GetTimeFromCurve(m_release2Sec); }
	return 0.;
}