#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "bmp.h"
#include "stego.h"

int process_rcode(const int rcode)
{
	// process_rcode: accept rcode and do nothing (in case of 0)
	// or print error message and exit. Be sure to free all
	// allocated memory before calling process_rcode
	switch (rcode)
	{
		case 0:
			break;
		case NARGO:
			printf("BMP Crop Rotate Stegano tool\n"
                    "\n"
                    "Usage: \n"
					"bmp_tool crop-rotate ‹in-bmp› ‹out-bmp› ‹x› ‹y› ‹w› ‹h›\n"
					"bmp_tool insert ‹in-bmp› ‹out-bmp› ‹key-txt› ‹msg-txt›\n"
					"bmp_tool extract ‹in-bmp› ‹key-txt› ‹msg-txt›\n");
			break;
		case NALLO:
			printf("unable to allocate memory\n");
			break;
		case NOPER:
			printf("unable to open input file\n");
			break;
		case NOPEW:
			printf("unable to open output file\n");
			break;
		case NOPEK:
			printf("unable to open key file\n");
			break;
		case NOPEM:
			printf("unable to open message file\n");
			break;
		case NREAD:
			printf("unable to read input file\n");
			break;
		case NWRIT:
			printf("unable to write to output file\n");
			break;
		case NKEYR:
			printf("unable to read key file\n");
			break;
		case NMESR:
			printf("unable to read message file\n");
			break;
		case NMESW:
			printf("unable to write to message file\n");
			break;
		case NFORM:
			printf("wrong bmp format\n");
			break;
		case NCROP:
			printf("wrong crop options\n");
			break;
		case NMESS:
			printf("wrong message letters, only [A-Z,. ] are allowed\n");
			break;
		case NKEYF:
			printf("wrong key format, correct is \n"
					"x_0 y_0 (R | G | B)\n"
					"...\n"
					"x_m y_m (R | G | B)\n");
			break;
		case NWKEY:
			printf("wrong coordinates in key\n");
			break;
		case NENHS:
			printf("not enough keys to encode message\n");
			break;
		case NMUL5:
			printf("key is not a multiple of 5, so unable to decode message correctly\n");
			break;
		case NWSYM:
			printf("unable to decode message, probably wrong key?\n");
			break;

			printf("unknown error\n");
	}
	return rcode;
}

int main(int argc, char **argv)
{
	int rcode = 0;
	if (argc != 5 && argc != 6 && argc != 8)
	{
		rcode = NARGO;
		return process_rcode(rcode);
	}

	bmp image;

	rcode = load_bmp(&image, argv[2]);
	if (rcode != 0)
		return process_rcode(rcode);

	int do_crop_rotate = (strcmp(argv[1], "crop-rotate") == 0);
	int do_insert = (strcmp(argv[1], "insert") == 0);
	int do_extract = (strcmp(argv[1], "extract") == 0);
	if (do_crop_rotate && argc == 8)
	{
		int x = atoi(argv[4]);
		int y = atoi(argv[5]);
		int w = atoi(argv[6]);
		int h = atoi(argv[7]);

		rcode = crop(&image, x, y, w, h);
		if (rcode != 0)
		{
			free_data(&image.data, image.biHeight);
			return process_rcode(rcode);
		}

		rcode = rotate(&image);
		if (rcode != 0)
		{
			free_data(&image.data, image.biHeight);
			return process_rcode(rcode);
		}
	}
	else if (do_insert && argc == 6)
	{
		rcode = insert(&image, argv[4], argv[5]);
		if (rcode != 0)
		{
			free_data(&image.data, image.biHeight);
			return process_rcode(rcode);
		}
	}
	else if (do_extract && argc == 5)
	{
		rcode = extract(&image, argv[3], argv[4]);
		if (rcode != 0)
		{
			free_data(&image.data, image.biHeight);
			return process_rcode(rcode);
		}
	}
	else
	{
		return process_rcode(NARGO);
	}

	if (do_crop_rotate || do_insert)
	{
		rcode = save_bmp(&image, argv[3]);
		if (rcode != 0)
			return process_rcode(rcode);
	}
	free_data(&image.data, image.biHeight);

	return 0;
}
