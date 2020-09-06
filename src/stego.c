#include <stdio.h>
#include <malloc.h>
#include <inttypes.h>
#include <math.h>
#include <assert.h>

#include "bmp.h"

int read_next_key(FILE *key_file, int32_t *x, int32_t *y,
		int8_t *ch_n, const int32_t W, const int32_t H)
{
	assert(key_file != NULL);

	char ch;
	int read = fscanf(key_file, "%d %d %c", x, y, &ch);
	if (read != 3)
	{
		if (feof(key_file))
			return EOF;
		else
			return NKEYR;
	}
	if (!(0 <= *x && *x < W) || !(0 <= *y && *y < H))
		return NWKEY;

	//channel to write secret bit. channels have BGR order in BMP files
	if (ch == 'R')
		*ch_n = 2;
	else if (ch == 'G')
		*ch_n = 1;
	else if (ch == 'B')
		*ch_n = 0;
	else
		return NKEYF;

	return 0;
}

int read_next_chr(FILE *msg_file, uint8_t *bits)
{
	assert(msg_file != NULL);
	assert(bits != NULL);

	char c;
	int read = fscanf(msg_file, "%c", &c);
	if (read != 1)
	{
		if (feof(msg_file))
			return EOF;
		else
			return NMESR;
	}

	uint8_t ch_n; //char number in range 0-28

	if ('A' <= c && c <= 'Z')
		ch_n = c - 65;
	else if (c == ' ')
		ch_n = 26;
	else if (c == '.')
		ch_n = 27;
	else if (c == ',')
		ch_n = 28;
	else
		return NMESS;

	for (int i = 0;i < 5;++i)
		bits[i] = (ch_n >> i) & 1;
	return 0;
}

int write_next_chr(FILE *msg_file, const uint8_t *bits)
{
	assert(msg_file != NULL);
	assert(bits != NULL);

	uint8_t ch_n = 0;
	for (int i = 0;i < 5;++i)
		ch_n = (ch_n << 1) + bits[5-1-i];

	char c = 0;
	if (ch_n < 26)
		c = ch_n + 65;
	else if (ch_n == 26)
		c = ' ';
	else if (ch_n == 27)
		c = '.';
	else if (ch_n == 28)
		c = ',';
	else
	{
		c = '\0';
		//printf '\0' do nothing
		return NWSYM;
	}

	int wrote = fprintf(msg_file, "%c", c);
	if (wrote < 0)
	{
		return NMESW;
	}

	return 0;
}

int open_files(const char *key_filename, const char *msg_filename,
		FILE **key_file, FILE **msg_file, const char* mode)
{
	assert(key_filename != NULL);
	assert(msg_filename != NULL);
	assert(mode != NULL);

	*key_file = fopen(key_filename, "r");
	if (key_file == NULL)
		return NOPEK;

	*msg_file = fopen(msg_filename, mode);
	if (msg_file == NULL)
		return NOPEM;

	return 0;
}

//true_x, true_y	from functions
//x,y,ch		position of byte in file
//h,wb, sgn		check of coordinates' correctness
//			and transofrming in case of negative h
int coord_process(int32_t *true_x, int32_t *true_y, const int32_t x, const int32_t y,
		const int8_t ch, const int32_t h, const int32_t wb, const int8_t sgn)
{
	*true_x = x*3;
	if (sgn < 0)
		(*true_y) = y;
	else
		(*true_y) = h - 1 - y;

	if (!(0 <= (*true_y) && (*true_y) < h)
			|| !(0 <= (*true_x) + ch && (*true_x) + ch < wb))
		return NWKEY;

	return 0;
}

int insert(bmp *image, const char *key_filename, const char *msg_filename)
{
	assert(image != NULL);

	FILE *key_file, *msg_file;
	open_files(key_filename, msg_filename, &key_file, &msg_file, "r");

	int32_t x = -1, y = -1;
	int8_t ch = -1;
	uint8_t bits[5];
	int rcode = 0;
	int done = 0;
	while ((rcode = read_next_chr(msg_file, bits)) == 0)
	{
		++done;
		for (int i = 0;i < 5;++i)
		{
			rcode = read_next_key(key_file, &x, &y, &ch,
					image->biWidth, image->biHeight);
			if (rcode != 0)
			{
				if (rcode == EOF)
					return NENHS;
				else
					return rcode;
			}

			int32_t true_x, true_y;
			rcode = coord_process(&true_x, &true_y, x, y, ch, image->biHeight,
					(int32_t) image->biByteWidth, image->biSgnHeight);

			if (rcode != 0)
				return rcode;

			int8_t byte = image->data[true_y][true_x+ch];
			if (bits[i] == 0)
				byte &= ~1;
			else
				byte |= 1;
			image->data[true_y][true_x+ch] = byte;
		}
	}

	if (rcode != EOF)
		return rcode;

	return 0;
}

int extract(const bmp *image, const char *key_filename, const char *msg_filename)
{
	assert(image != NULL);

	FILE *key_file, *msg_file;
	open_files(key_filename, msg_filename, &key_file, &msg_file, "w");

	int32_t x = -1, y = -1;
	int8_t ch = -1;
	uint8_t bits[5];

	int finish = 0;
	int rcode = 0;
	while (finish != 1)
	{
		for (int i = 0;i < 5;++i)
		{
			rcode = read_next_key(key_file, &x, &y, &ch,
					image->biWidth, image->biHeight);
			if (rcode == EOF)
			{
				if (i != 0)
					return NMUL5;

				finish = 1;
				rcode = 0;
				break;
			}
			else if (rcode != 0)
				return rcode;

			int32_t true_x, true_y;
			rcode = coord_process(&true_x, &true_y, x, y, ch, image->biHeight,
					(int32_t) image->biByteWidth, image->biSgnHeight);

			if (rcode != 0)
				return rcode;

			int8_t byte = image->data[true_y][true_x+ch];
			bits[i] = byte & 1;
		}
		if (finish != 1 && (rcode = write_next_chr(msg_file, bits)) != 0)
			break;
	}
	return rcode;
}

