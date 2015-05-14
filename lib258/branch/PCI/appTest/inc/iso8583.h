const ISO8583BitAttr Default_ISO8583BitAttrTab[ISO8583_MAXBITNUM]
={
	{0,				0	},	//Bit 1
	{ATTR_LLVARN,	20	},	//Bit 2
	{ATTR_N,		6	},  //Bit 3
	{ATTR_N,		12	},  //Bit 4
	{ATTR_N,		12	},  //Bit 5
	{ATTR_N,		12	},  //Bit 6
	{ATTR_N,		10	},  //Bit 7
	{ATTR_N,		8	},  //Bit 8
	{ATTR_N,		8	},  //Bit 9
	{ATTR_N,		8	},  //Bit 10
	{ATTR_N,		6	},  //Bit 11
	{ATTR_N,		6	},  //Bit 12
	{ATTR_N,		4	},  //Bit 13
	{ATTR_N,		4	},  //Bit 14
	{ATTR_N,		4	},  //Bit 15
	{ATTR_N,		4	},  //Bit 16
	{ATTR_N,		4	},  //Bit 17
	{ATTR_N,		4	},  //Bit 18
	{ATTR_N,		3	},  //Bit 19
	{ATTR_N,		3	},  //Bit 20
	{ATTR_N,		3	},  //Bit 21
	{ATTR_N,		3	},  //Bit 22
	{ATTR_N,		3	},  //Bit 23
	{ATTR_N,		3	},  //Bit 24
	{ATTR_N,		2	},  //Bit 25
	{ATTR_N,		2	},  //Bit 26
	{ATTR_N,		1	},  //Bit 27
	{ATTR_XN,		9	},  //Bit 28
	{ATTR_XN,		9	},  //Bit 29
	{ATTR_XN,		9	},  //Bit 30
	{ATTR_XN,		9	},  //Bit 31
	{ATTR_LLVARN,	99	},  //Bit 32
	{ATTR_LLVARANS,	11	},  //Bit 33
	{ATTR_LLVARNS,	28	},  //Bit 34
	{ATTR_LLVARZ,	37	},  //Bit 35
	{ATTR_LLLVARZ,	104	},  //Bit 36
	{ATTR_AN,		12	},  //Bit 37
	{ATTR_AN,		6	},  //Bit 38
	{ATTR_AN,		2	},  //Bit 39
	{ATTR_AN,		3	},  //Bit 40
	{ATTR_ANS,		8	},  //Bit 41
	{ATTR_ANS,		15	},  //Bit 42
	{ATTR_ANS,		40	},  //Bit 43
	{ATTR_LLVARANS,	25	},  //Bit 44
	{ATTR_LLVARANS,	76	},  //Bit 45
	{ATTR_LLLVARANS,999	},  //Bit 46
	{ATTR_LLLVARANS,999	},  //Bit 47
	{ATTR_LLLVARN,999	},  //Bit 48
//	{ATTR_BIN,		384	},  //Bit 48---
	{ATTR_ANS,		3	},  //Bit 49
	{ATTR_ANS,		3	},  //Bit 50
	{ATTR_ANS,		3	},  //Bit 51
	{ATTR_BIN,		64	},  //Bit 52
	{ATTR_N,		16	},  //Bit 53
	{ATTR_LLLVARANS,999	},  //Bit 54
	{ATTR_LLLVARANS,999	},  //Bit 55
	{ATTR_LLLVARANS,999	},  //Bit 56
	{ATTR_LLLVARANS,999	},  //Bit 57
	{ATTR_LLLVARANS,999	},  //Bit 58
	{ATTR_LLLVARANS,999	},  //Bit 59
//	{ATTR_LLLVARANS,999	},  //Bit 60
	{ATTR_LLLVARN,	999	},  //Bit 60
	{ATTR_LLLVARN,    999	},  //Bit 61
	{ATTR_LLLVARANS,999	},  //Bit 62
	{ATTR_LLLVARANS,999	},  //Bit 63
//	{ATTR_LLLVARN,	999	},  //Bit 63
	{ATTR_BIN,		64	},  //Bit 64
	{ATTR_BIN,		1	},  //Bit 65
	{ATTR_N,		1	},  //Bit 66
	{ATTR_N,		2	},  //Bit 67
	{ATTR_N,		3	},  //Bit 68
	{ATTR_N,		3	},  //Bit 69
	{ATTR_N,		3	},  //Bit 70
	{ATTR_N,		4	},  //Bit 71
	{ATTR_N,		4	},  //Bit 72
	{ATTR_N,		6	},  //Bit 73
	{ATTR_N,		10	},  //Bit 74
	{ATTR_N,		10	},  //Bit 75
	{ATTR_N,		10	},  //Bit 76
	{ATTR_N,		10	},  //Bit 77
	{ATTR_N,		10	},  //Bit 78
	{ATTR_N,		10	},  //Bit 79
	{ATTR_N,		10	},  //Bit 80
	{ATTR_N,		10	},  //Bit 81
	{ATTR_N,		12	},  //Bit 82
	{ATTR_N,		12	},  //Bit 83
	{ATTR_N,		12	},  //Bit 84
	{ATTR_N,		12	},  //Bit 85
	{ATTR_N,		16	},  //Bit 86
	{ATTR_N,		16	},  //Bit 87
	{ATTR_N,		16	},  //Bit 88
	{ATTR_N,		16	},  //Bit 89
	{ATTR_N,		42	},  //Bit 90
	{ATTR_AN,		1	},  //Bit 91
	{ATTR_AN,		2	},  //Bit 92
	{ATTR_AN,		5	},  //Bit 93
	{ATTR_AN,		7	},  //Bit 94
	{ATTR_AN,		42	},  //Bit 95
	{ATTR_BIN,		64	},  //Bit 96
	{ATTR_XN,		17	},  //Bit 97
	{ATTR_ANS,		25	},  //Bit 98
	{ATTR_LLVARN,	11	},  //Bit 99
	{ATTR_LLVARN,	11	},  //Bit 100
	{ATTR_LLVARANS,	17	},  //Bit 101
	{ATTR_LLVARANS,	28	},  //Bit 102
	{ATTR_LLVARANS,	28	},  //Bit 103
	{ATTR_LLVARANS,	100	},  //Bit 104
	{ATTR_LLLVARANS,999	},  //Bit 105
	{ATTR_LLLVARANS,999	},  //Bit 106
	{ATTR_LLLVARANS,999	},  //Bit 107
	{ATTR_LLLVARANS,999	},  //Bit 108
	{ATTR_LLLVARANS,999	},  //Bit 109
	{ATTR_LLLVARANS,999	},  //Bit 110
	{ATTR_LLLVARANS,999	},  //Bit 111
	{ATTR_LLLVARANS,999	},  //Bit 112
	{ATTR_LLLVARANS,999	},  //Bit 113
	{ATTR_LLLVARANS,999	},  //Bit 114
	{ATTR_LLLVARANS,999	},  //Bit 115
	{ATTR_LLLVARANS,999	},  //Bit 116
	{ATTR_LLLVARANS,999	},  //Bit 117
	{ATTR_LLLVARANS,999	},  //Bit 118
	{ATTR_LLLVARANS,999	},  //Bit 119
	{ATTR_LLLVARANS,999	},  //Bit 120
	{ATTR_LLLVARANS,999	},  //Bit 121
	{ATTR_LLLVARANS,999	},  //Bit 122
	{ATTR_LLLVARANS,999	},  //Bit 123
	{ATTR_LLLVARANS,999	},  //Bit 124
	{ATTR_LLLVARANS,999	},  //Bit 125
	{ATTR_LLLVARANS,999	},  //Bit 126
	{ATTR_LLLVARANS,999	},  //Bit 127
	{ATTR_BIN,		64	}   //Bit 128
};

