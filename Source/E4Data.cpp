#include "Header/E4Data.h"
#include "Header/VoiceDefinitions.h"
#include "Header/BinaryWriter.h"
#include "Header/MathFunctions.h"

E4LFO::E4LFO(const double rate, const uint8_t shape, const double delay, const bool keySync) : m_rate(VoiceDefinitions::GetByteFromLFORate(rate)),
	m_shape(shape), m_delay(VoiceDefinitions::GetByteFromLFODelay(delay)), m_keySync(!keySync) {}

bool E4LFO::write(BinaryWriter& writer)
{
	return writer.writeType(&m_rate) && writer.writeType(&m_shape) && writer.writeType(&m_delay) && writer.writeType(&m_variation)
		&& writer.writeType(&m_keySync) && writer.writeType(m_possibleRedundant1.data(), sizeof(int8_t) * m_possibleRedundant1.size());
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
	return MathFunctions::round_f_places(VoiceDefinitions::ConvertByteToPercentF(m_chorusAmount), 2u);
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
	return MathFunctions::round_f_places(VoiceDefinitions::ConvertByteToPercentF(m_filterQ), 1u);
}

double E4Voice::GetKeyDelay() const
{
	return static_cast<double>(_byteswap_ushort(m_keyDelay)) / 1000.;
}

std::string_view E4Voice::GetFilterType() const
{
	return VoiceDefinitions::GetFilterTypeFromByte(m_filterType);
}

bool E4VoiceNoteData::write(BinaryWriter& writer)
{
	return writer.writeType(&m_low) && writer.writeType(&m_lowFade) && writer.writeType(&m_highFade) && writer.writeType(&m_high);
}

E4Voice::E4Voice(const float chorusWidth, const float chorusAmount, const uint16_t filterFreq, const int8_t coarseTune, const int8_t pan, const int8_t volume, const double fineTune, const float keyDelay, const float filterQ,
	const std::pair<uint8_t, uint8_t> zone, const std::pair<uint8_t, uint8_t> velocity, E4Envelope&& ampEnv, E4Envelope&& filterEnv, const E4LFO lfo1) : m_keyData(zone.first, zone.second), m_velData(velocity.first, velocity.second),
	m_keyDelay(_byteswap_ushort(static_cast<uint16_t>(keyDelay * 1000.f))), m_coarseTune(coarseTune), m_fineTune(VoiceDefinitions::ConvertFineTuneToByte(fineTune)), m_chorusWidth(VoiceDefinitions::ConvertChorusWidthToByte(chorusWidth)),
	m_chorusAmount(VoiceDefinitions::ConvertPercentToByteF(chorusAmount)), m_volume(volume), m_pan(pan), m_filterFrequency(VoiceDefinitions::ConvertFilterFrequencyToByte(filterFreq)),
	m_filterQ(VoiceDefinitions::ConvertPercentToByteF(filterQ)), m_ampEnv(ampEnv), m_filterEnv(filterEnv), m_lfo1(lfo1) {}

