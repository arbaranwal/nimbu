#include "fft_engine.h"

int nfft = 64;
bool readyForFFT = false;
uint8_t fftValues[MAX_FFT_SRC_NUM] = {0};

void create_fft()
{
    for(;;)
    {
        if(readyForFFT)
        {
            // alloc FFT and configure
            printf("esp_get_free_heap_size: %f KiB\n", (float)esp_get_free_heap_size()/1024);
            // do FFT
            printf("DATA\n");
            for(uint8_t i = 0; i < nfft; i++)
            {
                // printf("%lf %lf", cout[i].r, cout[i].i);
                printf("\n");
            }
            printf("DATA END\n");
            freezeBLTData = false;
            // free data used by FFT
        }
    }
}