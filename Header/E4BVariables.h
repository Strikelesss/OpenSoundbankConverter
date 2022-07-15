#pragma once
#include <array>
#include <string_view>

namespace E4BVariables
{
	constexpr auto CHUNK_NAME_LEN = 4u;
	constexpr auto CHUNK_SIZE = 8u;
	constexpr auto CONTENT_CHUNK_DATA_LEN = 24u;
	constexpr auto CONTENT_CHUNK_LEN = CHUNK_SIZE + CONTENT_CHUNK_DATA_LEN;
	constexpr auto CHUNK_NAME_OFFSET = 10ull;

	constexpr std::string_view EMU4_FORM_TAG = "FORM";
	constexpr std::string_view EMU4_E4_FORMAT_TAG = "E4B0";
	constexpr std::string_view EMU4_TOC_TAG = "TOC1";
	constexpr std::string_view EMU4_E4_PRESET_TAG = "E4P1";
	constexpr std::string_view EMU4_E3_SAMPLE_TAG = "E3S1";
	constexpr std::string_view EMU4_E4_SEQ_TAG = "E4s1";
	constexpr std::string_view EMU4_EMSt_TAG = "EMSt";
	constexpr auto EMU4_E3_SAMPLE_OFFSET = CHUNK_SIZE + 2u;

	constexpr auto NAME_SIZE = 16u;
	constexpr auto NUM_SAMPLE_PARAMETERS = 9u;
	constexpr auto NUM_EXTRA_SAMPLE_PARAMETERS = 8u;

	constexpr size_t EMU4_E3_SAMPLE_REDUNDANT_OFFSET = EMU4_E3_SAMPLE_OFFSET + NAME_SIZE + (sizeof(unsigned int) * NUM_SAMPLE_PARAMETERS) +
		sizeof(unsigned int) + sizeof(unsigned int) + (sizeof(unsigned int) * NUM_EXTRA_SAMPLE_PARAMETERS);

	constexpr auto STEREO_SAMPLE = 0x00700001;
	constexpr auto STEREO_SAMPLE_2 = 0x00700000;

	constexpr auto FILTER_TYPE_COUNT = 56u;
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