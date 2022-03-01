//
// Created by Bhavesh on 06/02/22.
//
#include <iostream>
#include <complex>

//FFT constants pi and  i
const double pi = std::acos(-1);
const std::complex<double> i(0, 1);

//N is a power of 2
//X is complex sample array if SIZE N
std::complex<double>* fft(int N, std::complex<double> X[N]){
    if (N == 1) {
        return X;
    }
    else {
        std::complex<double> X_even[N/2];
        std::complex<double> X_odd[N/2];

        for(int i=0; i<N; i++){
            if(i%2 == 1){
                X_odd[i/2] = X[i];
            } else {
                X_even[i/2] = X[i];
            }
        }
        std::complex<double>* pointer_even = fft(N/2,X_even);
        std::complex<double>* pointer_odd  = fft(N/2,X_odd);

        for(int i=0;i<N/2;i++) {
            X_even[i] = pointer_even[i];
            X_odd[i] = pointer_odd[i];
        }

        std::complex<double> factor[N];
        for(int j=0;j<N;j++) {
            factor[j] = std::exp((double)j*(-2)*i*pi/(double)N);
        }
        for(int i = 0;i < N;i++) {
            X[i] = X_even[i%(N/2)] + factor[i]*X_odd[i];
        }
        return X;
    }
}

