#ifndef _CUST_BATTERY_METER_TABLE_H
#define _CUST_BATTERY_METER_TABLE_H

#include <mach/mt_typedefs.h>

// ============================================================
// define
// ============================================================
#define BAT_NTC_10 1
#define BAT_NTC_47 0

#if (BAT_NTC_10 == 1)
#define RBAT_PULL_UP_R             16900	
#define RBAT_PULL_DOWN_R		   27000	
#endif

#if (BAT_NTC_47 == 1)
#define RBAT_PULL_UP_R             61900	
#define RBAT_PULL_DOWN_R		  100000	
#endif
#define RBAT_PULL_UP_VOLT          1800



// ============================================================
// ENUM
// ============================================================

// ============================================================
// structure
// ============================================================

// ============================================================
// typedef
// ============================================================
typedef struct _BATTERY_PROFILE_STRUC
{
    kal_int32 percentage;
    kal_int32 voltage;
} BATTERY_PROFILE_STRUC, *BATTERY_PROFILE_STRUC_P;

typedef struct _R_PROFILE_STRUC
{
    kal_int32 resistance; // Ohm
    kal_int32 voltage;
} R_PROFILE_STRUC, *R_PROFILE_STRUC_P;

typedef enum
{
    T1_0C,
    T2_25C,
    T3_50C
} PROFILE_TEMPERATURE;

// ============================================================
// External Variables
// ============================================================

// ============================================================
// External function
// ============================================================

// ============================================================
// <DOD, Battery_Voltage> Table
// ============================================================
#if (BAT_NTC_10 == 1)
    BATT_TEMPERATURE Batt_Temperature_Table[] = {
    #if 1
        {-20,68237},
        {-15,53650},
        {-10,42506},
        { -5,33892},
        {  0,27219},
        {  5,22021},
        { 10,17926},
        { 15,14674},
        { 20,12081},
        { 25,10000},
        { 30,8315},
        { 35,6948},
        { 40,5834},
        { 45,4917},
        { 50,4161},
        { 55,3535},
        { 60,3014}
    #else
        {-20,67790},
        {-15,53460},
        {-10,42450},
        { -5,33930},
        {  0,27280},
        {  5,22070},
        { 10,17960},
        { 15,14700},
        { 20,12090},
        { 25,10000},
        { 30,8312},
        { 35,6942},
        { 40,5826},
        { 45,4911},
        { 50,4158},
        { 55,3536},
        { 60,3019}
    #endif
    };
#endif

#if (BAT_NTC_47 == 1)
    BATT_TEMPERATURE Batt_Temperature_Table[] = {
        {-20,483954},
        {-15,360850},
        {-10,271697},
        { -5,206463},
        {  0,158214},
        {  5,122259},
        { 10,95227},
        { 15,74730},
        { 20,59065},
        { 25,47000},
        { 30,37643},
        { 35,30334},
        { 40,24591},
        { 45,20048},
        { 50,16433},
        { 55,13539},
        { 60,11210}        
    };
#endif

// T0 -10C
BATTERY_PROFILE_STRUC battery_profile_t0[] =
{
	{0 , 4192},         
	{1 , 4174},         
	{3 , 4159},         
	{4 , 4144},         
	{6 , 4130},         
	{7 , 4116},         
	{8 , 4103},         
	{10 , 4091},         
	{11 , 4083},         
	{13 , 4073},         
	{14 , 4058},         
	{16 , 4037},         
	{17 , 4017},         
	{18 , 4000},         
	{20 , 3987},         
	{21 , 3978},         
	{23 , 3970},         
	{24 , 3961},         
	{25 , 3952},         
	{27 , 3943},         
	{28 , 3933},         
	{30 , 3923},         
	{31 , 3912},         
	{32 , 3902},         
	{34 , 3893},         
	{35 , 3884},         
	{37 , 3875},         
	{38 , 3866},         
	{39 , 3859},         
	{41 , 3851},         
	{42 , 3844},         
	{44 , 3837},         
	{45 , 3831},         
	{47 , 3826},         
	{48 , 3820},         
	{49 , 3815},         
	{51 , 3810},         
	{52 , 3806},         
	{54 , 3802},         
	{55 , 3798},         
	{56 , 3794},         
	{58 , 3790},         
	{59 , 3787},         
	{61 , 3784},         
	{62 , 3781},         
	{63 , 3778},         
	{65 , 3776},         
	{66 , 3774},         
	{68 , 3772},         
	{69 , 3769},         
	{71 , 3766},         
	{72 , 3763},         
	{73 , 3760},         
	{75 , 3756},         
	{76 , 3752},         
	{78 , 3748},         
	{79 , 3743},         
	{80 , 3737},         
	{82 , 3731},         
	{83 , 3724},         
	{85 , 3718},          
    {86 , 3713},
    {87 , 3709},
    {89 , 3705},
    {90 , 3702},
    {92 , 3696},
    {93 , 3684},
    {94 , 3659},
    {96 , 3616},
    {97 , 3549},
	{100 , 3450}, 
	{100 , 3400}       
};      
        