bool E4Voice::write(BinaryWriter& writer)
{
	const uint16_t totalVoiceSize(_byteswap_ushort(static_cast<uint16_t>(VOICE_DATA_SIZE + VOICE_END_DATA_SIZE)));
	if (writer.writeType(&totalVoiceSize) && writer.writeType(&m_zone) && writer.writeType(&m_group) && writer.writeType(&m_amplifierData, sizeof(int8_t) * m_amplifierData.size())
		&& m_keyData.write(writer) && m_velData.write(writer) && m_rtData.write(writer) && writer.writeType(&m_possibleRedundant1) && writer.writeType(&m_keyAssignGroup) && writer.writeType(&m_keyDelay) && writer.writeType(m_possibleRedundant2.data(), sizeof(int8_t) * m_possibleRedundant2.size()) && writer.writeType(&m_sampleOffset)
		&& writer.writeType(&m_transpose) && writer.writeType(&m_coarseTune) && writer.writeType(&m_fineTune) && writer.writeType(&m_glideRate) && writer.writeType(&m_fixedPitch) && writer.writeType(&m_keyMode) && writer.writeType(&m_possibleRedundant3)
		&& writer.writeType(&m_chorusWidth) && writer.writeType(&m_chorusAmount) && writer.writeType(m_possibleRedundant4.data(), sizeof(int8_t) * m_possibleRedundant4.size()) && writer.writeType(&m_volume)
		&& writer.writeType(&m_pan) && writer.writeType(&m_possibleRedundant5) && writer.writeType(&m_ampEnvDynRange))
	{
		// If the filter frequency isn't 20,000, pick 4 Pole Lowpass, if it is, pick No Filter if there's no filter resonance or realtime filter modifications:
		
		bool hasFilter(m_filterFrequency < 255ui8 || m_filterQ > 0ui8);
		if(!hasFilter)
		{
			for(const auto& cord : m_cords)
			{
				const auto dest(cord.GetDest());
				if((dest == FILTER_FREQ || dest == FILTER_RES) && cord.GetAmount() != 0ui8) { hasFilter = true; break; }
			}
		}

		const auto filterType(hasFilter ? 0ui8 : 127ui8);

		if (writer.writeType(&filterType) && writer.writeType(&m_possibleRedundant6) && writer.writeType(&m_filterFrequency) && writer.writeType(&m_filterQ) &&
			writer.writeType(m_possibleRedundant7.data(), sizeof(int8_t) * m_possibleRedundant7.size()))
		{
			if (m_ampEnv.write(writer) && writer.writeType(m_possibleRedundant8.data(), sizeof(int8_t) * m_possibleRedundant8.size())
				&& m_filterEnv.write(writer) && writer.writeType(m_possibleRedundant9.data(), sizeof(int8_t) * m_possibleRedundant9.size())
				&& m_auxEnv.write(writer) && writer.writeType(m_possibleRedundant10.data(), sizeof(int8_t) * m_possibleRedundant10.size())
				&& m_lfo1.write(writer) && m_lfo2.write(writer) && writer.writeType(m_possibleRedundant11.data(), sizeof(int8_t) * m_possibleRedundant11.size())
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
		if(cord.GetSource() == SRC_OFF && cord.GetDest() == DST_OFF)
		{
			cord.SetAmount(static_cast<int8_t>(amount));
			cord.SetSrc(src);
			cord.SetDst(dst);
			break;
		}
	}
}

void E4Voice::DisableCord(const uint8_t src, const uint8_t dst)
{
    for(auto& cord : m_cords)
    {
        if(cord.GetSource() == src && cord.GetDest() == dst)
        {
            cord.SetSrc(SRC_OFF);
            break;
        }
    }
}

bool E4EMSt::write(BinaryWriter& writer)
{
	const uint32_t emstLen(_byteswap_ulong(TOTAL_EMST_DATA_SIZE));
	return writer.writeType(&emstLen) && writer.writeType(m_possibleRedundant1.data(), sizeof(int8_t) * m_possibleRedundant1.size())
		&& writer.writeType(m_name.data(), sizeof(char) * m_name.size()) && writer.writeType(m_possibleRedundant2.data(), sizeof(int8_t) * m_possibleRedundant2.size())
		&& writer.writeType(&m_currentPreset) && writer.writeType(m_midiChannels.data(), sizeof(E4MIDIChannel) * m_midiChannels.size())
		&& writer.writeType(m_possibleRedundant3.data(), sizeof(uint8_t) * m_possibleRedundant3.size()) && writer.writeType(&m_tempo)
		&& writer.writeType(m_possibleRedundant4.data(), sizeof(uint8_t) * m_possibleRedundant4.size());
}

float E4Envelope::GetAttack1Level() const
{
	return MathFunctions::round_f_places(VoiceDefinitions::ConvertByteToPercentF(m_attack1Level), 2u);
}

float E4Envelope::GetAttack2Level() const
{
	return MathFunctions::round_f_places(VoiceDefinitions::ConvertByteToPercentF(m_attack2Level), 2u);
}

float E4Envelope::GetDecay1Level() const
{
	return MathFunctions::round_f_places(VoiceDefinitions::ConvertByteToPercentF(m_decay1Level), 2u);
}

float E4Envelope::GetDecay2Level() const
{
	return MathFunctions::round_f_places(VoiceDefinitions::ConvertByteToPercentF(m_decay2Level), 2u);
}

float E4Envelope::GetRelease1Level() const
{
	return MathFunctions::round_f_places(VoiceDefinitions::ConvertByteToPercentF(m_release1Level), 2u);
}

float E4Envelope::GetRelease2Level() const
{
	return MathFunctions::round_f_places(VoiceDefinitions::ConvertByteToPercentF(m_release2Level), 2u);
}

bool E4Envelope::write(BinaryWriter& writer)
{
	return writer.writeType(&m_attack1Sec) && writer.writeType(&m_attack1Level) && writer.writeType(&m_attack2Sec) && writer.writeType(&m_attack2Level)
		&& writer.writeType(&m_decay1Sec) && writer.writeType(&m_decay1Level) && writer.writeType(&m_decay2Sec) && writer.writeType(&m_decay2Level)
		&& writer.writeType(&m_release1Sec) && writer.writeType(&m_release1Level) && writer.writeType(&m_release2Sec) && writer.writeType(&m_release2Level);
}

E4Envelope::E4Envelope(const float attackSec, const float decaySec, const float holdSec, const float releaseSec, const float delaySec, const float sustainLevel)
    : E4Envelope(static_cast<double>(attackSec), static_cast<double>(decaySec), static_cast<double>(holdSec),
        static_cast<double>(releaseSec), static_cast<double>(delaySec), sustainLevel) {}

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

		constexpr auto TEMP_ATTACK_1_LEVEL(99.2f);
		m_attack1Level = VoiceDefinitions::ConvertPercentToByteF(TEMP_ATTACK_1_LEVEL);
	}
}

double E4VoiceEndData::GetFineTune() const
{
	return VoiceDefinitions::ConvertByteToFineTune(m_fineTune);
}

bool E4VoiceEndData::write(BinaryWriter& writer)
{
	constexpr std::array<int8_t, 6> redundant{};
	return m_keyData.write(writer) && m_velData.write(writer) && writer.writeType(&m_possibleRedundant1)
		&& writer.writeType(&m_sampleIndex) && writer.writeType(&m_possibleRedundant2) && writer.writeType(&m_fineTune)
		&& writer.writeType(&m_originalKey) && writer.writeType(&m_volume) && writer.writeType(&m_pan)
		&& writer.writeType(&m_possibleRedundant3) && writer.writeType(redundant.data(), sizeof(int8_t) * redundant.size());
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