const ISO8583BitAttr ISO8583BitAttrTab_Untouch[ISO8583_MAXBITNUM]
={
	{0,				0	},	//Bit 1
	{ATTR_LLVARN,	20	},	//Bit 2
	{ATTR_N,		6	},  //Bit 3
	{ATTR_N,		12	},  //Bit 4
	{ATTR_N,		12	},  //Bit 5
	{ATTR_N,		12	},  //Bit 6
	{ATTR_N,		10	},  //Bit 7
	{ATTR_N,		8	},  //Bit 8
	{ATTR_N,		8	},  //Bit 9
	{ATTR_N,		8	},  //Bit 10
	{ATTR_N,		6	},  //Bit 11
	{ATTR_N,		6	},  //Bit 12
	{ATTR_N,		4	},  //Bit 13
	{ATTR_N,		4	},  //Bit 14
	{ATTR_N,		4	},  //Bit 15
	{ATTR_N,		4	},  //Bit 16
	{ATTR_N,		4	},  //Bit 17
	{ATTR_N,		4	},  //Bit 18
	{ATTR_N,		3	},  //Bit 19
	{ATTR_N,		3	},  //Bit 20
	{ATTR_N,		3	},  //Bit 21
	{ATTR_N,		3	},  //Bit 22
	{ATTR_N,		3	},  //Bit 23
	{ATTR_N,		3	},  //Bit 24
	{ATTR_N,		2	},  //Bit 25
	{ATTR_N,		2	},  //Bit 26
	{ATTR_N,		1	},  //Bit 27
	{ATTR_XN,		9	},  //Bit 28
	{ATTR_XN,		9	},  //Bit 29
	{ATTR_XN,		9	},  //Bit 30
	{ATTR_XN,		9	},  //Bit 31
	{ATTR_LLVARN,	99	},  //Bit 32
	{ATTR_LLVARANS,	11	},  //Bit 33
	{ATTR_LLVARNS,	28	},  //Bit 34
	{ATTR_LLVARANZS,	37	},  //Bit 35
	{ATTR_LLLVARANZS,	104	},  //Bit 36
	{ATTR_AN,		12	},  //Bit 37
	{ATTR_AN,		6	},  //Bit 38
	{ATTR_AN,		2	},  //Bit 39
	{ATTR_AN,		3	},  //Bit 40
	{ATTR_ANS,		8	},  //Bit 41
	{ATTR_ANS,		15	},  //Bit 42
	{ATTR_ANS,		40	},  //Bit 43
	{ATTR_LLVARANS,	25	},  //Bit 44
	{ATTR_LLVARANS,	76	},  //Bit 45
	{ATTR_LLLVARANS,999	},  //Bit 46
	{ATTR_LLLVARANS,999	},  //Bit 47
	{ATTR_LLLVARN,999	},  //Bit 48
//	{ATTR_BIN,		384	},  //Bit 48---
	{ATTR_ANS,		3	},  //Bit 49
	{ATTR_ANS,		3	},  //Bit 50
	{ATTR_ANS,		3	},  //Bit 51
	{ATTR_BIN,		64	},  //Bit 52
	{ATTR_N,		16	},  //Bit 53
	{ATTR_LLLVARANS,999	},  //Bit 54
	{ATTR_LLLVARANS,999	},  //Bit 55
	{ATTR_LLLVARANS,999	},  //Bit 56
	{ATTR_LLLVARANS,999	},  //Bit 57
	{ATTR_LLLVARANS,999	},  //Bit 58
	{ATTR_LLLVARANS,999	},  //Bit 59
//	{ATTR_LLLVARANS,999	},  //Bit 60
	{ATTR_LLLVARN,	999	},  //Bit 60
	{ATTR_LLLVARN,    999	},  //Bit 61
	{ATTR_LLLVARANS,999	},  //Bit 62
	{ATTR_LLLVARANS,999	},  //Bit 63
//	{ATTR_LLLVARN,	999	},  //Bit 63
	{ATTR_BIN,		64	},  //Bit 64
	{ATTR_BIN,		1	},  //Bit 65
	{ATTR_N,		1	},  //Bit 66
	{ATTR_N,		2	},  //Bit 67
	{ATTR_N,		3	},  //Bit 68
	{ATTR_N,		3	},  //Bit 69
	{ATTR_N,		3	},  //Bit 70
	{ATTR_N,		4	},  //Bit 71
	{ATTR_N,		4	},  //Bit 72
	{ATTR_N,		6	},  //Bit 73
	{ATTR_N,		10	},  //Bit 74
	{ATTR_N,		10	},  //Bit 75
	{ATTR_N,		10	},  //Bit 76
	{ATTR_N,		10	},  //Bit 77
	{ATTR_N,		10	},  //Bit 78
	{ATTR_N,		10	},  //Bit 79
	{ATTR_N,		10	},  //Bit 80
	{ATTR_N,		10	},  //Bit 81
	{ATTR_N,		12	},  //Bit 82
	{ATTR_N,		12	},  //Bit 83
	{ATTR_N,		12	},  //Bit 84
	{ATTR_N,		12	},  //Bit 85
	{ATTR_N,		16	},  //Bit 86
	{ATTR_N,		16	},  //Bit 87
	{ATTR_N,		16	},  //Bit 88
	{ATTR_N,		16	},  //Bit 89
	{ATTR_N,		42	},  //Bit 90
	{ATTR_AN,		1	},  //Bit 91
	{ATTR_AN,		2	},  //Bit 92
	{ATTR_AN,		5	},  //Bit 93
	{ATTR_AN,		7	},  //Bit 94
	{ATTR_AN,		42	},  //Bit 95
	{ATTR_BIN,		64	},  //Bit 96
	{ATTR_XN,		17	},  //Bit 97
	{ATTR_ANS,		25	},  //Bit 98
	{ATTR_LLVARN,	11	},  //Bit 99
	{ATTR_LLVARN,	11	},  //Bit 100
	{ATTR_LLVARANS,	17	},  //Bit 101
	{ATTR_LLVARANS,	28	},  //Bit 102
	{ATTR_LLVARANS,	28	},  //Bit 103
	{ATTR_LLVARANS,	100	},  //Bit 104
	{ATTR_LLLVARANS,999	},  //Bit 105
	{ATTR_LLLVARANS,999	},  //Bit 106
	{ATTR_LLLVARANS,999	},  //Bit 107
	{ATTR_LLLVARANS,999	},  //Bit 108
	{ATTR_LLLVARANS,999	},  //Bit 109
	{ATTR_LLLVARANS,999	},  //Bit 110
	{ATTR_LLLVARANS,999	},  //Bit 111
	{ATTR_LLLVARANS,999	},  //Bit 112
	{ATTR_LLLVARANS,999	},  //Bit 113
	{ATTR_LLLVARANS,999	},  //Bit 114
	{ATTR_LLLVARANS,999	},  //Bit 115
	{ATTR_LLLVARANS,999	},  //Bit 116
	{ATTR_LLLVARANS,999	},  //Bit 117
	{ATTR_LLLVARANS,999	},  //Bit 118
	{ATTR_LLLVARANS,999	},  //Bit 119
	{ATTR_LLLVARANS,999	},  //Bit 120
	{ATTR_LLLVARANS,999	},  //Bit 121
	{ATTR_LLLVARANS,999	},  //Bit 122
	{ATTR_LLLVARANS,999	},  //Bit 123
	{ATTR_LLLVARANS,999	},  //Bit 124
	{ATTR_LLLVARANS,999	},  //Bit 125
	{ATTR_LLLVARANS,999	},  //Bit 126
	{ATTR_LLLVARANS,999	},  //Bit 127
	{ATTR_BIN,		64	}   //Bit 128
};


