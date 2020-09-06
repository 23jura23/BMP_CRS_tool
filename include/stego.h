#ifndef STEGO_H
#define STEGO_H

#include <stdio.h>
#include <inttypes.h>

#include "bmp.h"

int read_next_key(FILE *key_file, int32_t *x, int32_t *y, int8_t *ch_n, int32_t W, int32_t H);
int read_next_chr(FILE *msg_file, uint8_t **bits);
int write_next_chr(FILE *msg_file, uint8_t *bits);

int open_files(const char *key_filename, const char *msg_filename,
		FILE **key_file, FILE **msg_file, const char* mode);
int coord_process(int32_t *true_x, int32_t *true_y, const int32_t x, const int32_t y,
		const int8_t ch, const int32_t h, const int32_t wb, const int8_t sgn);

int insert(bmp *image, char *key_filename, char *msg_filename);
int extract(bmp *image, char *key_filename, char *msg_filename);

#endif
