#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int test_main_start(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    return test_main_start(argc, argv);
}


/* BMP : Header (14bytes) -> InfoHeader (40bytes) -> ColorTable (4*NumColors) -> PixelData (Width*Height)
	 		Header
				int16 signature      = *(int16*)&header[0];  // "BM" = 0x42 0x4D
				int32 filesize       = *(int32*)&header[2];  // 54 + 4*num_colors + imageSize
				int32 reserved       = *(int32*)&header[6];  // unused (=0)
				int32 dataoffset     = *(int32*)&header[10]; // 54 + 4*NumColors

			InfoHeader
				int32 size           = *(int32*)&header[14]; // Size of InfoHeader = 40 
				int32 width          = *(int32*)&header[18]; // Horizontal width of bitmap in pixels
				int32 height         = *(int32*)&header[22]; // Vertical height of bitmap in pixels
				int16 planes         = *(int16*)&header[26]; // Number of Planes (=1)
				int16 bits_per_pixel = *(int16*)&header[28]; // 1, 4, 8, 16, 24 : Refer below for more details 
				int32 compression    = *(int32*)&header[30]; // Type of Compression : 0 = BI_RGB (no compression), 1 = BI_RLE8 (8bit RLE encoding), 2 = BI_RLE4 (4bit RLE encoding)
				int32 imageSize      = *(int32*)&header[34]; // height * width 
				int32 XpixelsPerM    = *(int32*)&header[38]; // Horizontal resolution: Pixels/meter
				int32 YpixelsPerM    = *(int32*)&header[42]; // Vertical resolution: Pixels/meter
				int32 num_colors     = *(int32*)&header[46]; // num_colors used in the Pixel data
				int32 imp_colors     = *(int32*)&header[50]; // Number of important colors (0=all)

			int32 ColorTable [4 * num_colors]; // 1byte Red intensity, 1byte Green intensity, 1byte Blue intensity, 1byte reserved (unused=0) // 00 01 01 01
			int8  PixelData  [height * width];

		BitsPerPixel Field
			1 : (NumColors = 1)
				The bitmap is monochrome, and the palette contains two entries. Each bit in the bitmap array represents a pixel. If the bit is clear, the pixel is displayed with the color of the first entry in the palette; if the bit is set, the pixel has the color of the second entry in the table.
			4 : (NumColors = 16)
				The bitmap has a maximum of 16 colors, and the palette contains up to 16 entries. Each pixel in the bitmap is represented by a 4-bit index into the palette. For example, if the first byte in the bitmap is 1Fh, the byte represents two pixels. The first pixel contains the color in the second palette entry, and the second pixel contains the color in the sixteenth palette entry.
			8 : (NumColors = 256)
				The bitmap has a maximum of 256 colors, and the palette contains up to 256 entries. In this case, each byte in the array represents a single pixel.
			16 : (NumColors = 65536)
				The bitmap has a maximum of 2^16 colors. If the Compression field of the bitmap file is set to BI_RGB, the Palette field does not contain any entries. Each word in the bitmap array represents a single pixel. The relative intensities of red, green, and blue are represented with 5 bits for each color component. The value for blue is in the least significant 5 bits, followed by 5 bits each for green and red, respectively. The most significant bit is not used.
				If the Compression field of the bitmap file is set to BI_BITFIELDS, the Palette field contains three 4 byte color masks that specify the red, green, and blue components, respectively, of each pixel.  Each 2 bytes in the bitmap array represents a single pixel.
			24 : (NumColors = 16M)
				The bitmap has a maximum of 2^24 colors, and the Palette field does not contain any entries. Each 3-byte triplet in the bitmap array represents the relative intensities of blue, green, and red, respectively, for a pixel.
*/

int get_pixel_data_from_image(uint8_t **header, uint8_t **colorTable, uint8_t **srcBuf)
{
	int i = 0; 
	FILE *streamIn = fopen("images/lena512.bmp", "r");    // Input file name
	if(streamIn == (FILE *)0) 
	{
       printf("get_pixel_data_from_image : File opening error!\n");
       return -1; 
 	}

 	for(i=0;i<54;i++) 
 	{
 		*header[i] = getc(streamIn);  // strip out BMP header	
 	}

 	uint32_t width = *(uint32_t*)header[18]; // read the width from the image header
 	uint32_t height = *(uint32_t*)header[22]; // read the height from the image header
	uint32_t bitDepth = *(uint32_t*)header[28]; // read the bitDepth from the image header

	if(bitDepth == 8)
		fread(*colorTable, sizeof(unsigned char), 1024, streamIn);
	else
		return -1;

	printf("width: %d\n",width);
	printf("height: %d\n",height);

	*srcBuf = malloc(height * width);
	fread(*srcBuf, sizeof(unsigned char), (height * width), streamIn);
	fclose(streamIn);

	return 0;
}

int store_pixel_data_to_image(uint8_t *header, uint8_t *colorTable, uint8_t *dstBuf)
{
	int i = 0; 
	FILE *streamOut = fopen("images/lena_copy.bmp","wb"); // Output File name
	if(streamOut == (FILE *)0) 
	{
       printf("store_pixel_data_to_image : File opening error!\n");
       return -1; 
 	}

	fwrite(header, sizeof(unsigned char), 54, streamOut); // write the image header to output file
	fwrite(colorTable, sizeof(unsigned char), 1024, streamOut); // write color table 
	fwrite(buf, sizeof(unsigned char), (height * width), streamOut); // write pixel data

	fclose(streamOut);

	return 0;
}

int test_main_start(int argc, char* argv[]) 
{
   int retVal, i;

   uint8_t header[54]; // to store the image header
   uint8_t colorTable[1024]; // to store the colorTable, if it exists.
   uint8_t *srcBuf;

   get_pixel_data_from_image(&header, &colorTable, &srcBuf);


   uint32_t srcWidth = *(uint32_t*)&header[18]; // read the width from the image header
   uint32_t srcHeight = *(uint32_t*)&header[22]; // read the height from the image header

   store_pixel_data_to_image(header, colorTable, srcBuf);

   return retVal;
}
