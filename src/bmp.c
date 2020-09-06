#include <stdio.h>
#include <malloc.h>
#include <inttypes.h>
#include <math.h>
#include <assert.h>

#include "bmp.h"

// le = little-endian

int32_t _2comp_to_signed (const uint32_t x)
{
	if ((x >> (4*8 - 1)) == 1) //byte = 8 bits
		return -(int32_t)(~x + 1);
	else
		return (int32_t) x;
}

uint32_t _signed_to_2comp (const int32_t x)
{
	if (x < 0)
		return ~(-x) + 1;
	else
		return (uint32_t) x;
}

uint16_t b8to16_le (const uint8_t *v8)
{
	assert(v8 != NULL);

	uint16_t v16 = 0;
	int nbytes = 16/8;
	for (int i = 0; i < nbytes; ++i)
		v16 = (v16 << 8) | v8[nbytes - 1 - i];
	return v16;
}

uint32_t b8to32_le (const uint8_t *v8)
{
	assert(v8 != NULL);

	uint32_t v32 = 0;
	int nbytes = 32/8;
	for (int i = 0; i < nbytes; ++i)
		v32 = (v32 << 8) | v8[nbytes - 1 - i];
	return v32;
}

void b16to8_le (const uint32_t v16, uint8_t *v8)
{
	assert(v8 != NULL);		

	int nbytes = 16/8;
	uint32_t v16_c = v16;
	for (int i = 0; i < nbytes; ++i)
	{
		v8[i] = v16_c & ((1 << 8) - 1);
		v16_c >>= 8;
	}
}

void b32to8_le (const uint32_t v32, uint8_t *v8)
{
	assert(v8 != NULL);		

	int nbytes = 32/8;
	uint32_t v32_c = v32;
	for (int i = 0; i < nbytes; ++i)
	{
		v8[i] = v32_c & ((1 << 8) - 1);
		v32_c >>= 8;
	}
}

int read_header (bmp *image, uint8_t *data)
{
	assert(data != NULL);
	assert(image != NULL);

	uint32_t buf; //for signed
	image->bfType		= b8to16_le(data+0);
	image->bfSize		= b8to32_le(data+2);
	image->bfReserved1	= b8to16_le(data+6);
	image->bfReserved2	= b8to16_le(data+8);
	image->bfOffBits	= b8to32_le(data+10);

	image->biSize		= b8to32_le(data+14);

	buf			= b8to32_le(data+18);
	image->biWidth	= _2comp_to_signed(buf);
	//though, it is guaranteed to be positive...

	image->biSgnHeight = 1;
	buf			= b8to32_le(data+22);
	image->biHeight		= _2comp_to_signed(buf);
	if (image->biHeight < 0)
	{
		image->biHeight *= -1;
		image->biSgnHeight *= -1;
	}

	image->biPlanes		= b8to16_le(data+26);
	image->biBitCount	= b8to16_le(data+28);
	image->biCompression	= b8to32_le(data+30);
	image->biSizeImage	= b8to32_le(data+34);

	buf			= b8to32_le(data+38);
	image->biXPelsPerMeter	= _2comp_to_signed(buf);

	buf			= b8to32_le(data+42);
	image->biYPelsPerMeter	= _2comp_to_signed(buf);

	image->biClrUsed	= b8to32_le(data+46);
	image->biClrImportant	= b8to32_le(data+50);

	if (image->bfType != 0x4D42) // 4D42 - first bytes of bmp header
		return NFORM;
	if (image->biSize != 40) // guaranteed by v3 of DIB
		return NFORM;
	if (image->biBitCount != 24) //24 bits per pixes
		return NFORM;
	if (image->biClrUsed != 0) //no color table
		return NFORM;
	if (image->biCompression != 0) //no compression
		return NFORM;

	return 0;
}

int alloc_data (uint8_t ***data, const int32_t height, const int32_t width)
{
	int rcode = 0;
	(*data) = malloc(height * sizeof(uint8_t *));
	if ((*data) == NULL)
		return NALLO;

	int32_t al = 0; //al = already alloced
	for (; al < height; ++al)
	{
		(*data)[al] = calloc(width, sizeof(uint8_t));
		if ((*data)[al] == NULL)
		{
			rcode = NALLO;
			break;
		}
	}
	if (rcode == NALLO)
	{
		for (int32_t i = 0; i < al; ++i)
			free((*data)[i]);
		free((*data));
		return rcode;
	}
	return 0;
}

void free_data (uint8_t ***data, const int32_t height)
{
	for (int32_t i = 0; i < height; ++i)
		free((*data)[i]);
	free(*data);
}

void recount (bmp *image)
{
	image->biByteWidth = image->biWidth*3 + (4 - image->biWidth*3 % 4) % 4;
	image->biByteHeight = (image->biHeight)*3;
	image->bfSize = 14 + image->biSize + image->biHeight * image->biByteWidth;
	image->biSizeImage = image->biHeight * image->biByteWidth;
}