const ISO8583BitAttr SAND_ISO8583BitAttrTab[ISO8583_MAXBITNUM]
={
	{0,				0	},	//Bit 1
	{ATTR_LLVARN,	20	},	//Bit 2
	{ATTR_N,		6	},  //Bit 3
	{ATTR_N,		12	},  //Bit 4
	{ATTR_N,		12	},  //Bit 5
	{ATTR_N,		12	},  //Bit 6
	{ATTR_N,		10	},  //Bit 7
	{ATTR_N,		8	},  //Bit 8
	{ATTR_N,		8	},  //Bit 9
	{ATTR_N,		8	},  //Bit 10
	{ATTR_N,		6	},  //Bit 11
	{ATTR_N,		6	},  //Bit 12
	{ATTR_N,		6	},  //Bit 13
	{ATTR_N,		4	},  //Bit 14
	{ATTR_N,		4	},  //Bit 15
	{ATTR_N,		4	},  //Bit 16
	{ATTR_N,		4	},  //Bit 17
	{ATTR_N,		4	},  //Bit 18
	{ATTR_N,		3	},  //Bit 19
	{ATTR_N,		3	},  //Bit 20
	{ATTR_N,		3	},  //Bit 21
	{ATTR_N,		3	},  //Bit 22
	{ATTR_N,		3	},  //Bit 23
	{ATTR_N,		3	},  //Bit 24
	{ATTR_N,		2	},  //Bit 25
	{ATTR_N,		2	},  //Bit 26
	{ATTR_N,		1	},  //Bit 27
	{ATTR_XN,		9	},  //Bit 28
	{ATTR_XN,		9	},  //Bit 29
	{ATTR_XN,		9	},  //Bit 30
	{ATTR_XN,		9	},  //Bit 31
	{ATTR_LLVARN,	99	},  //Bit 32
	{ATTR_LLVARANS,	11	},  //Bit 33
	{ATTR_LLVARNS,	28	},  //Bit 34
	{ATTR_LLVARANZS,	37	},  //Bit 35
	{ATTR_LLLVARANZS,	104	},  //Bit 36
	{ATTR_AN,		12	},  //Bit 37
	{ATTR_AN,		6	},  //Bit 38
	{ATTR_AN,		4	},  //Bit 39[2] //SAND ISO 4bit
	{ATTR_AN,		3	},  //Bit 40
	{ATTR_ANS,		8	},  //Bit 41
	{ATTR_ANS,		15	},  //Bit 42
	{ATTR_ANS,		40	},  //Bit 43
	{ATTR_ANS,		5	},  //Bit 44  //SAND ISO 5bit
	{ATTR_LLVARANS,	76	},  //Bit 45
	{ATTR_LLLVARANS,999	},  //Bit 46
	{ATTR_AN,	6	},  //Bit 47
	{ATTR_LLLVARN,999	},  //Bit 48
	{ATTR_ANS,		3	},  //Bit 49
	{ATTR_ANS,		3	},  //Bit 50
	{ATTR_ANS,		3	},  //Bit 51
	{ATTR_BIN,		64	},  //Bit 52
	{ATTR_N,		16	},  //Bit 53
	{ATTR_LLLVARANS,999	},  //Bit 54
	{ATTR_LLLVARANS,999	},  //Bit 55
	{ATTR_LLLVARANS,999	},  //Bit 56
	{ATTR_LLLVARANS,999	},  //Bit 57
	{ATTR_LLLVARANS,999	},  //Bit 58
	{ATTR_LLLVARANS,999	},  //Bit 59
	{ATTR_LLLVARN,999	},  //Bit 60
	//{ATTR_LLLVARANS,999	},  //Bit 60
	{ATTR_LLLVARANS, 999	},  //Bit 61
	{ATTR_LLLVARANS, 999	},  //Bit 62
	{ATTR_LLLVARANS,999	},  //Bit 63
	{ATTR_BIN,		64	},  //Bit 64
	{ATTR_BIN,		1	},  //Bit 65
	{ATTR_N,		1	},  //Bit 66
	{ATTR_N,		2	},  //Bit 67
	{ATTR_N,		3	},  //Bit 68
	{ATTR_N,		3	},  //Bit 69
	{ATTR_N,		3	},  //Bit 70
	{ATTR_N,		4	},  //Bit 71
	{ATTR_N,		4	},  //Bit 72
	{ATTR_N,		6	},  //Bit 73
	{ATTR_N,		10	},  //Bit 74
	{ATTR_N,		10	},  //Bit 75
	{ATTR_N,		10	},  //Bit 76
	{ATTR_N,		10	},  //Bit 77
	{ATTR_N,		10	},  //Bit 78
	{ATTR_N,		10	},  //Bit 79
	{ATTR_N,		10	},  //Bit 80
	{ATTR_N,		10	},  //Bit 81
	{ATTR_N,		12	},  //Bit 82
	{ATTR_N,		12	},  //Bit 83
	{ATTR_N,		12	},  //Bit 84
	{ATTR_N,		12	},  //Bit 85
	{ATTR_N,		16	},  //Bit 86
	{ATTR_N,		16	},  //Bit 87
	{ATTR_N,		16	},  //Bit 88
	{ATTR_N,		16	},  //Bit 89
	{ATTR_N,		42	},  //Bit 90
	{ATTR_AN,		1	},  //Bit 91
	{ATTR_AN,		2	},  //Bit 92
	{ATTR_AN,		5	},  //Bit 93
	{ATTR_AN,		7	},  //Bit 94
	{ATTR_AN,		42	},  //Bit 95
	{ATTR_BIN,		64	},  //Bit 96
	{ATTR_XN,		17	},  //Bit 97
	{ATTR_ANS,		25	},  //Bit 98
	{ATTR_LLVARN,	11	},  //Bit 99
	{ATTR_LLVARN,	11	},  //Bit 100
	{ATTR_LLVARANS,	17	},  //Bit 101
	{ATTR_LLVARANS,	28	},  //Bit 102
	{ATTR_LLVARANS,	28	},  //Bit 103
	{ATTR_LLVARANS,	100	},  //Bit 104
	{ATTR_LLLVARANS,999	},  //Bit 105
	{ATTR_LLLVARANS,999	},  //Bit 106
	{ATTR_LLLVARANS,999	},  //Bit 107
	{ATTR_LLLVARANS,999	},  //Bit 108
	{ATTR_LLLVARANS,999	},  //Bit 109
	{ATTR_LLLVARANS,999	},  //Bit 110
	{ATTR_LLLVARANS,999	},  //Bit 111
	{ATTR_LLLVARANS,999	},  //Bit 112
	{ATTR_LLLVARANS,999	},  //Bit 113
	{ATTR_LLLVARANS,999	},  //Bit 114
	{ATTR_LLLVARANS,999	},  //Bit 115
	{ATTR_LLLVARANS,999	},  //Bit 116
	{ATTR_LLLVARANS,999	},  //Bit 117
	{ATTR_LLLVARANS,999	},  //Bit 118
	{ATTR_LLLVARANS,999	},  //Bit 119
	{ATTR_LLLVARANS,999	},  //Bit 120
	{ATTR_LLLVARANS,999	},  //Bit 121
	{ATTR_LLLVARANS,999	},  //Bit 122
	{ATTR_LLLVARANS,999	},  //Bit 123
	{ATTR_LLLVARANS,999	},  //Bit 124
	{ATTR_LLLVARANS,999	},  //Bit 125
	{ATTR_LLLVARANS,999	},  //Bit 126
	{ATTR_LLLVARANS,999	},  //Bit 127
	{ATTR_BIN,		64	}   //Bit 128
};

