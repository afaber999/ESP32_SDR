#include <cstdint>
#include <cassert>
#include "post_filter.h"

#define IIR_SECTS  (3)

static float frx1[IIR_SECTS];
static float frx2[IIR_SECTS];
static float fry1[IIR_SECTS];
static float fry2[IIR_SECTS];

static const float* c_a0;
static const float* c_a1;
static const float* c_a2;
static const float* c_b0;
static const float* c_b1;
static const float* c_b2;

// 3 pole elliptic filter
// fs = 44100 Hz, fc = 650, bw = 60  
static const float c_a0_650_60[IIR_SECTS] = { 1.000000000000000000f, 1.000000000000000000f, 1.000000000000000000f };
static const float c_a1_650_60[IIR_SECTS] = {-1.985632592261582060f,-1.989228203463019760f,-1.987896285705462280f };
static const float c_a2_650_60[IIR_SECTS] = { 0.994190538694407211f, 0.997246448470034630f, 0.997057320611459463f };
static const float c_b0_650_60[IIR_SECTS] = { 0.002904730652796400f, 0.144531693541046891f, 0.144504283095276326f };
static const float c_b1_650_60[IIR_SECTS] = { 0.000000000000000000f,-0.288312778318129514f,-0.286959973743163788f };
static const float c_b2_650_60[IIR_SECTS] = {-0.002904730652796400f, 0.144531693541046891f, 0.144504283095276326f };

// 3 pole elliptic filter
// fs = 44100 Hz, fc = 700, bw = 100  
static const float c_a0_700_100[IIR_SECTS] = { 1.000000000000000000f, 1.000000000000000000f, 1.000000000000000000f };
static const float c_a1_700_100[IIR_SECTS] = {-1.980166439561481750f,-1.986440605178366290f,-1.983825180252146490f };
static const float c_a2_700_100[IIR_SECTS] = { 0.990089264025892923f, 0.995391778633943014f, 0.994880682980767372f };
static const float c_b0_700_100[IIR_SECTS] = { 0.004955367987053496f, 0.144484679958274298f, 0.144410492594602813f };
static const float c_b1_700_100[IIR_SECTS] = { 0.000000000000000000f,-0.288312107725802902f,-0.285669090021633321f };
static const float c_b2_700_100[IIR_SECTS] = {-0.004955367987053496f, 0.144484679958274298f, 0.144410492594602813f };

// 3 pole elliptic filter
// fs = 44100 Hz, fc = 750, bw = 200
static const float c_a0_750_200[IIR_SECTS] = { 1.000000000000000000f, 1.000000000000000000f, 1.000000000000000000f };
static const float c_a1_750_200[IIR_SECTS] = {-1.969139346639576130f,-1.981977737742514290f,-1.975643685830563930f };
static const float c_a2_750_200[IIR_SECTS] = { 0.980426210717733504f, 0.991305248133998673f, 0.989437481052324985f };
static const float c_b0_750_200[IIR_SECTS] = { 0.009786894641133265f, 0.144574615202922308f, 0.144302215044010612f };
static const float c_b1_750_200[IIR_SECTS] = { 0.000000000000000000f,-0.288735620070850629f,-0.282092593831271887f };
static const float c_b2_750_200[IIR_SECTS] = {-0.009786894641133265f, 0.144574615202922280f, 0.144302215044010612f };


// 3 pole elliptic filter
// fs = 44100 Hz, fc = 750, bw = 500
static const float c_a0_750_500[IIR_SECTS] = { 1.000000000000000000f, 1.000000000000000000f, 1.000000000000000000f };
static const float c_a1_750_500[IIR_SECTS] = {-1.940752794281249470f,-1.974860707067195300f,-1.952287880007184780f };
static const float c_a2_750_500[IIR_SECTS] = { 0.951876950139360134f, 0.981794947242693006f, 0.970568026340965839f };
static const float c_b0_750_500[IIR_SECTS] = { 0.024061524930319975f, 0.146558504762332897f, 0.144882593977638918f };
static const float c_b1_750_500[IIR_SECTS] = { 0.000000000000000000f,-0.293011634279705724f,-0.264583473888029719f };
static const float c_b2_750_500[IIR_SECTS] = {-0.024061524930319975f, 0.146558504762332925f, 0.144882593977638918f };


//SSB_1650_1350,
//SSB_1750_1250,
//SSB_2150_850,

// 3 pole elliptic filter LPF
// fs = 44100 Hz, bw = 3000
static const float c_a0_3000[IIR_SECTS] = {  1.000000000000000000f, 1.000000000000000000f, 1.000000000000000000f };
static const float c_a1_3000[IIR_SECTS] = { -1.537279588054798700f,-1.697101250394967840f,-1.790225661577712790f };
static const float c_a2_3000[IIR_SECTS] = {  0.615137738310341575f, 0.838354277199815812f, 0.963899533747554660f };
static const float c_b0_3000[IIR_SECTS] = {  0.352196979602954596f, 0.455409852435663676f, 0.137672400201695244f };
static const float c_b1_3000[IIR_SECTS] = { -0.626535808950366424f,-0.769566678066479493f,-0.101670928233548660f };
static const float c_b2_3000[IIR_SECTS] = {  0.352196979602954596f, 0.455409852435663676f, 0.137672400201695244f };
  
