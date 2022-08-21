#include "Header/E4Result.h"
#include "Header/VoiceDefinitions.h"

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