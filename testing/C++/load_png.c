#include <stdlib.h>
#include <assert.h>

#include "png.h"

#define PNG_SIG_BYTES 8
typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned int      uint32_t;
typedef unsigned long int uint64_t;

char * load_png(char *name, int *width, int *height) {
	FILE *png_file = fopen(name, "rb");
	assert(png_file);

	uint8_t header[PNG_SIG_BYTES];

	fread(header, 1, PNG_SIG_BYTES, png_file);
	assert(!png_sig_cmp(header, 0, PNG_SIG_BYTES));

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	assert(png_ptr);

	png_infop info_ptr = png_create_info_struct(png_ptr);
	assert(info_ptr);

	png_infop end_info = png_create_info_struct(png_ptr);
	assert(end_info);

	assert(!setjmp(png_jmpbuf(png_ptr)));
	png_init_io(png_ptr, png_file);
	png_set_sig_bytes(png_ptr, PNG_SIG_BYTES);
	png_read_info(png_ptr, info_ptr);

	*width = png_get_image_width(png_ptr, info_ptr);
	*height = png_get_image_height(png_ptr, info_ptr);

	png_uint_32 bit_depth, color_type;
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
			
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_gray_1_2_4_to_8(png_ptr);

	if (bit_depth == 16)
			png_set_strip_16(png_ptr);
			
	if(color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);
	else if(color_type == PNG_COLOR_TYPE_GRAY ||
			color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		{
			png_set_gray_to_rgb(png_ptr);
		}

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);
	else
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

	png_read_update_info(png_ptr, info_ptr);

	png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	png_uint_32 numbytes = rowbytes*(*height);
	png_byte* pixels = malloc(numbytes);
	png_byte** row_ptrs = malloc((*height) * sizeof(png_byte*));

	int i;
	for (i=0; i<(*height); i++)
	  row_ptrs[i] = pixels + ((*height) - 1 - i)*rowbytes;

	png_read_image(png_ptr, row_ptrs);

	free(row_ptrs);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	fclose(png_file);

	return (char *)pixels;
}
