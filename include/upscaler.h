#ifndef __UPSCALER_H__
#define __UPSCALER_H__

void scale2x(int* input_img, int* output_img, int width, int height);
void scale3x(int* input_img, int* output_img, int width, int height);
void hq2x(int* input_img, int* output_img, int width, int height);
void hq3x(int* input_img, int* output_img, int width, int height);

#endif