#include "Header/E4B/Data/E4Envelope.h"
#include "Header/E4B/Helpers/E4VoiceHelpers.h"
#include "Header/IO/BinaryReader.h"
#include "Header/IO/BinaryWriter.h"

float E4Envelope::GetAttack1Level() const
{
	return MathFunctions::round_f_places(E4VoiceHelpers::ConvertByteToPercentF(m_attack1Level), 2u);
}

float E4Envelope::GetAttack2Level() const
{
	return MathFunctions::round_f_places(E4VoiceHelpers::ConvertByteToPercentF(m_attack2Level), 2u);
}

float E4Envelope::GetDecay1Level() const
{
	return MathFunctions::round_f_places(E4VoiceHelpers::ConvertByteToPercentF(m_decay1Level), 2u);
}

float E4Envelope::GetDecay2Level() const
{
	return MathFunctions::round_f_places(E4VoiceHelpers::ConvertByteToPercentF(m_decay2Level), 2u);
}

float E4Envelope::GetRelease1Level() const
{
	return MathFunctions::round_f_places(E4VoiceHelpers::ConvertByteToPercentF(m_release1Level), 2u);
}

float E4Envelope::GetRelease2Level() const
{
	return MathFunctions::round_f_places(E4VoiceHelpers::ConvertByteToPercentF(m_release2Level), 2u);
}

E4Envelope::E4Envelope(const float attackSec, const float decaySec, const float holdSec, const float releaseSec, const float delaySec, const float sustainLevel)
    : E4Envelope(static_cast<double>(attackSec), static_cast<double>(decaySec), static_cast<double>(holdSec),
        static_cast<double>(releaseSec), static_cast<double>(delaySec), sustainLevel) {}

E4Envelope::E4Envelope(const double attackSec, const double decaySec, const double holdSec, const double releaseSec, const double delaySec, const float sustainLevel)
	: m_attack2Sec(E4VoiceHelpers::GetByteFromSecAttack(attackSec)), m_decay1Sec(E4VoiceHelpers::GetByteFromSecDecay1(holdSec)),
	m_decay2Sec(E4VoiceHelpers::GetByteFromSecDecay2(decaySec)), m_decay2Level(E4VoiceHelpers::ConvertPercentToByteF(sustainLevel)),
	m_release1Sec(E4VoiceHelpers::GetByteFromSecRelease(releaseSec))
{
	if(delaySec > 0.)
	{
		m_attack1Sec = E4VoiceHelpers::GetByteFromSecDecay1(delaySec);

		// This is set to 99.2% to allow for the seconds to be set, otherwise it would be ignored.
		// It can also be 100%, but then Attack2 cannot be set.
		// TODO: Attempt to get level(s) based on the time (see issue on GitHub)

		constexpr auto TEMP_ATTACK_1_LEVEL(99.2f);
		m_attack1Level = E4VoiceHelpers::ConvertPercentToByteF(TEMP_ATTACK_1_LEVEL);
	}
}

void E4Envelope::write(BinaryWriter& writer) const
{
    writer.writeType(&m_attack1Sec);
    writer.writeType(&m_attack1Level);
    writer.writeType(&m_attack2Sec);
    writer.writeType(&m_attack2Level);
    
    writer.writeType(&m_decay1Sec);
    writer.writeType(&m_decay1Level);
    writer.writeType(&m_decay2Sec);
    writer.writeType(&m_decay2Level);
    
    writer.writeType(&m_release1Sec);
    writer.writeType(&m_release1Level);
    writer.writeType(&m_release2Sec);
    writer.writeType(&m_release2Level);
}

void E4Envelope::readAtLocation(ReadLocationHandle& readHandle)
{
    readHandle.readType(&m_attack1Sec);
    readHandle.readType(&m_attack1Level);
    readHandle.readType(&m_attack2Sec);
    readHandle.readType(&m_attack2Level);

    readHandle.readType(&m_decay1Sec);
    readHandle.readType(&m_decay1Level);
    readHandle.readType(&m_decay2Sec);
    readHandle.readType(&m_decay2Level);

    readHandle.readType(&m_release1Sec);
    readHandle.readType(&m_release1Level);
    readHandle.readType(&m_release2Sec);
    readHandle.readType(&m_release2Level);
}

double E4Envelope::GetAttack1Sec() const
{
	if(m_attack1Sec > 0ui8) { return E4VoiceHelpers::GetTimeFromCurveAttack(m_attack1Sec); }
	return 0.;
}

double E4Envelope::GetAttack2Sec() const
{
	if(m_attack2Sec > 0ui8) { return E4VoiceHelpers::GetTimeFromCurveAttack(m_attack2Sec); }
	return 0.;
}

double E4Envelope::GetDecay1Sec() const
{
	if(m_decay1Sec > 0ui8) { return E4VoiceHelpers::GetTimeFromCurveDecay1(m_decay1Sec); }
	return 0.;
}

double E4Envelope::GetDecay2Sec() const
{
	if(m_decay2Sec > 0ui8) { return E4VoiceHelpers::GetTimeFromCurveDecay2(m_decay2Sec); }
	return 0.;
}

double E4Envelope::GetRelease1Sec() const
{
	if(m_release1Sec > 0ui8) { return E4VoiceHelpers::GetTimeFromCurveRelease(m_release1Sec); }
	return 0.;
}

double E4Envelope::GetRelease2Sec() const
{
	if(m_release2Sec > 0ui8) { return E4VoiceHelpers::GetTimeFromCurveRelease(m_release2Sec); }
	return 0.;
}