// 3 pole elliptic filter LPF
// fs = 44100 Hz, bw = 2700
static const float c_a0_2700[IIR_SECTS] = { 1.000000000000000000f, 1.000000000000000000f, 1.000000000000000000f };
static const float c_a1_2700[IIR_SECTS] = {-1.581810596595366380f,-1.736956096442083600f,-1.825734085680839060f };
static const float c_a2_2700[IIR_SECTS] = { 0.646103998731393125f, 0.852614963911420554f, 0.967245082131163447f };
static const float c_b0_2700[IIR_SECTS] = { 0.356774404615265051f, 0.455441530974172282f, 0.130607178756940195f };
static const float c_b1_2700[IIR_SECTS] = {-0.649255407094503356f,-0.795224194479007607f,-0.119703361063555877f };
static const float c_b2_2700[IIR_SECTS] = { 0.356774404615265051f, 0.455441530974172282f, 0.130607178756940195f };


// 3 pole elliptic filter LPF
// fs = 44100 Hz, bw = 2500
static const float c_a0_2500[IIR_SECTS] = {  1.000000000000000000f, 1.000000000000000000f, 1.000000000000000000f };
static const float c_a1_2500[IIR_SECTS] = { -1.612311801052039640f,-1.763017330659917280f,-1.848177323655816990f };
static const float c_a2_2500[IIR_SECTS] = {  0.667993837598834461f, 0.862574306359823573f, 0.969564473120013703f };
static const float c_b0_2500[IIR_SECTS] = {  0.360151351308009016f, 0.455696163244783403f, 0.126197926471069588f };
static const float c_b1_2500[IIR_SECTS] = { -0.664620666069223209f,-0.811835350789660515f,-0.131008703477942606f };
static const float c_b2_2500[IIR_SECTS] = {  0.360151351308009016f, 0.455696163244783403f, 0.126197926471069588f };


// 3 pole elliptic filter LPF
// fs = 44100 Hz, bw = 1700
static const float c_a0_1700[IIR_SECTS] = { 1.000000000000000000f, 1.000000000000000000f, 1.000000000000000000f };
static const float c_a1_1700[IIR_SECTS] = {-1.732647349174988530f,-1.855671112329910780f,-1.921708731766513130f };
static const float c_a2_1700[IIR_SECTS] = { 0.759979897673345062f, 0.903257637342662623f, 0.978886090476094761f };
static const float c_b0_1700[IIR_SECTS] = { 0.375474174355530377f, 0.458547185621806519f, 0.112225650070058941f };
static const float c_b1_1700[IIR_SECTS] = {-0.723615800212704219f,-0.869507846230861414f,-0.167273941430536116f };
static const float c_b2_1700[IIR_SECTS] = { 0.375474174355530377f, 0.458547185621806519f, 0.112225650070058941f };



void post_filter_select(POST_FILTERS filter)
{

	switch (filter)
	{
		case CW_650_60:
			c_a0 = c_a0_650_60;
			c_a1 = c_a1_650_60;
			c_a2 = c_a2_650_60;
			c_b0 = c_b0_650_60;
			c_b1 = c_b1_650_60;
			c_b2 = c_b2_650_60;
			break;
		case CW_700_100:
			c_a0 = c_a0_700_100;
			c_a1 = c_a1_700_100;
			c_a2 = c_a2_700_100;
			c_b0 = c_b0_700_100;
			c_b1 = c_b1_700_100;
			c_b2 = c_b2_700_100;
			break;
		case CW_750_200:
			c_a0 = c_a0_750_200;
			c_a1 = c_a1_750_200;
			c_a2 = c_a2_750_200;
			c_b0 = c_b0_750_200;
			c_b1 = c_b1_750_200;
			c_b2 = c_b2_750_200;
			break;
		case CW_750_500:
			c_a0 = c_a0_750_500;
			c_a1 = c_a1_750_500;
			c_a2 = c_a2_750_500;
			c_b0 = c_b0_750_500;
			c_b1 = c_b1_750_500;
			c_b2 = c_b2_750_500;
			break;
		case SSB_3000:
			c_a0 = c_a0_3000;
			c_a1 = c_a1_3000;
			c_a2 = c_a2_3000;
			c_b0 = c_b0_3000;
			c_b1 = c_b1_3000;
			c_b2 = c_b2_3000;
			break;
		case SSB_2700:
			c_a0 = c_a0_2700;
			c_a1 = c_a1_2700;
			c_a2 = c_a2_2700;
			c_b0 = c_b0_2700;
			c_b1 = c_b1_2700;
			c_b2 = c_b2_2700;
			break;
		case SSB_2500:
			c_a0 = c_a0_2500;
			c_a1 = c_a1_2500;
			c_a2 = c_a2_2500;
			c_b0 = c_b0_2500;
			c_b1 = c_b1_2500;
			c_b2 = c_b2_2500;
			break;
		case SSB_1700:
			c_a0 = c_a0_1700;
			c_a1 = c_a1_1700;
			c_a2 = c_a2_1700;
			c_b0 = c_b0_1700;
			c_b1 = c_b1_1700;
			c_b2 = c_b2_1700;
			break;

		default:
			assert(false);
	}
	// clear filter regs
	for (auto s = 0; s < IIR_SECTS; s++) {
		frx1[s] = 0.0f;
		frx2[s] = 0.0f;
		fry1[s] = 0.0f;
		fry2[s] = 0.0f;
	}
}


// Form 1 Biquad Section Calc, called by iir_filter_sample
inline float post_filter_sect(int k, float x)
{
	auto ct = x * c_b0[k] + c_b1[k] * frx1[k] + c_b2[k] * frx2[k];
	auto y  = c_a0[k] * ct - c_a1[k] * fry1[k] - c_a2[k] * fry2[k];

	frx2[k] = frx1[k];
	frx1[k] = x;
	fry2[k] = fry1[k];
	fry1[k] = y;

	return y;
}


float post_filter_sample( float sample)
{
	auto y = post_filter_sect(0, sample);
	
	for (auto s = 1; s < IIR_SECTS; s++)
	{
		y = post_filter_sect(s, y);
	}
	
	return y;
}
