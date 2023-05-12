#include <stdio.h>
#include <wdsp/wdsp.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

//#define DEBUG 

#define LMSNR

void SetRXAPreGenRun (int channel, int run);
int RXANBPAddNotch (int channel, int notch, double fcenter, double fwidth, int active);
void RXANBPSetRun (int channel, int run);

int main(int argc, char *argv[]) {
    WDSPwisdom(".");

	int channel_index = 0;
	static const int in_size = 256;
	int dsp_size = 32;
	int input_samplerate = 48000;
	int dsp_rate = 48000;
	int output_samplerate = 48000;
	double tdelayup = 0.1;
	double tslewup = 0.1;
	double tdelaydown = 0.1;
	double tslewdown = 0.1;
	
	
	OpenChannel(channel_index,
		in_size,
		dsp_size,
		input_samplerate,
		dsp_rate,
		output_samplerate,
		0 /** receive **/,
		1 /** on at start **/,
		tdelayup,
		tslewup,
		tdelaydown,
		tslewdown,
		1 /** do not return until data is available **/);


    int16_t * const in_buffer_raw = malloc(sizeof(int16_t) * in_size * 2);
    double * const in_buffer = malloc(sizeof(double) * in_size * 2);
    double * const out_buffer = malloc(sizeof(double) * in_size * 2);
    
    int error = 0;
    int read_result = 0;
    int amount_read = 0;
    int amount_written = 0;
    int i = 0;
    int write_result = 0;
    int running = 1;

//RXANBPAddNotch (channel_index, 0, 10000.0, 7000.0, 1);

SetRXAShiftRun(channel_index, 0);
RXANBPSetRun(channel_index, 0);
SetRXAAMSQRun(channel_index, 0);
SetRXAAGCMode (channel_index, 0);
SetRXAMode(channel_index, 2);


    while(running){
        read_result = read(0, ((char*)in_buffer_raw) + amount_read, (sizeof(int16_t) * in_size *2) - amount_read);
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
        if (amount_read != sizeof(int16_t) * in_size * 2){
            continue;
        }
        
        amount_read = 0;
        // convert to doubles in I-Q
        for(i = 0; i != in_size * 2; i++) {
            in_buffer[i] = ((double)in_buffer_raw[i]) / 32768.0;
//            in_buffer[(i*2)+1] = 0.0; // fake I-Q data
        }
        

#ifdef LMSNR
        fexchange0(channel_index, in_buffer, out_buffer, &error);
        if (error != 0) {
            fprintf(stderr, "error is %d\n", error);
            running = 0;
            break;
        }
#else
        for(i = 0; i != in_size; i++) {
            out_buffer[i*2] = in_buffer[i*2];
            out_buffer[(i*2) + 1] = in_buffer[(i*2)+1];
        }
#endif
        
        // should have the same number of samples in the output
        for(i = 0; i != in_size; i++){
            double out_value = out_buffer[i * 2] * 32768.0;
            in_buffer_raw[i] = (int16_t)out_value;
        }
        
        amount_written = 0;
        while(amount_written != sizeof(int16_t) * in_size) {
            write_result = write(1, ((char*)in_buffer_raw) + amount_written, (sizeof(int16_t) * in_size) - amount_written);
            if (write_result < 0) {
                running = 0;
                break;
            }
            amount_written += write_result;
        }
    }
    

	CloseChannel(channel_index);
	
	free(in_buffer);
	free(out_buffer);
	return 0;
}
