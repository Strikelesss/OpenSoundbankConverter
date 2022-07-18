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

	constexpr auto E4_MAX_NAME_LEN = 16u;
	constexpr auto NUM_SAMPLE_PARAMETERS = 9u;
	constexpr auto NUM_EXTRA_SAMPLE_PARAMETERS = 8u;

	constexpr size_t EMU4_E3_SAMPLE_REDUNDANT_OFFSET = EMU4_E3_SAMPLE_OFFSET + E4_MAX_NAME_LEN + (sizeof(unsigned int) * NUM_SAMPLE_PARAMETERS) +
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

	/*
	// TODO: reformat and figure out how to calculate without array
	constexpr std::array<double, 127> envelopeValues =
	{
		0, 0.005, 0.09, 0.014, 0.019, 0.021, 0.023, 0.026, 0.028, 0.03, 0.033, 0.035, 0.042,
		0.044, 0.049, 0.051, 0.054, 0.058, 0.065, 0.068, 0.074, 0.077, 0.084, 0.089, 0.095,
		0.1, 0.109, 0.114,
		0.123,
		0.131,
		0.14,
		0.149,
		0.159,
		0.168,
		0.177,
		0.189,
		0.201,
		0.215,
		0.227,
		0.241,
		0.255,
		0.271,
		0.288,
		0.306,
		0.325,
		0.342,
		0.363,
		0.384,
		0.407,
		0.431,
		0.456,
		0.483,
		0.513,
		0.543,
		0.575,
		0.609,
		0.644,
		0.68,
		0.72,
		0.763,
		0.807,
		0.854,
		0.904,
		0.956,
		1.012,
		1.069,
		1.133,
		1.198,
		1.267,
		1.34,
		1.418,
		1.499,
		1.586,
		1.677,
		1.772,
		1.874,
		1.982,
		2.096,
		2.217,
		2.345,
		2.481,
		2.621,
		2.773,
		2.934,
		3.104,
		3.282,
		3.472,
		3.673,
		3.885,
		4.113,
		4.354,
		4.609,
		4.88,
		5.167,
		5.471,
		5.795,
		6.141,
		6.51,
		6.902,
		7.32,
		7.766,
		8.243,
		8.753,
		9.299,
		9.883,
		10.512,
		11.189,
		11.917,
		12.703,
		13.554,
		14.476,
		15.479,
		16.571,
		17.766,
		19.077,
		20.522,
		22.121,
		23.899,
		25.891,
		28.139,
		30.695,
		33.625,
		37.021,
		41.015,
		45.802,
		51.653,
		58.992,
	};
	*/
}