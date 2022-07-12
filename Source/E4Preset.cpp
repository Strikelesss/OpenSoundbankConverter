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