// T1 0C 
BATTERY_PROFILE_STRUC battery_profile_t1[] =
{
	{0 , 4180},         
	{1 , 4163},         
	{3 , 4148},         
	{4 , 4133},         
	{6 , 4119},         
	{7 , 4106},         
	{9 , 4093},         
	{10 , 4083},         
	{11 , 4074},         
	{13 , 4062},         
	{14 , 4043},         
	{16 , 4024},         
	{17 , 4008},         
	{19 , 3996},         
	{20 , 3987},         
	{22 , 3980},         
	{23 , 3971},         
	{24 , 3963},         
	{26 , 3955},         
	{27 , 3945},         
	{29 , 3937},         
	{30 , 3927},         
	{32 , 3917},         
	{33 , 3907},         
	{34 , 3897},         
	{36 , 3887},         
	{37 , 3877},         
	{39 , 3867},         
	{40 , 3858},         
	{42 , 3849},         
	{43 , 3842},         
	{44 , 3834},         
	{46 , 3828},         
	{47 , 3822},         
	{49 , 3816},         
	{50 , 3811},         
	{52 , 3807},         
	{53 , 3803},         
	{55 , 3799},         
	{56 , 3795},         
	{57 , 3792},         
	{59 , 3789},         
	{60 , 3786},         
	{62 , 3784},         
	{63 , 3781},         
	{65 , 3779},         
	{66 , 3778},         
	{67 , 3776},         
	{69 , 3774},         
	{70 , 3772},         
	{72 , 3769},         
	{73 , 3767},         
	{75 , 3764},         
	{76 , 3760},         
	{77 , 3757},         
	{79 , 3752},         
	{80 , 3747},         
	{82 , 3741},         
    {83 , 3734},
	{85 , 3727},         
	{86 , 3720},          
    {88 , 3710},
    {89 , 3702},
    {90 , 3698},
    {92 , 3696},
    {93 , 3692},
    {95 , 3686},
    {96 , 3659},
    {98 , 3599},
    {99 , 3508},
	{100 , 3450}, 
	{100 , 3400}
};           

