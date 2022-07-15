#include "Header/E4Preset.h"
#include "Header/VoiceDefinitions.h"

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

float E4Voice::GetAttack1Level() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_attack1Level);
}

float E4Voice::GetAttack2Level() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_attack2Level);
}

float E4Voice::GetDecay1Level() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_decay1Level);
}

float E4Voice::GetDecay2Level() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_decay2Level);
}

float E4Voice::GetRelease1Level() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_release1Level);
}

float E4Voice::GetRelease2Level() const
{
	return VoiceDefinitions::GetBottomSectionPercent(m_release2Level);
}