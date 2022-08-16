#include "Header/E4Preset.h"
#include "Header/VoiceDefinitions.h"
#include "Header/BinaryWriter.h"
#include <cmath>

E4LFO::E4LFO(const double rate, const uint8_t shape, const double delay, const bool keySync) : m_rate(VoiceDefinitions::GetByteFromLFORate(rate)),
	m_shape(shape), m_delay(VoiceDefinitions::GetByteFromLFODelay(delay)), m_keySync(!keySync) {}

bool E4LFO::write(BinaryWriter& writer)
{
	constexpr std::array<int8_t, 3> redundant{};
	return writer.writeType(&m_rate) && writer.writeType(&m_shape) && writer.writeType(&m_delay) && writer.writeType(&m_variation)
		&& writer.writeType(&m_keySync) && writer.writeType(redundant.data(), sizeof(int8_t) * redundant.size());
}

double E4LFO::GetRate() const
{
	return VoiceDefinitions::GetLFORateFromByte(m_rate);
}

double E4LFO::GetDelay() const
{
	return VoiceDefinitions::GetLFODelayFromByte(m_delay);
}

float E4Voice::GetChorusWidth() const
{
	return VoiceDefinitions::GetChorusWidthPercent(m_chorusWidth);
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

float E4Voice::GetFilterQ() const
{
	return VoiceDefinitions::ConvertByteToFilterQ(m_filterQ);
}

double E4Voice::GetKeyDelay() const
{
	return static_cast<double>(_byteswap_ushort(m_keyDelay)) / 1000.;
}

std::string_view E4Voice::GetFilterType() const
{
	return VoiceDefinitions::GetFilterTypeFromByte(m_filterType);
}

E4Voice::E4Voice(const float chorusWidth, const float chorusAmount, const uint16_t filterFreq, const int8_t coarseTune, const int8_t pan, const int8_t volume, const double fineTune, const double keyDelay, const float filterQ,
	const std::pair<uint8_t, uint8_t> zone, const std::pair<uint8_t, uint8_t> velocity, E4Envelope&& ampEnv, E4Envelope&& filterEnv, const E4LFO lfo1) : m_lowZone(zone.first), m_highZone(zone.second), m_minVelocity(velocity.first), m_maxVelocity(velocity.second),
	m_keyDelay(_byteswap_ushort(static_cast<uint16_t>(keyDelay * 1000.))), m_coarseTune(coarseTune), m_fineTune(VoiceDefinitions::ConvertFineTuneToByte(fineTune)), m_chorusWidth(VoiceDefinitions::ConvertChorusWidthToByte(chorusWidth)),
	m_chorusAmount(VoiceDefinitions::ConvertPercentToByteF(chorusAmount)), m_volume(volume), m_pan(pan), m_filterFrequency(VoiceDefinitions::ConvertFilterFrequencyToByte(filterFreq)),
	m_filterQ(VoiceDefinitions::ConvertPercentToByteF(filterQ)), m_ampEnv(ampEnv), m_filterEnv(filterEnv), m_lfo1(lfo1) {}

bool E4Voice::write(BinaryWriter& writer)
{
	const uint16_t totalVoiceSize(_byteswap_ushort(static_cast<uint16_t>(VOICE_DATA_SIZE + VOICE_END_DATA_SIZE)));
	if (writer.writeType(&totalVoiceSize) && writer.writeType(&m_possibleRedundant1) && writer.writeType(&m_group) && writer.writeType(&m_possibleRedundant2, sizeof(int8_t) * m_possibleRedundant2.size())
		&& writer.writeType(&m_lowZone) && writer.writeType(&m_lowFade) && writer.writeType(&m_highFade) && writer.writeType(&m_highZone) && writer.writeType(&m_minVelocity) && writer.writeType(m_possibleRedundant3.data(), sizeof(int8_t) * m_possibleRedundant3.size()) && writer.writeType(&m_maxVelocity)
		&& writer.writeType(m_possibleRedundant4.data(), sizeof(int8_t) * m_possibleRedundant4.size()) && writer.writeType(&m_keyDelay) && writer.writeType(m_possibleRedundant5.data(), sizeof(int8_t) * m_possibleRedundant5.size()) && writer.writeType(&m_sampleOffset)
		&& writer.writeType(&m_transpose) && writer.writeType(&m_coarseTune) && writer.writeType(&m_fineTune) && writer.writeType(&m_possibleRedundant6) && writer.writeType(&m_fixedPitch) && writer.writeType(m_possibleRedundant7.data(), sizeof(int8_t) * m_possibleRedundant7.size())
		&& writer.writeType(&m_chorusWidth) && writer.writeType(&m_chorusAmount) && writer.writeType(m_possibleRedundant8.data(), sizeof(int8_t) * m_possibleRedundant8.size()) && writer.writeType(&m_volume)
		&& writer.writeType(&m_pan) && writer.writeType(m_possibleRedundant9.data(), sizeof(int8_t) * m_possibleRedundant9.size()))
	{
		// If the filter frequency isn't 20,000, pick 4 Pole Lowpass, if it is, pick No Filter if it has no filter resonance
		const auto filterType(m_filterFrequency < 255ui8 ? 0ui8 : m_filterQ > 0ui8 ? 0ui8 : 127ui8);

		if (writer.writeType(&filterType) && writer.writeType(&m_possibleRedundant10) && writer.writeType(&m_filterFrequency) && writer.writeType(&m_filterQ) &&
			writer.writeType(m_possibleRedundant11.data(), sizeof(int8_t) * m_possibleRedundant11.size()))
		{
			if (m_ampEnv.write(writer) && writer.writeType(m_possibleRedundant12.data(), sizeof(int8_t) * m_possibleRedundant12.size())
				&& m_filterEnv.write(writer) && writer.writeType(m_possibleRedundant13.data(), sizeof(int8_t) * m_possibleRedundant13.size())
				&& m_auxEnv.write(writer) && writer.writeType(m_possibleRedundant14.data(), sizeof(int8_t) * m_possibleRedundant14.size())
				&& m_lfo1.write(writer) && m_lfo2.write(writer) && writer.writeType(m_possibleRedundant15.data(), sizeof(int8_t) * m_possibleRedundant15.size())
				&& writer.writeType(m_cords.data(), sizeof(E4Cord) * m_cords.size()))
			{
				return true;
			}
		}
	}

	return false;
}

void E4Voice::ReplaceOrAddCord(const uint8_t src, const uint8_t dst, const float amount)
{
	// Replace if existing
	for(auto& cord : m_cords)
	{
		if(cord.GetSource() == src && cord.GetDest() == dst)
		{
			cord.SetAmount(static_cast<int8_t>(amount));
			return;
		}
	}

	// Replace null cord
	for(auto& cord : m_cords)
	{
		if(cord.GetSource() == 0ui8 && cord.GetDest() == 0ui8)
		{
			cord.SetAmount(static_cast<int8_t>(amount));
			cord.SetSrc(src);
			cord.SetDst(dst);
			break;
		}
	}
}

bool E4EMSt::write(BinaryWriter& writer)
{
	const uint32_t emstLen(_byteswap_ulong(TOTAL_EMST_DATA_SIZE));
	return writer.writeType(&emstLen) && writer.writeType(m_possibleRedundant1.data(), sizeof(int8_t) * m_possibleRedundant1.size())
		&& writer.writeType(m_name.data(), sizeof(char) * m_name.size()) && writer.writeType(m_possibleRedundant2.data(), sizeof(int8_t) * m_possibleRedundant2.size())
		&& writer.writeType(&m_currentPreset) && writer.writeType(m_possibleRedundant3.data(), sizeof(int8_t) * m_possibleRedundant3.size());
}

bool E4VoiceResult::GetAmountFromCord(const uint8_t src, const uint8_t dst, float& outAmount) const
{
	for(const auto& cord : m_cords)
	{
		if(cord.GetSource() == src && cord.GetDest() == dst)
		{
			const auto amount(cord.GetAmount());
			if(amount != 0i8)
			{
				outAmount = std::roundf(VoiceDefinitions::ConvertByteToPercentF(cord.GetAmount()));
				return true;
			}
		}
	}

	return false;
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

bool E4Envelope::write(BinaryWriter& writer)
{
	return writer.writeType(&m_attack1Sec) && writer.writeType(&m_attack1Level) && writer.writeType(&m_attack2Sec) && writer.writeType(&m_attack2Level)
		&& writer.writeType(&m_decay1Sec) && writer.writeType(&m_decay1Level) && writer.writeType(&m_decay2Sec) && writer.writeType(&m_decay2Level)
		&& writer.writeType(&m_release1Sec) && writer.writeType(&m_release1Level) && writer.writeType(&m_release2Sec) && writer.writeType(&m_release2Level);
}

E4Envelope::E4Envelope(const double attackSec, const double decaySec, const double holdSec, const double releaseSec, const double delaySec, const float sustainLevel)
	: m_attack2Sec(VoiceDefinitions::GetByteFromSecAttack(attackSec)), m_decay1Sec(VoiceDefinitions::GetByteFromSecDecay1(holdSec)),
	m_decay2Sec(VoiceDefinitions::GetByteFromSecDecay2(decaySec)), m_decay2Level(VoiceDefinitions::ConvertPercentToByteF(sustainLevel)),
	m_release1Sec(VoiceDefinitions::GetByteFromSecRelease(releaseSec))
{
	if(delaySec > 0.)
	{
		m_attack1Sec = VoiceDefinitions::GetByteFromSecDecay1(delaySec);

		// This is set to 99.2% to allow for the seconds to be set, otherwise it would be ignored.
		// It can also be 100%, but then Attack2 cannot be set.
		// TODO: attempt to get level(s) based on the time (see issue on GitHub)
		m_attack1Level = VoiceDefinitions::ConvertPercentToByteF(99.2f);
	}
}

double E4VoiceEndData::GetFineTune() const
{
	return VoiceDefinitions::ConvertByteToFineTune(m_fineTune);
}

bool E4VoiceEndData::write(BinaryWriter& writer)
{
	constexpr std::array<int8_t, 6> redundant{};
	return writer.writeType(&m_lowZone) && writer.writeType(m_possibleRedundant1.data(), sizeof(int8_t) * m_possibleRedundant1.size())
		&& writer.writeType(&m_highZone) && writer.writeType(m_possibleRedundant2.data(), sizeof(int8_t) * m_possibleRedundant2.size())
		&& writer.writeType(&m_sampleIndex) && writer.writeType(&m_possibleRedundant3) && writer.writeType(&m_fineTune)
		&& writer.writeType(&m_originalKey) && writer.writeType(&m_volume) && writer.writeType(&m_pan) && writer.writeType(&m_possibleRedundant4)
		&& writer.writeType(redundant.data(), sizeof(int8_t) * redundant.size());
}

double E4Envelope::GetAttack1Sec() const
{
	if(m_attack1Sec > 0ui8) { return VoiceDefinitions::GetTimeFromCurveAttack(m_attack1Sec); }
	return 0.;
}

double E4Envelope::GetAttack2Sec() const
{
	if(m_attack2Sec > 0ui8) { return VoiceDefinitions::GetTimeFromCurveAttack(m_attack2Sec); }
	return 0.;
}

double E4Envelope::GetDecay1Sec() const
{
	if(m_decay1Sec > 0ui8) { return VoiceDefinitions::GetTimeFromCurveDecay1(m_decay1Sec); }
	return 0.;
}

double E4Envelope::GetDecay2Sec() const
{
	if(m_decay2Sec > 0ui8) { return VoiceDefinitions::GetTimeFromCurveDecay2(m_decay2Sec); }
	return 0.;
}

double E4Envelope::GetRelease1Sec() const
{
	if(m_release1Sec > 0ui8) { return VoiceDefinitions::GetTimeFromCurveRelease(m_release1Sec); }
	return 0.;
}

double E4Envelope::GetRelease2Sec() const
{
	if(m_release2Sec > 0ui8) { return VoiceDefinitions::GetTimeFromCurveRelease(m_release2Sec); }
	return 0.;
}