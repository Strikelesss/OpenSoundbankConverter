#include "Header/E4Preset.h"
#include "Header/VoiceDefinitions.h"

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

double E4Envelope::GetAttack1Sec() const
{
	//return E4BVariables::envelopeValues[std::clamp(m_ampEnvelope.GetAttack1Sec(), 0ui8, 126ui8)];
	return 1.;
}

double E4Envelope::GetAttack2Sec() const
{
	//return E4BVariables::envelopeValues[std::clamp(m_ampEnvelope.GetAttack2Sec(), 0ui8, 126ui8)];
	return 1.;
}

double E4Envelope::GetDecay1Sec() const
{
	//return E4BVariables::envelopeValues[std::clamp(m_ampEnvelope.GetDecay1Sec(), 0ui8, 126ui8)];
	return 1.;
}

double E4Envelope::GetDecay2Sec() const
{
	//return E4BVariables::envelopeValues[std::clamp(m_ampEnvelope.GetDecay2Sec(), 0ui8, 126ui8)];
	return 1.;
}

double E4Envelope::GetRelease1Sec() const
{
	//return E4BVariables::envelopeValues[std::clamp(m_ampEnvelope.GetRelease1Sec(), 0ui8, 126ui8)];
	//return 1.;

	return 1.;
}

double E4Envelope::GetRelease2Sec() const
{
	//return E4BVariables::envelopeValues[std::clamp(m_ampEnvelope.GetRelease2Sec(), 0ui8, 126ui8)];
	return 1.;
}