// T2 25C
BATTERY_PROFILE_STRUC battery_profile_t2[] =
{
	{0 , 4184},         
	{1 , 4166},         
	{3 , 4151},         
	{4 , 4136},         
	{6 , 4122},         
	{7 , 4109},         
	{9 , 4095},         
	{10 , 4083},         
	{11 , 4071},         
	{13 , 4060},         
	{14 , 4047},         
	{16 , 4033},         
	{17 , 4020},         
	{19 , 4008},         
	{20 , 3998},         
	{21 , 3987},         
	{23 , 3978},         
	{24 , 3969},         
	{26 , 3960},         
	{27 , 3951},         
	{29 , 3942},         
	{30 , 3933},         
	{31 , 3925},         
	{33 , 3917},         
	{34 , 3909},         
	{36 , 3901},         
	{37 , 3892},         
	{38 , 3883},         
	{40 , 3872},         
	{41 , 3860},         
	{43 , 3849},         
	{44 , 3839},         
	{46 , 3831},         
	{47 , 3824},         
	{48 , 3818},         
	{50 , 3813},         
	{51 , 3808},         
	{53 , 3803},         
	{54 , 3800},         
	{56 , 3795},         
	{57 , 3792},         
	{58 , 3789},         
	{60 , 3786},         
	{61 , 3783},         
	{63 , 3781},         
	{64 , 3779},         
	{66 , 3777},         
	{67 , 3774},         
	{68 , 3773},         
	{70 , 3770},         
	{71 , 3767},         
	{73 , 3764},         
	{74 , 3760},         
	{76 , 3755},         
	{77 , 3750},         
	{78 , 3744},         
	{80 , 3740},         
	{81 , 3735},         
	{83 , 3729},         
	{84 , 3721},         
	{86 , 3713},          
    {87 , 3705},
    {88 , 3694},
    {90 , 3687},
    {91 , 3685},
    {93 , 3684},
    {94 , 3680},
    {96 , 3667},
    {97 , 3619},
    {98 , 3543},
	{100 , 3450}, 
	{100 , 3400}
       
};     

// T3 50C
BATTERY_PROFILE_STRUC battery_profile_t3[] =
{
	{0 , 4179},         
	{1 , 4162},         
	{3 , 4146},         
	{4 , 4131},         
	{6 , 4117},         
	{7 , 4103},         
	{9 , 4090},         
	{10 , 4077},         
	{12 , 4064},         
	{13 , 4052},         
	{14 , 4040},         
	{16 , 4027},         
	{17 , 4016},         
	{19 , 4004},         
	{20 , 3994},         
	{22 , 3983},         
	{23 , 3973},         
	{24 , 3963},         
	{26 , 3954},         
	{27 , 3944},         
	{29 , 3935},         
	{30 , 3927},         
	{32 , 3918},         
	{33 , 3910},         
	{35 , 3902},         
	{36 , 3895},         
	{37 , 3887},         
	{39 , 3879},         
	{40 , 3870},         
	{42 , 3858},         
	{43 , 3845},         
	{45 , 3834},         
	{46 , 3826},         
	{47 , 3819},         
	{49 , 3813},         
	{50 , 3808},         
	{52 , 3803},         
	{53 , 3799},         
	{55 , 3795},         
	{56 , 3791},         
	{58 , 3787},         
	{59 , 3784},         
	{60 , 3781},         
	{62 , 3778},         
	{63 , 3775},         
	{65 , 3773},         
	{66 , 3770},         
	{68 , 3768},         
	{69 , 3766},         
	{70 , 3762},         
	{72 , 3753},         
	{73 , 3744},         
	{75 , 3740},         
	{76 , 3735},         
	{78 , 3729},         
	{79 , 3724},         
	{81 , 3720},         
	{82 , 3715},         
	{83 , 3708},         
	{85 , 3700},         
	{86 , 3693},          
    {88 , 3684},
    {89 , 3674},
    {91 , 3671},
    {92 , 3670},
    {93 , 3667},
    {95 , 3659},
    {96 , 3627},
    {98 , 3565},
    {100 , 3450},
	{100 , 3400}, 
	{100 , 3400}
};           

// battery profile for actual temperature. The size should be the same as T1, T2 and T3
BATTERY_PROFILE_STRUC battery_profile_temperature[] =
{
  {0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },  
	{0  , 0 }, 
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },  
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 }
};    

