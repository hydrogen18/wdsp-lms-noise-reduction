#include <stdio.h>

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include "ANR.c"

//#define DEBUG 


int main(int argc, char *argv[]) {
    char * endptr = NULL;
    char * gainStr = getenv("LMS_GAIN");
    char * tapsStr = getenv("LMS_TAPS");
    char * delayStr = getenv("LMS_DELAY");
    char * leakageStr = getenv("LMS_LEAKAGE");
    
    if (gainStr == NULL) {
        gainStr = "8.0e-5";
    }

    float gain = strtof(gainStr, &endptr);
    if (endptr == gainStr) {
        fprintf(stderr, "cannot parse gain value %s\n", gainStr);
        return 1;
    }
    
    if (tapsStr == NULL) {
        tapsStr = "40";
    }
    
    float taps = strtof(tapsStr, &endptr);
    if (endptr == tapsStr) {
        fprintf(stderr, "cannot parse taps value %s\n", tapsStr);
        return 1;
    }
    
    if (delayStr == NULL) {
        delayStr = "16";
    }
    float delay = strtof(delayStr, &endptr);
    if (endptr == delayStr) {
        fprintf(stderr, "cannot parse delay value %s\n", delayStr);
        return 1;
    }
    
    if (leakageStr == NULL) {
        leakageStr = "1.60e+1";
    }
    float leakage = strtof(leakageStr, &endptr);
    if (endptr == leakageStr) {
        fprintf(stderr, "cannot parse leakage value %s\n", leakageStr);
        return 1;
    }
    


    TYPEREAL params[NOISE_PARAMS];
    params[NR_GAIN] = gain;
    params[NR_TAPS] = taps;
    params[NR_DLY] = delay;
    params[NR_LEAKAGE] = leakage;
    int rx_chan = 0;
    wdsp_ANR_init(rx_chan, NR_DENOISE, params);


	

    int in_size = 12000;
    TYPEMONO16 * const in_buffer_raw = malloc(sizeof(TYPEMONO16) * in_size);
    TYPEMONO16 * const out_buffer_raw = malloc(sizeof(TYPEMONO16) * in_size);
    
    int read_result = 0;
    int amount_read = 0;
    int amount_written = 0;
    int write_result = 0;
    int running = 1;


    while(running){
        read_result = read(0, ((char*)in_buffer_raw) + amount_read, (sizeof(TYPEMONO16) * in_size) - amount_read);
        if (read_result == 0){
            running = 0; // eof reached
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
