
#include "context_ini.h"

#define CTX_UNUSED          {0,64}

static const char INIT_IPR_I[1][1][32][2] =
{
	//----- model 0 -----
	{
        { { -1,47} , {   0,97},  { -4,66} , { -6,58},{ 6,92} , { -3,95}, CTX_UNUSED , {   0,64}, { 0,64} , {   0,64},{ 0,64} , {   0,64},{ 0,64} , {   0,64},{ 0,64} , {   0,64},{ 0,64} , {   0,64},{ 0,64} , {   0,64},{ 0,64} , {   0,64},  { 0,64} , {   0,64},  { 0,64} , {   0,64},{ 0,64} , {   0,64},{ 0,64} , {   0,64}, { 0,64} , {   0,64}}
    }
};

static const char INIT_TRANS_I[1][1][32][2] =
{
	//----- model 0 -----
	{
       { { 2,61} , CTX_UNUSED, {  0,63},  { 7,52} , {   8,65} , {   0,64},  { 0,64} , {   0,64},  { 0,64} , {   0,64},{ 0,64} , {   0,64},{ 0,64} , {   0,64}, { 0,64} , {   0,64},
      { 0,64} , {   0,64},  { 0,64} , {   0,64},{ 0,64} , {   0,64},  { 0,64} , {   0,64},  { 0,64} , {   0,64},{ 0,64} , {   0,64},{ 0,64} , {   0,64}, { 0,64} , {   0,64}
        }
	}
};