// ============================================================
// <Rbat, Battery_Voltage> Table
// ============================================================
// T0 -10C
R_PROFILE_STRUC r_profile_t0[] =
{
	{495, 4192},         
	{495, 4174},         
	{500, 4159},         
	{507, 4144},         
	{509, 4130},         
	{512, 4116},         
	{512, 4103},         
	{532, 4091},         
	{537, 4083},         
	{542, 4073},         
	{543, 4058},         
	{542, 4037},         
	{543, 4017},         
	{543, 4000},         
	{544, 3987},         
	{546, 3978},         
	{548, 3970},         
	{546, 3961},         
	{543, 3952},         
	{541, 3943},         
	{537, 3933},         
	{534, 3923},         
	{530, 3912},         
	{527, 3902},         
	{526, 3893},         
	{523, 3884},         
	{520, 3875},         
	{518, 3866},         
	{516, 3859},         
	{515, 3851},         
	{514, 3844},         
	{514, 3837},         
	{513, 3831},         
	{514, 3826},         
	{515, 3820},         
	{514, 3815},         
	{515, 3810},         
	{516, 3806},         
	{515, 3802},         
	{516, 3798},         
	{516, 3794},         
	{516, 3790},         
	{517, 3787},         
	{517, 3784},         
	{516, 3781},         
	{516, 3778},         
	{516, 3776},         
	{515, 3774},         
	{517, 3772},         
	{518, 3769},         
	{520, 3766},         
	{521, 3763},         
	{522, 3760},         
	{523, 3756},         
	{523, 3752},         
	{524, 3748},         
	{524, 3743},         
	{525, 3737},         
	{522, 3731},         
	{519, 3724},         
	{517, 3718},          
    {516, 3713},
    {516, 3709},
    {522, 3705},
    {534, 3702},
    {556, 3696},
    {580, 3684},
    {608, 3659},
    {646, 3616},
    {740, 3549},
	{1122, 3443}, 
	{1013, 3400}
};      

// T1 0C
R_PROFILE_STRUC r_profile_t1[] =
{
	{331 , 4180},         
	{331 , 4163},         
	{334 , 4148},         
	{338 , 4133},         
	{345 , 4119},         
	{348 , 4106},         
	{350 , 4093},         
	{353 , 4083},         
	{354 , 4074},         
	{358 , 4062},         
	{359 , 4043},         
	{361 , 4024},         
	{363 , 4008},         
	{367 , 3996},         
	{371 , 3987},         
	{374 , 3980},         
	{374 , 3971},         
	{376 , 3963},         
	{377 , 3955},         
	{376 , 3945},         
	{375 , 3937},         
	{374 , 3927},         
	{373 , 3917},         
	{370 , 3907},         
	{365 , 3897},         
	{361 , 3887},         
	{354 , 3877},         
	{350 , 3867},         
	{344 , 3858},         
	{340 , 3849},         
	{337 , 3842},         
	{334 , 3834},         
	{332 , 3828},         
	{330 , 3822},         
	{329 , 3816},         
	{329 , 3811},         
	{330 , 3807},         
	{331 , 3803},         
	{333 , 3799},         
	{334 , 3795},         
	{335 , 3792},         
	{336 , 3789},         
	{340 , 3786},         
	{340 , 3784},         
	{341 , 3781},         
	{342 , 3779},         
	{344 , 3778},         
	{344 , 3776},         
	{344 , 3774},         
	{344 , 3772},         
	{344 , 3769},         
	{344 , 3767},         
	{345 , 3764},         
	{346 , 3760},         
	{350 , 3757},         
	{349 , 3752},         
	{349 , 3747},         
	{348 , 3741},         
	{347 , 3734},         
	{347 , 3727},         
	{347 , 3720},          
    {342 , 3710},
    {340 , 3702},
    {344 , 3698},
    {353 , 3696},
    {366 , 3692},
    {387 , 3686},
    {391 , 3659},
    {390 , 3599},
    {426 , 3508},
	{550 , 3356}, 
	{550 , 3356}
};     

