#ifndef BMP_H
#define BMP_H

#include <inttypes.h>

#define NARGO 10
#define NALLO 11

#define NOPER 20
#define NOPEW 21
#define NOPEK 22
#define NOPEM 23

#define NREAD 30
#define NWRIT 31
#define NKEYR 32
#define NMESR 33
#define NMESW 34

#define NFORM 40
#define NCROP 41
#define NMESS 42
#define NKEYF 43
#define NWKEY 44
#define NKEYC 45
#define NENHS 46
#define NMUL5 47
#define NWSYM 48

#define HEADER_SIZE 54

struct bmp //core type 3
{
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;

	uint32_t biSize;
	int32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t biXPelsPerMeter;
	int32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;

	//non-format variables
	uint32_t biByteHeight;
	uint32_t biByteWidth;
	int8_t biSgnHeight;

	uint8_t **data;
};

typedef struct bmp bmp;

int32_t _2comp_to_signed (const uint32_t x);
uint32_t _signed_to_2comp (const int32_t x);

void b16to8 (const uint32_t v16, uint8_t *v8);
void b32to8 (const uint32_t v32, uint8_t *v8);
uint16_t b8to16 (uint8_t *v8);
uint32_t b8to32 (uint8_t *v8);

int alloc_data (uint8_t ***data, const int32_t height, const int32_t width);
void free_data (uint8_t ***data, const int32_t height);
void recount (bmp *image);

int read_header (bmp *image, uint8_t *data);
void write_header(bmp *image, uint8_t *data);

int load_bmp (bmp *image, const char *filename);
int crop (bmp *image, const int x, const int y, const int w, const int h);
int rotate (bmp *image);
int save_bmp (bmp *image, const char *filename);

#endif
