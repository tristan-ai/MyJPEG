#pragma once

#include <fstream>
#include "TGA.h"

namespace debug
{
	void _D_TGAtoCSV(const TGAImage& tga_img, const char* filename, size_t channel = 0)
	{
		std::ofstream csv_file;
		csv_file.open(filename, std::ios::out | std::ios::trunc);
		csv_file << (int)tga_img.GetYCbCrData()[0][channel];
		for (size_t i = 1; i < tga_img.GetSize(); i++) csv_file << ',' << (int)tga_img.GetYCbCrData()[i][channel];
		csv_file << std::endl;
		csv_file.close();
	}
}
