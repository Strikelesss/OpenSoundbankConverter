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

	constexpr std::array<std::string_view, 128> midiKeyNotes{
		"C-2", "C#-2", "D-2", "D#-2", "E-2", "F-2", "F#-2", "G-2", "G#-2", "A-2", "A#-2", "B-2",
		"C-1", "C#-1", "D-1", "D#-1", "E-1", "F-1", "F#-1", "G-1", "G#-1", "A-1", "A#-1", "B-1",
		"C0", "C#0", "D0", "D#0", "E0", "F0", "F#0", "G0", "G#0", "A0", "A#0", "B0",
		"C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1",
		"C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2",
		"C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A3", "A#3", "B3",
		"C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4",
		"C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5", "A#5", "B5",
		"C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6", "A6", "A#6", "B6",
		"C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7", "A7", "A#7", "B7",
		"C8", "C#8", "D8", "D#8", "E8", "F8", "F#8", "G8"
	};

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