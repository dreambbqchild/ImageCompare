#pragma once
void ZeroOutLowNibbleAVX(unsigned char* bytes, int length);
void ConvertBytesToShortsAVX(unsigned char* bytes, short* shorts, int length);
float CalcAverageDiffBetweenAVX(short* lShorts, short* rShorts, int length);