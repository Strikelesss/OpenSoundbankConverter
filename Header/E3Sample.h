#pragma once

struct E3Sample final
{
	char name[E4BVariables::NAME_SIZE];
	unsigned int parameters[E4BVariables::SAMPLE_PARAMETERS];
	unsigned int sample_rate;
	unsigned int format;
	unsigned int more_parameters[E4BVariables::MORE_SAMPLE_PARAMETERS];
	short int frames[];
};