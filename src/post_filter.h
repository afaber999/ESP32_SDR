
#ifndef POST_FILTER_H_INCLUDED
#define POST_FILTER_H_INCLUDED

enum POST_FILTERS
{
	CW_650_60,	
	CW_700_100,
	CW_750_200,
	CW_750_500,

	SSB_3000,
	SSB_2700,
	SSB_2500,
	SSB_1700,
};

void post_filter_select(POST_FILTERS filter);
float post_filter_sample(float sample);

#endif

