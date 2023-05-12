#include <stdio.h>

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include "ANR.c"

//#define DEBUG 

#define LMSNR


int main(int argc, char *argv[]) {
    TYPEREAL params[NOISE_PARAMS];
    params[NR_GAIN] = 8.0e-5 ;
    params[NR_TAPS] = 40.0;
    params[NR_DLY] = 16.0;
    params[NR_LEAKAGE] = 1.60e+1;
    int rx_chan = 0;
    wdsp_ANR_init(rx_chan, NR_DENOISE, params);

	int input_samplerate = 48000;
	int dsp_rate = 48000;
	int output_samplerate = 48000;
	

    int in_size = 12000;
    TYPEMONO16 * const in_buffer_raw = malloc(sizeof(TYPEMONO16) * in_size);
    TYPEMONO16 * const out_buffer_raw = malloc(sizeof(TYPEMONO16) * in_size);
    
    int error = 0;
    int read_result = 0;
    int amount_read = 0;
    int amount_written = 0;
    int i = 0;
    int write_result = 0;
    int running = 1;


    while(running){
        read_result = read(0, ((char*)in_buffer_raw) + amount_read, (sizeof(TYPEMONO16) * in_size) - amount_read);
        if (read_result == 0){
            running = 0;
        } else if (read_result < 0) {
            running = 0;
            break;
        }

#ifdef DEBUG
        fprintf(stderr, "read %d bytes\n", read_result);
#endif
        amount_read += read_result;
        if (amount_read != sizeof(TYPEMONO16) * in_size){
            continue;
        }
        
        amount_read = 0;        
        wdsp_ANR_filter(rx_chan, NR_DENOISE, in_size, in_buffer_raw, out_buffer_raw);
        
        // should have the same number of samples in the output

        
        amount_written = 0;
        while(amount_written != sizeof(int16_t) * in_size) {
            write_result = write(1, ((char*)out_buffer_raw) + amount_written, (sizeof(int16_t) * in_size) - amount_written);
            if (write_result < 0) {
                running = 0;
                break;
            }
            amount_written += write_result;
        }
    }
    
	return 0;
}
