 vStart;
		hStart = hTotal - 3;
		hEnd = hTotal - 2;
		hBlankEnd = hTotal + 4;
	}

	newmode.crtc[0x0] = Set8Bits (hTotal); 
	newmode.crtc[0x1] = Set8Bits (hDisplay);
	newmode.crtc[0x2] = Set8Bits (hBlankStart);
	newmode.crtc[0x3] = SetBitField (hBlankEnd, 4: 0, 4:0) | SetBit (7);
	newmode.crtc[0x4] = Set8Bits (hStart);
	newmode.crtc[0x5] = SetBitField (hBlankEnd, 5: 5, 7:7)
		| SetBitField (hEnd, 4: 0, 4:0);
	newmode.crtc[0x6] = SetBitField (vTotal, 7: 0, 7:0);
	newmode.crtc[0x7] = SetBitField (vTotal, 8: 8, 0:0)
		| SetBitField (vDisplay, 8: 8, 1:1)
		| SetBitField (vStart, 8: 8, 2:2)
		| SetBitField (vBlankStart, 8: 8, 3:3)
		| SetBit (4)
		| SetBitField (vTotal, 9: 9, 5:5)
		| SetBitField (vDisplay, 9: 9, 6:6)
		| SetBitField (vStart, 9: 9, 7:7);
	newmode.crtc[0x9] = SetBitField (vBlankStart, 9: 9, 5:5)
		| SetBit (6);
	newmode.crtc[0x10] = Set8Bits (vStart);
	newmode.crtc[0x11] = SetBitField (vEnd, 3: 0, 3:0)
		| SetBit (5);
	newmode.