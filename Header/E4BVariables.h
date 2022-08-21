#pragma once
#include <array>
#include <string_view>

namespace E4BVariables
{
	constexpr auto EOS_CHUNK_NAME_LEN = 4ull;
	constexpr auto EOS_CHUNK_SIZE = EOS_CHUNK_NAME_LEN + sizeof(uint32_t);
	constexpr auto EOS_CHUNK_DATA_LEN = 24u;
	constexpr auto EOS_CHUNK_TOTAL_LEN = EOS_CHUNK_SIZE + EOS_CHUNK_DATA_LEN;
	constexpr auto EOS_CHUNK_NAME_OFFSET = EOS_CHUNK_SIZE + sizeof(uint16_t);

	constexpr std::string_view EOS_FORM_TAG = "FORM";
	constexpr std::string_view EOS_E4_FORMAT_TAG = "E4B0";
	constexpr std::string_view EOS_TOC_TAG = "TOC1";
	constexpr std::string_view EOS_E4_PRESET_TAG = "E4P1";
	constexpr std::string_view EOS_E3_SAMPLE_TAG = "E3S1";
	constexpr std::string_view EOS_E4_SEQ_TAG = "E4s1";
	constexpr std::string_view EOS_EMSt_TAG = "EMSt";

	constexpr auto EOS_E4_MAX_NAME_LEN = 16u;
	constexpr auto NUM_SAMPLE_PARAMETERS = 9u;
	constexpr auto NUM_EXTRA_SAMPLE_PARAMETERS = 8u;

	constexpr size_t EOS_E3_SAMPLE_REDUNDANT_OFFSET = EOS_CHUNK_NAME_OFFSET + EOS_E4_MAX_NAME_LEN + sizeof(uint32_t) * NUM_SAMPLE_PARAMETERS +
		sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) * NUM_EXTRA_SAMPLE_PARAMETERS;

	constexpr std::array<std::string_view, 56> filterTypes =
	{
		"No Filter", "2 Pole Lowpass", "4 Pole Lowpass", "6 Pole Lowpass",
		"2 Pole Highpass", "4 Pole Highpass", "2 Pole Bandpass", "4 Pole Bandpass",
		"Contrary Bandpass", "Swept EQ 1 Octave", "Swept EQ 2/1 Octave", "Swept EQ 3/1 Octave",
		"Phaser 1", "Phaser 2", "Bat Phaser", "Flanger Lite",
		"Vocal Ah-Ay-Ee", "Vocal Oo-Ah", "Dual EQ Morph", "Dual EQ + LP Morph",
		"Dual EQ Morph/Expression", "Peak/Shelf Morph", "Morph Designer", "Ace of Bass",
		"MegaSweepz", "Early Rizer", "Millennium", "Meaty Gizmo",
		"Klub Klassik", "BassBox 303", "Fuzzi Face", "Dead Ringer",
		"TB or Not TB", "Ooh to Eee", "Boland Bass", "Multi Q Vox",
		"Talking Hedz", "Zoom Peaks", "DJ Alkaline", "Bass Tracer",
		"Rogue Hertz", "Razor Blades", "Radio Craze", "Eeh to Aah",
		"Ubu Orator", "Deep Bouche", "Freak Shifta", "Cruz Pusher",
		"Angelz Hairz", "Dream Weava", "Acid Ravage", "Bass-O-Matic",
		"Lucifer's Q", "Tooth Comb", "Ear Bender", "Klang Kling"
	};
}