static const char INIT_BCBP_I[1][22][4][2] =
{
	//----- model 0 -----
	{
        { { -17, 123} , { -12, 115} , { -16, 122} , { -11, 115} },
        { { -12,  63} , {  -2,  68} , { -15,  84} , { -13, 104} },
        { {  14,  82} , { -2,  126} , CTX_UNUSED , CTX_UNUSED },      //2: luma_8x8
        {  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        { {  0,  64} , {  0,  120} , { 0,  120} , { -30, 127} },
        { {  -12,  75} , {  -7,  127} , CTX_UNUSED , CTX_UNUSED },   //5: luma_4x4
        { {  -4,  56} , {  -5,  82} , {  -7,  76} , { -22, 125} },
        {  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        {  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        {  {  3,  66} ,  { -2,  127} ,  CTX_UNUSED ,  CTX_UNUSED },   //9: luma8x4

        { { -17, 123} , { -12, 115} , { -16, 122} , { -11, 115} },
        { { -12,  63} , {  -2,  68} , { -15,  84} , { -13, 104} },
        { {  -3,  70} , {  -8,  93} , { -10,  90} , { -30, 127} },
        {  {  3,  65} ,  { -2,  125} ,  CTX_UNUSED ,  CTX_UNUSED },   //13: luma4x8
        { {  0,  64} , {  0,  64} , { 0,  120} , { -30, 127} },
        {  {  -13,  83} ,  {-12, 127} ,  CTX_UNUSED ,  CTX_UNUSED },   //15: cb4x4

        { { -17, 123} , { -12, 115} , { -16, 122} , { -11, 115} },
        { { -11,  81} , {  -10,  127} , CTX_UNUSED , CTX_UNUSED },   //17: cb4x8
        { {  -3,  70} , {  -8,  93} , { -10,  90} , { -30, 127} },
        {  {  -21, 91} ,  { -16,  127} ,  CTX_UNUSED ,  CTX_UNUSED },   //19: cr4x8
        { {  0,  64} , {  0,  64} , { 0,  120} , { -30, 127} },
        {  {  -21,  97} , {  -15,  127} ,  CTX_UNUSED ,  CTX_UNUSED }    //21: cr4x4
	}
};

static const char INIT_MAP_I[1][22][15][2] =
{
	//----- model 0 -----
	{
        { { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        {  { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        { CTX_UNUSED , {-7,82},{-8,85},{-7,74},{-6,70},{-7,67},{-7,65},{-8,66},{-10,74},{-17,77},{-16,78},{-18,83},{-18,85},{-17,87},{-18,88} },  //2: luma_8x8
        {  { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        { { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        { CTX_UNUSED , {-2,76},{-2,75},{-3,69},{-5,72},{-4,71},{-7,70},{-6,68},{-5,68},{-8,74},{-9,72},{-7,72},{-8 ,72},{-9	,73},{-12,85} },//5: luma_4x4
        { { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        {  { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        { { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        { {-13,127},{-16,126},{-16,126},{-17,123},{-17,123},{-17,119},{-19,114},{-17,116},{-17,114},{-18,109},{-17,110},{-17,106},{-16,105},{-16,103},{-17,104} },//9: luma8x4

         { { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        {  { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        { { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        {{-10,121},{-12,119},{-12,120},{-14,117},{-14,117},{-14,112},{-16,108},{-14,109},{-14,108},{-15,104},{-14,107},{-14,103},{-14,101},{-15,100},{-15,101} },//13: luma4x8
        { { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        { {-16,111},{-16,104},{-14,98 },{-15,95 },{-14,90 },{-12,89 },{-14,88 },{-11,84 },{-13,85 },{-12,80 },{-12,79 },{-12,82 },{-14,87 },{-12,82 },{-13,84 } },//15: cb4x4

     { { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        { CTX_UNUSED,{-14,82},{-14,82},{-16,76},{-15,78},{-16,76},{-20,73},{-18,72},{-19,70},{-20,69},{-18,71},{-18,69},{-18,67},{-18,66},{-19,66} }, //17: cb4x8
        { { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        { CTX_UNUSED,{-16,79},{-15,79},{-20,75},{-16,74},{-17,71},{-21,67},{-19,67},{-20,66},{-19,62},{-14,63},{-14,62},{-16,60},{-12,57},{-13,58}},//19: cr4x8
        { { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , { 0,64} , {  0,64} , { 0,64} , { 0,64} , {   0,64} , {   0,64} , {  0,64} , {  0,64} },
        { {-14,104},{-13,94 },{-11,88 },{-13,87 },{-9	,79 },{-10,81 },{-11,78 },{-8,76 },{-7,73 },{-8	,71 },{-8,70 },{-7	,72 },{-10,77 },{-9	,76 },{-9,76 } },//21: cr4x4
    }
};

static const char INIT_LAST_I[1][22][15][2] =
{
	//----- model 0 -----
	{
         { {  24,   0} , {  15,   9} , {   8,  25} , {  13,  18} , {  15,   9} , {  13,  19} , {  10,  37} , {  12,  18} , {   6,  29} , {  20,  33} , {  15,  30} , {   4,  45} , {   1,  58} , {   0,  62} , {   7,  61} },
        {  { 31,43} , { -24,127} , {-25, 124} , {-25, 115} , {-17,99} , {-8, 77} , { -5, 72} , CTX_UNUSED , {  0, 37} , {  0, 41} , {  0, 45} , {  0, 49} , {  0, 53} , {  0, 57} , {  0, 61} },
        { {  31, 71} , {  -5, 36} , {-3,36} , {2,33} , {3, 37} , {2, 46} , {2,45} , CTX_UNUSED , {  0, 37} , {  0, 41} , {  0, 45} , {  0, 49} , {  0, 53} , {  0, 57} , {  0, 61} },//2: luma_8x8
        { {  0, 5} , {  0, 9} , {  0, 13} , {  0, 17} , {  0, 21} , {  0, 25} , {  0, 29} , {  0, 33} , {  0, 37} , {  0, 41} , {  0, 45} , {  0, 49} , {  0, 53} , {  0, 57} , {  0, 61} },
        { {  24, 48} , {  -27,116} , { -16,94} , {-5, 71} , {-5,71} , CTX_UNUSED , {  0, 29} , {  0, 33} , {  0, 37} , {  0, 41} , {  0, 45} , {  0, 49} , {  0, 53} , {  0, 57} , {  0, 61} },
        { {  20, 79} , {  -6, 45} , {0, 45} , { 5,45} , {4, 48} , CTX_UNUSED , {  0, 29} , {  0, 33} , {  0, 37} , {  0, 41} , {  0, 45} , {  0, 49} , {  0, 53} , {  0, 57} , {  0, 61} },//5: luma_4x4
        { {  30,  -6} , {  27,   3} , {  26,  22} ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        {  CTX_UNUSED , {  37, -16} , {  35,  -4} , {  38,  -8} , {  38,  -3} , {  37,   3} , {  38,   5} , {  42,   0} , {  35,  16} , {  39,  22} , {  14,  48} , {  27,  37} , {  21,  60} , {  12,  68} , {   2,  97} },
        {  {30,19} ,  {-17,127} ,  {-21,127} , {-16,110} , {-6,78} , {-4,74} ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        {  {24,55} ,  {-19,75} ,  {-10,64} ,  {-7,60} ,  {-2,59} ,  {-3,60} ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },//9: luma8x4

        { {  24,   0} , {  15,   9} , {   8,  25} , {  13,  18} , {  15,   9} , {  13,  19} , {  10,  37} , {  12,  18} , {   6,  29} , {  20,  33} , {  15,  30} , {   4,  45} , {   1,  58} , {   0,  62} , {   7,  61} },
        {  CTX_UNUSED , {  12,  38} , {  11,  45} , {  15,  39} , {  11,  42} , {  13,  44} , {  16,  45} , {  12,  41} , {  10,  49} , {  30,  34} , {  18,  42} , {  10,  55} , {  17,  51} , {  17,  46} , {   0,  89} },
        { {  0,30} , {  0,33} , {  0,36} , {  0,39} , {  0,42} , {  0,45} , {  0,48} , {  0,51} , {  0,54} , {  0,57} , {  0,60} , {  0,63} , {  0,66} , {  0,69} , {  0,72} },
        { {  0,30} , {  0,33} , {  0,36} , {  0,39} , {  0,42} , {  0,45} , {  0,48} , {  0,51} , {  0,54} , {  0,57} , {  0,60} , {  0,63} , {  0,66} , {  0,69} , {  0,72} },
        { {  16,64} , { -34,126} , {-19,97} , {-5,70} , {-5,72} , CTX_UNUSED , {  0,48} , {  0,51} , {  0,54} , {  0,57} , {  0,60} , {  0,63} , {  0,66} , {  0,69} , {  0,72} },
        { {  22,65} , {  -10,49} , {-4,47} , {-1,50} , {1,53} , CTX_UNUSED , {  0,48} , {  0,51} , {  0,54} , {  0,57} , {  0,60} , {  0,63} , {  0,66} , {  0,69} , {  0,72} },//15: cb4x4

        { {25,78} , {-37,113} , {-17,86} , {-10,78} , {-3,63} , {-3,66} , CTX_UNUSED , {  12,  18} , {   6,  29} , {  20,  33} , {  15,  30} , {   4,  45} , {   1,  58} , {   0,  62} , {   7,  61} },
        {  {8,112} , {  -12,43} , {-5,44} , {-3,50} , {-2,53} , {0,53} , CTX_UNUSED , {  12,  41} , {  10,  49} , {  30,  34} , {  18,  42} , {  10,  55} , {  17,  51} , {  17,  46} , {   0,  89} },//17: cb4x8
        { {  17,91} , {-35,103} , {-17,81} , {-11,75} , {-2,62} , {-3,66} , CTX_UNUSED , {  0,51} , {  0,54} , {  0,57} , {  0,60} , {  0,63} , {  0,66} , {  0,69} , {  0,72} },
        { {  6,115} , {-17,47} , {-11,48} , {-8,53} , {-5,56} , {-4,57} , CTX_UNUSED , {  0,51} , {  0,54} , {  0,57} , {  0,60} , {  0,63} , {  0,66} , {  0,69} , {  0,72} },//19: cr4x8
        { {  5,87} , {-30,113} , {-20,94} , {-6,71} , {-8,75} , CTX_UNUSED , {  0,48} , {  0,51} , {  0,54} , {  0,57} , {  0,60} , {  0,63} , {  0,66} , {  0,69} , {  0,72} },
        { {  16,80} , {  -14,56} , {-11,58} , {-5,58} , {-4,61} , CTX_UNUSED , {  0,48} , {  0,51} , {  0,54} , {  0,57} , {  0,60} , {  0,63} , {  0,66} , {  0,69} , {  0,72} },//21: cr4x4
    }
};

static const char INIT_ONE_I[1][22][8][2] =
{
	//----- model 0 -----
	{
        { CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        { CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        { {-10,81},{-26,66},{-16,55},{-7,45},{-2,39},{1,36},{4,33},{7,33}},//2: luma_8x8
        { CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        { CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        { {-4,74},{-18,63},{-7,54},{0,49},{0,53},{0	,56},{0 ,58},{0,61} },//5: luma_4x4
        { CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        {  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED},
        {  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        {  {-10,96},{-23,80},{-12,60},{-4, 48},{0, 43},{2, 42},{4, 37},{4, 44} },//9: luma8x4

        { CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        { CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        { CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        { {-8, 93},{-19,73},{-8, 53},{-1, 43},{2, 39},{4, 38},{5, 35},{4, 42}},//13: luma4x8
        { CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        {  {-6, 77},{-12,48},{-7, 49},{-3, 50},{-1, 51},{-1, 54},{-12,72},{-13,77} },//15: cb4x4

        { CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        { {-6 ,65},{-10,27},{-7,34},{-4,38},{-2,41},{0,43},{-3,48},{-4,53} },//17: cb4x8
        { CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        {{-3,61},{-6,21},{-3,28},{-1,33},{1	,37},{4	,38},{-3,46},{-6,52} },//19: cr4x8
        { CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED, CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
        {  {-3, 70},{-8, 38},{-2, 39},{2, 40},{3, 43},{3, 47},{1, 54},{-3, 63} }//21: cr4x4
	}
};

static const char INIT_ABS_I[1][22][5][2] =
{
	//----- model 0 -----
	{
		{ {   0,  58} , {   1,  63} , {  -2,  72} , {  -1,  74} , {  -9,  91} },
		{ { -16,  64} , {  -8,  68} , { -10,  78} , {  -6,  77} , { -10,  86} },
		{  {  -2,  55} , {   0,  61} , {   1,  64} , {   0,  68} , {  -9,  92} },
		{  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
		{ { -12,  73} , {  -8,  76} , {  -7,  80} , {  -9,  88} , { -17, 110} },
		{ { -13,  86} , { -13,  96} , { -11,  97} , { -19, 117} ,  CTX_UNUSED },
		{ { -13,  71} , { -10,  79} , { -12,  86} , { -13,  90} , { -14,  97} },
		{  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
		{  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
		{  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
		//Cb in the 4:4:4 common mode
		{ {   0,  58} , {   1,  63} , {  -2,  72} , {  -1,  74} , {  -9,  91} },
		{ { -16,  64} , {  -8,  68} , { -10,  78} , {  -6,  77} , { -10,  86} },
		{  {  -2,  55} , {   0,  61} , {   1,  64} , {   0,  68} , {  -9,  92} },
		{  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
		{ { -12,  73} , {  -8,  76} , {  -7,  80} , {  -9,  88} , { -17, 110} },
		{  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
		//Cr in the 4:4:4 common mode
		{ {   0,  58} , {   1,  63} , {  -2,  72} , {  -1,  74} , {  -9,  91} },
		{ { -16,  64} , {  -8,  68} , { -10,  78} , {  -6,  77} , { -10,  86} },
		{  {  -2,  55} , {   0,  61} , {   1,  64} , {   0,  68} , {  -9,  92} },
		{  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED },
		{ { -12,  73} , {  -8,  76} , {  -7,  80} , {  -9,  88} , { -17, 110} },
		{  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED ,  CTX_UNUSED }
	}
};

static const char INIT_MV_RES_P[3][2][10][2] =
{
	//----- model 0 -----
	{
		{ {  -3,  69} ,  CTX_UNUSED , {  -6,  81} , { -11,  96} ,  CTX_UNUSED , {   0,  58} ,  CTX_UNUSED , {  -3,  76} , { -10,  94} ,  CTX_UNUSED },
		{ {   6,  55} , {   7,  67} , {  -5,  86} , {   2,  88} ,  CTX_UNUSED , {   5,  54} , {   4,  69} , {  -3,  81} , {   0,  88} ,  CTX_UNUSED }
	},
	//----- model 1 -----
	{
		{ {  -2,  69} ,  CTX_UNUSED , {  -5,  82} , { -10,  96} ,  CTX_UNUSED , {   1,  56} ,  CTX_UNUSED , {  -3,  74} , {  -6,  85} ,  CTX_UNUSED },
		{ {   2,  59} , {   2,  75} , {  -3,  87} , {  -3, 100} ,  CTX_UNUSED , {   0,  59} , {  -3,  81} , {  -7,  86} , {  -5,  95} ,  CTX_UNUSED }
	},
	//----- model 2 -----
	{
		{ { -11,  89} ,  CTX_UNUSED , { -15, 103} , { -21, 116} ,  CTX_UNUSED , {   1,  63} ,  CTX_UNUSED , {  -5,  85} , { -13, 106} ,  CTX_UNUSED },
		{ {  19,  57} , {  20,  58} , {   4,  84} , {   6,  96} ,  CTX_UNUSED , {   5,  63} , {   6,  75} , {  -3,  90} , {  -1, 101} ,  CTX_UNUSED }
	}
};


static const char INIT_PRED_P[1][1][16][2] =
{
	//----- model 0 -----
	{
		{ { 0,64} , {   0,64},  { 0,64} , {   0,64},{ 0,64} , {   0,64},{ 0,64} , {   0,64}, { 0,64} , {   0,64},{ 0,64} , {   0,64},{ 0,64} , {   0,64},{ 0,64} , {   0,64}}
	}
};

#define BIARI_CTX_INIT2(qp, ii,jj,ctx,tab) \
{ \
  for (i=0; i<ii; i++) \
  for (j=0; j<jj; j++) \
  { \
    biari_init_context (qp, &(ctx[i][j]), &(tab[i][j][0])); \
  } \
}

static inline void binary_context_init1(int qp, int jj, BiContextType *ctx, const char table[][2])
{
	int j;
	for (j = 0; j < jj; j++)
	{
		biari_init_context(qp, &(ctx[j]), &(table[j][0]));
	}
}

void InitContextsLl(int qp,TextureInfoContexts *texCtx, MotionInfoContexts *motCtx)
{
	TextureInfoContexts* tc = texCtx;
	int model_number = 0;
	int i, j;

	//--- motion coding contexts ---
	MotionInfoContexts*  mc = motCtx;
	BIARI_CTX_INIT2(qp, 2, NUM_MV_RES_CTX, mc->mv_res_contexts, INIT_MV_RES_P[model_number]);
	binary_context_init1(qp, NUM_MB_PRED_CTX, mc->mb_pred_context, INIT_PRED_P[model_number][0]);

	//--- texture coding contexts ---
	binary_context_init1(qp, NUM_IPR_CTX, tc->ipr_contexts, INIT_IPR_I[model_number][0]);   //pre and transform mode
	binary_context_init1(qp, NUM_IPR_CTX, tc->trans_contexts, INIT_TRANS_I[model_number][0]);   //pre and transform mode
	BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_BCBP_CTX, tc->bcbp_contexts, INIT_BCBP_I[model_number]);
	BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_MAP_CTX, tc->map_contexts[0], INIT_MAP_I[model_number]);
	BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_LAST_CTX, tc->last_contexts[0], INIT_LAST_I[model_number]);
	BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_ONE_CTX, tc->one_contexts, INIT_ONE_I[model_number]);
}

void InitContextsHf(int qp, HighBandInfoContexts *highBandCtx)
{
    char defaulTable[2] = { 0, 64 };
    HighBandInfoContexts*  hb = highBandCtx;
    for (int i = 0; i < NUM_MODELS_Z_FLAGS; i++) {
        biari_init_context(qp, &(hb->zFlag[i]), defaulTable);
    }
}