int load_bmp (bmp *image, const char *filename)
{
	assert(filename != NULL);
	assert(image != NULL);

	size_t read = 0;
	FILE * in = fopen(filename,"rb");
	if (in == NULL)
		return NOPER;
	uint8_t header_buf[HEADER_SIZE];
	read = fread(header_buf, sizeof(uint8_t), HEADER_SIZE, in);
	if (read < HEADER_SIZE)
	{
		fclose(in);
		return NREAD;
	}

	int rcode = read_header(image, header_buf);
	if (rcode != 0)
	{
		fclose(in);
		return rcode;
	}

	recount(image);

	rcode = alloc_data(&image->data, image->biHeight, image->biByteWidth);
	if (rcode != 0)
	{
		fclose(in);
		return rcode;
	}

	for (int32_t i = 0; i < image->biHeight; ++i)
	{
		read = fread(image->data[i], sizeof(int8_t),image->biByteWidth, in);
		if (read < image->biByteWidth)
		{
			rcode = NREAD;
			break;
		}
	}

	if (rcode == NREAD)
	{
		free_data(&image->data,image->biHeight);
		fclose(in);
		return rcode;
	}
	
	fclose(in);
	return 0;
}

int crop (bmp *image, const int x, const int y, const int w, const int h)
{
	assert(image != NULL);
	
	if (!(0 <= x && x < x + w && x + w <= image->biWidth) || 
			!(0 <= y && y < y + h && y + h <= image->biHeight))
		return NCROP;
	
	int32_t lcx = x*3, lcy, rcx = (x+w)*3, rcy; // left/right-corner x/y
	
	if (image->biSgnHeight < 0)
	{
		lcy = y;
		rcy = y+h;
	}
	else
	{
		lcy = image->biHeight - (y+h);
		rcy = image->biHeight - y;
	}

	image->biWidth = (int32_t) w;
	int32_t oldHeight = image->biHeight;
	image->biHeight = (int32_t) h;
	recount(image);

	uint8_t **pro_data;
	int rcode = alloc_data(&pro_data, image->biHeight, image->biByteWidth);
	if (rcode != 0)
	{
		free_data(&pro_data, image->biHeight);
		return rcode;
	}
	for (int32_t y_it = lcy; y_it < rcy; ++y_it)
	{
		for (int32_t x_it = lcx; x_it < rcx; ++x_it)
		{
			pro_data[y_it - lcy][x_it - lcx] = image->data[y_it][x_it];
		}
	}
	
	free_data(&image->data, oldHeight);
	image->data = pro_data;

	return 0;
}

int rotate (bmp *image)
{
	uint8_t **pro_data;
	image->biByteHeight = image->biHeight * 3 + (4 - image->biHeight*3 % 4)%4;
	int rcode = alloc_data(&pro_data, image->biWidth, image->biByteHeight);
	if (rcode != 0)
	{
		free_data(&pro_data, image->biHeight);
		return rcode;
	}

	for (int32_t y_it = 0; y_it < image->biHeight; ++y_it)
	{
		for (int32_t x_it = 0; x_it < image->biWidth; ++x_it)
		{
			if ((uint32_t) x_it * 3 < image->biByteWidth)
				for (uint32_t pix_it = 0; pix_it < 3; ++pix_it) 
				{
					if (image->biSgnHeight < 0)
						pro_data[x_it][y_it * 3 + pix_it] = image->data[image->biHeight - 1 - y_it][x_it * 3 + pix_it];	
					else
						pro_data[image->biWidth-1-x_it][y_it * 3 + pix_it] = image->data[y_it][x_it * 3 + pix_it];	
				}
		}
	}

	free_data(&image->data, image->biHeight);
	image->data = pro_data;
	
	int32_t swap = image->biHeight;
	image->biHeight = image->biWidth;
	image->biWidth = swap;
	
	recount(image);

	return 0;
}

void write_header(bmp *image, uint8_t *data)
{
	assert(data != NULL);
	assert(image != NULL);

	b16to8_le(image->bfType,data);
	b32to8_le(image->bfSize,data+2);
	b16to8_le(image->bfReserved1,data+6);
	b16to8_le(image->bfReserved2,data+8);
	b32to8_le(image->bfOffBits,data+10);
	
	b32to8_le(image->biSize,data+14);
	b32to8_le(_signed_to_2comp(image->biWidth),data+18);
	b32to8_le(_signed_to_2comp(image->biSgnHeight * image->biHeight),data+22);
	b16to8_le(image->biPlanes,data+26);
	b16to8_le(image->biBitCount,data+28);
	b32to8_le(image->biCompression,data+30);
	b32to8_le(image->biSizeImage,data+34);
	b32to8_le(_signed_to_2comp(image->biXPelsPerMeter),data+38);
	b32to8_le(_signed_to_2comp(image->biYPelsPerMeter),data+42);
	b32to8_le(image->biClrUsed,data+46);
	b32to8_le(image->biClrImportant,data+50);
}


int save_bmp (bmp *image, const char *filename)
{
	FILE * out = fopen(filename,"wb");
	if (out == NULL)
		return NOPEW;
	
	uint8_t header_buf[HEADER_SIZE];
	write_header(image, header_buf);
	int wrote = fwrite(header_buf,sizeof(uint8_t),HEADER_SIZE,out);
	if (wrote < HEADER_SIZE)
	{
		fclose(out);
		return NWRIT;
	}
	
	for (int32_t y_it = 0; y_it < image->biHeight; ++y_it)
	{
		wrote = fwrite(image->data[y_it], sizeof(uint8_t), image->biByteWidth, out);
		if (wrote < (int32_t) image->biByteWidth)
		{
			fclose(out);
			return NWRIT;
		}
	}
		
	fclose(out);
	return 0;
}

