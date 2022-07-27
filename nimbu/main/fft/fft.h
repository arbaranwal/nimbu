#ifndef __FFT__
#define __FFT__

typedef char byte;

float sine(int i);
float cosine(int i);
void FFT(int in[], int N, float freq);

#endif // __FFT__
