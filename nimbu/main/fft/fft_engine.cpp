#include "fft_engine.h"
void FFT(int in[],int N,float Frequency);

int nfft = 64;
bool readyForFFT = true;
int *validFFTData;
uint8_t fftValues[MAX_FFT_SRC_NUM] = {0};

void fft_task_start_up()
{
    ESP_LOGI(FFT_TAG, "Starting FFT engine");
    xTaskCreate((TaskFunction_t)do_fft, "do_fft", 4096, NULL, tskIDLE_PRIORITY, &s_fft_task_handle);
}

void do_fft()
{
    for(;;)
    {
        if(freezeBLTData)   // only run if BLTData is frozen
        {
            // deassert readyForFFT
            readyForFFT = false;
            // process data
            FFT(validFFTData,64,22050);
            // printf("S\n");
            // for (int i = 0; i < 5; i++)
            //     printf("%f\n", f_peaks[i]);
            // printf("E\n");
            // assert ready
            readyForFFT = true;
        }
    }
}