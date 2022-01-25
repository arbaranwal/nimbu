#include "fft_engine.h"

int nfft = 64;
bool readyForFFT = false;
kiss_fft_cpx cin[64];
kiss_fft_cpx cout[64];
kiss_fft_cfg kiss_fft_state;
uint8_t fftValues[MAX_FFT_SRC_NUM] = {0};

void create_fft()
{
    for(;;)
    {
        if(readyForFFT)
        {
            kiss_fft_state = kiss_fft_alloc(nfft,0,0,0);
            kiss_fft_cfg cfg = kiss_fft_alloc( nfft, 0, 0, 0 );
            for(uint8_t i = 0; i < nfft; i++)
            {
                cin[i].r = bltData[i];
                cin[i].i = 0;
            }
            printf("esp_get_free_heap_size: %f KiB\n", (float)esp_get_free_heap_size()/1024);
            kiss_fft( cfg , cin , cout );
            printf("DATA\n");
            for(uint8_t i = 0; i < nfft; i++)
            {
                printf("%lf %lf", cout[i].r, cout[i].i);
                printf("\n");
            }
            printf("DATA END\n");
            freezeBLTData = false;
            kiss_fft_free(cfg);
        }
    }
}