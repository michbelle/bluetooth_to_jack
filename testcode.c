#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int bits=32;
int SAMPLE_PER_CYCLE =36000/100;
double PI=3.1415;
FILE *fp;
int main()
{

	fp=fopen("data.txt","w" );
    int *samples_data = malloc(((bits+8)/16)*SAMPLE_PER_CYCLE*4);
    unsigned int i, sample_val;
    double sin_float, triangle_float=0.0, triangle_step = (double) pow(2, bits) / SAMPLE_PER_CYCLE;

    triangle_float = -(pow(2, bits)/2 - 1);

    for(i = 0; i < SAMPLE_PER_CYCLE; i++) {
        sin_float = sin(i * 2 * PI / SAMPLE_PER_CYCLE);
        
        if(sin_float >= 0)
            triangle_float += triangle_step;
        else
            triangle_float -= triangle_step;

        

        sin_float *= (pow(2, bits)/2 - 1);

        if (bits == 16) {
            sample_val = 0;
            sample_val += (short)triangle_float;
            sample_val = sample_val << 16;
            sample_val += (short) sin_float;
            samples_data[i] = sample_val;
            
        } else if (bits == 24) { //1-bytes unused
            samples_data[i*2] = ((int) triangle_float) << 8;
            samples_data[i*2 + 1] = ((int) sin_float) << 8;
            
        } else {
            samples_data[i*2] = ((int) triangle_float);
            samples_data[i*2 + 1] = ((int) sin_float);
        }

        fprintf(fp, "%d\t%d\t%d\n", i, samples_data[i*2] , samples_data[i*2+1]);

    }
int fclose( FILE *fp );
}
