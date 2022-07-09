#pragma once
#include <array>

namespace E4BVariables
{
	constexpr auto CHUNK_NAME_LEN = 4;
	constexpr auto CHUNK_SIZE = 8;
	constexpr auto CONTENT_CHUNK_DATA_LEN = 24;
	constexpr uint32_t CONTENT_CHUNK_LEN = CHUNK_SIZE + CONTENT_CHUNK_DATA_LEN;

	constexpr std::string_view EMU4_FORM_TAG = "FORM";
	constexpr std::string_view EMU4_E4_FORMAT = "E4B0";
	constexpr std::string_view EMU4_TOC_TAG = "TOC1";
	constexpr std::string_view EMU4_E4_PRESET_TAG = "E4P1";
	constexpr std::string_view EMU4_E3_SAMPLE_TAG = "E3S1";
	constexpr std::string_view EMU4_EMSt_TAG = "EMSt";
	constexpr auto EMU4_E3_SAMPLE_OFFSET = CHUNK_SIZE + 2;

	constexpr auto NAME_SIZE = 16;
	constexpr auto SAMPLE_PARAMETERS = 9;
	constexpr auto MORE_SAMPLE_PARAMETERS = 8;

	constexpr size_t EMU4_E3_SAMPLE_REDUNDANT_OFFSET = EMU4_E3_SAMPLE_OFFSET + (sizeof(char) * NAME_SIZE) + (sizeof(unsigned int) * SAMPLE_PARAMETERS) +
		sizeof(unsigned int) + sizeof(unsigned int) + (sizeof(unsigned int) * MORE_SAMPLE_PARAMETERS);

	constexpr auto STEREO_SAMPLE = 0x00700001;
	constexpr auto STEREO_SAMPLE_2 = 0x00700000;

	constexpr auto FILTER_TYPE_COUNT = 56;
	constexpr std::array<std::string_view, FILTER_TYPE_COUNT> filterTypes =
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
