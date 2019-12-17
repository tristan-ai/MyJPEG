// JPEG Compressor 2019

// for using mathematical constants like pi
#define _USE_MATH_DEFINES

#include <iostream>
#include <math.h>
#include "TGA.h"
#include "MyJPG.h"

#include "Debug.h"

constexpr float quality = { .4f };

void init_cosblocks()
{
	if (block_size >= 65536ULL /*std::numeric_limits<size_t>::max() ^ 0.25*/) throw std::length_error("error occurred in init_blocks(): block_size is too big");
	if (block_size < 2ULL) throw std::length_error("error occurred in init_blocks(): block_size must have a length of at least 2");

	size_t i; // counter variable

	float arguments[block_size]; // samples x and y
	uint64_t coefficients[block_size_sqr][2]; // coefficients a and b

	// calculate discrete arguments x and y for cos(ax) * cos(by)
	for (i = 0; i < block_size; i++) arguments[i] = M_PI * ((float)i / block_size) + (M_PI / (block_size * 2));

	// calculate coefficients a and b for cos(ax) * cos(by) sorted by frequency
	size_t pos[2] = { 0, 0 };
	coefficients[0][0] = pos[0];
	coefficients[0][1] = pos[1];
	i = 1;
	while (!(pos[0] == (block_size - 1) && pos[1] == (block_size - 1)))
	{
		if (pos[0] != (block_size - 1)) ++pos[0];
		else ++pos[1];
		coefficients[i][0] = pos[0];
		coefficients[i++][1] = pos[1];
		while (pos[0] != 0 && pos[1] != (block_size - 1))
		{
			coefficients[i][0] = --pos[0];
			coefficients[i++][1] = ++pos[1];
		}
		if (pos[1] != (block_size - 1)) ++pos[1];
		else ++pos[0];
		coefficients[i][0] = pos[0];
		coefficients[i++][1] = pos[1];
		while (pos[0] != (block_size - 1) && pos[1] != 0)
		{
			coefficients[i][0] = ++pos[0];
			coefficients[i++][1] = --pos[1];
		}
	}

	// calculate block_size^2 block_size x block_size blocks
	for (i = 0; i < block_size_sqr * block_size_sqr; i++) cosblocks[i / block_size_sqr][i % block_size_sqr] = cos(coefficients[i % block_size_sqr][0] * arguments[i / (block_size * block_size_sqr)]) * cos(coefficients[i % block_size_sqr][1] * arguments[(i / block_size_sqr) % block_size]);
}

int main(int argc, char** argv)
{
	init_cosblocks();

	TGAImage tga_img{ "tga/bild003.tga" };

	MyJPG myjpg(&tga_img);
	myjpg.Compress("tga/bild003", quality);

	TGAImage decomp_img{};

	myjpg.Decompress("tga/bild003", &decomp_img);

	decomp_img.Make("tga/bild003_d.tga");


	std::cout << "\nready\n";
	std::cin.get();
	return 0;
}