// T2 25C
R_PROFILE_STRUC r_profile_t2[] =
{
	{181  , 4184},         
	{181  , 4166},         
	{181  , 4151},         
	{180  , 4136},         
	{182  , 4122},         
	{181  , 4109},         
	{181  , 4095},         
	{181  , 4083},         
	{182  , 4071},         
	{185  , 4060},         
	{184  , 4047},         
	{186  , 4033},         
	{187  , 4020},         
	{188  , 4008},         
	{192  , 3998},         
	{192  , 3987},         
	{196  , 3978},         
	{198  , 3969},         
	{200  , 3960},         
	{203  , 3951},         
	{206  , 3942},         
	{209  , 3933},         
	{210  , 3925},         
	{215  , 3917},         
	{217  , 3909},         
	{218  , 3901},         
	{218  , 3892},         
	{217  , 3883},         
	{210  , 3872},         
	{200  , 3860},         
	{192  , 3849},         
	{185  , 3839},         
	{182  , 3831},         
	{180  , 3824},         
	{178  , 3818},         
	{177  , 3813},         
	{176  , 3808},         
	{178  , 3803},         
	{179  , 3800},         
	{178  , 3795},         
	{178  , 3792},         
	{179  , 3789},         
	{180  , 3786},         
	{181  , 3783},         
	{183  , 3781},         
	{185  , 3779},         
	{185  , 3777},         
	{184  , 3774},         
	{185  , 3773},         
	{185  , 3770},         
	{182  , 3767},         
	{181  , 3764},         
	{180  , 3760},         
	{178  , 3755},         
	{176  , 3750},         
	{175  , 3744},         
	{176  , 3740},         
	{178  , 3735},         
	{179  , 3729},         
	{178  , 3721},         
	{179  , 3713},          
    {180  , 3705},
    {178  , 3694},
    {174  , 3687},
    {179  , 3685},
    {185  , 3684},
    {195  , 3680},
    {200  , 3667},
    {196  , 3619},
    {208  , 3543},
	{234  , 3429}, 
	{412  , 3206}
}; 

// T3 50C
R_PROFILE_STRUC r_profile_t3[] =
{
	{144  , 4179},         
	{144  , 4162},         
	{145  , 4146},         
	{144  , 4131},         
	{143  , 4117},         
	{144  , 4103},         
	{145  , 4090},         
	{147  , 4077},         
	{149  , 4064},         
	{155  , 4052},         
	{156  , 4040},         
	{155  , 4027},         
	{156  , 4016},         
	{155  , 4004},         
	{153  , 3994},         
	{154  , 3983},         
	{154  , 3973},         
	{156  , 3963},         
	{157  , 3954},         
	{157  , 3944},         
	{158  , 3935},         
	{160  , 3927},         
	{162  , 3918},         
	{164  , 3910},         
	{168  , 3902},         
	{171  , 3895},         
	{175  , 3887},         
	{178  , 3879},         
	{178  , 3870},         
	{172  , 3858},         
	{160  , 3845},         
	{152  , 3834},         
	{150  , 3826},         
	{148  , 3819},         
	{148  , 3813},         
	{149  , 3808},         
	{148  , 3803},         
	{148  , 3799},         
	{147  , 3795},         
	{149  , 3791},         
	{149  , 3787},         
	{151  , 3784},         
	{153  , 3781},         
	{154  , 3778},         
	{153  , 3775},         
	{155  , 3773},         
	{156  , 3770},         
	{157  , 3768},         
	{158  , 3766},         
	{156  , 3762},         
	{146  , 3753},         
	{144  , 3744},         
	{146  , 3740},         
	{147  , 3735},         
	{146  , 3729},         
	{146  , 3724},         
	{147  , 3720},         
	{148  , 3715},         
	{147  , 3708},         
	{146  , 3700},         
	{147  , 3693},          
    {148  , 3684},
    {144  , 3674},
    {146  , 3671},
    {151  , 3670},
    {155  , 3667},
    {160  , 3659},
    {156  , 3627},
    {161  , 3565},
    {171  , 3475},
	{201  , 3330}, 
	{201  , 3330}
}; 

// r-table profile for actual temperature. The size should be the same as T1, T2 and T3
R_PROFILE_STRUC r_profile_temperature[] =
{
  {0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },  
	{0  , 0 }, 
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },  
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 }
};    

// ============================================================
// function prototype
// ============================================================
int fgauge_get_saddles(void);
BATTERY_PROFILE_STRUC_P fgauge_get_profile(kal_uint32 temperature);

int fgauge_get_saddles_r_table(void);
R_PROFILE_STRUC_P fgauge_get_profile_r_table(kal_uint32 temperature);

#endif	//#ifndef _CUST_BATTERY_METER_TABLE_H
