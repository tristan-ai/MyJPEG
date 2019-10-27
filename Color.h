#pragma once

#include <vector>
#include <array>
#include "Image.h"
#include "Gauss.h"

namespace color
{
	void transform_rgb_to_ycbcr(const RGB& _rgb, YCbCr& ycbcr)
	{
		RGB rgb;
		rgb = _rgb;
		rgb.Customize();
		float y_val = 0.299f * static_cast<float>(rgb.r) + 0.587f * static_cast<float>(rgb.g) + 0.114f * static_cast<float>(rgb.b);
		float cb_val = 128 + (-0.168736f * static_cast<float>(rgb.r) - 0.331264f * static_cast<float>(rgb.g) + 0.5f * static_cast<float>(rgb.b));
		float cr_val = 128 + (0.5f * static_cast<float>(rgb.r) - 0.418688f * static_cast<float>(rgb.g) - 0.081312f * static_cast<float>(rgb.b));
		ycbcr.y = (y_val > 255) ? 255 : (y_val < 0) ? 0 : y_val;
		ycbcr.cb = (cb_val > 255) ? 255 : (cb_val < 0) ? 0 : cb_val;
		ycbcr.cr = (cr_val > 255) ? 255 : (cr_val < 0) ? 0 : cr_val;
	}

	void transform_ycbcr_to_rgb(const YCbCr& _ycbcr, RGB& rgb)
	{
		YCbCr ycbcr;
		ycbcr = _ycbcr;
		ycbcr.Customize();
		float r_val = static_cast<float>(ycbcr.y) + 1.402f * (static_cast<float>(ycbcr.cr) - 128);
		float g_val = static_cast<float>(ycbcr.y) - 0.344136f * (static_cast<float>(ycbcr.cb) - 128) - 0.714136f * (static_cast<float>(ycbcr.cr) - 128);
		float b_val = static_cast<float>(ycbcr.y) + 1.772f * (static_cast<float>(ycbcr.cb) - 128);
		rgb.r = (r_val > 255) ? 255 : (r_val < 0) ? 0 : r_val;
		rgb.g = (g_val > 255) ? 255 : (g_val < 0) ? 0 : g_val;
		rgb.b = (b_val > 255) ? 255 : (b_val < 0) ? 0 : b_val;
	}

	void chroma_subsampling(const IImage& image, SubsampledData& subsampled_data, size_t channel)
	{
		if (channel == 0)
		{
			for (size_t i = 0; i < image.GetSize(); i++)
				subsampled_data[i] = image.GetYCbCrData()[i].y;
			return;
		}

		if ((image.GetWidth() % 2) || (image.GetHeight() % 2)) throw std::logic_error("error occurred in [Color.h]chroma_subsampling(): can't subsample, please set block_size = 8");
		
		size_t width = image.GetWidth() / 2;
		size_t height = image.GetHeight() / 2;

		size_t width_leftover = ((width % block_size) ? block_size - (width % block_size) : 0);
		size_t height_leftover = ((height % block_size) ? block_size - (height % block_size) : 0);

		size_t final_width = width + width_leftover;
		size_t final_height = height + height_leftover;
		size_t size = final_width * final_height;

		subsampled_data.Resize(width, height, final_width, final_height);

		size_t index;

		uint16_t _4x4block_sum;

		size_t line, col;

		for (line = 0; line < image.GetHeight(); line += 2)
		{
			_4x4block_sum = 0;
			for (col = 0; col < image.GetWidth(); col += 2)
			{
				_4x4block_sum =
				{
					(uint16_t)(((uint16_t)(image.GetYCbCrData()[ line      * image.GetWidth() + col    ][channel]))
							 + ((uint16_t)(image.GetYCbCrData()[ line      * image.GetWidth() + col + 1][channel]))
							 + ((uint16_t)(image.GetYCbCrData()[(line + 1) * image.GetWidth() + col    ][channel]))
							 + ((uint16_t)(image.GetYCbCrData()[(line + 1) * image.GetWidth() + col + 1][channel])))
				};
				index = (line / 2) * final_width + (col / 2);
				subsampled_data[index] = ((uint8_t)(_4x4block_sum / 4));
			}
			for (col /= 2; col < final_width; col++)
			{
				index = (line / 2) * final_width + col;
				subsampled_data[index] = ((uint8_t)(_4x4block_sum / 4));
			}
		}
		for (line /= 2; line < final_height; line++)
		{
			for (col = 0; col < final_width; col++)
			{
				index = line * final_width + col;
				subsampled_data[index] = ((uint8_t)(subsampled_data[index - final_width]));
			}
		}
	}

	void scale_up_subsampled_block(const PIXEL_BLOCK_8& pixel_block, std::array<PIXEL_BLOCK_8, 4>& return_blocks)
	{
		size_t block_index, row, col; // for return_blocks
		uint8_t pixel_value;
		for (size_t pixel_index = 0; pixel_index < block_size_sqr; pixel_index++)
		{
			pixel_value = pixel_block[pixel_index / block_size][pixel_index % block_size];

			block_index = (pixel_index % block_size) / (block_size / 2) + (pixel_index / (block_size_sqr / 2)) * 2;
			row = (pixel_index / block_size) * 2 - (pixel_index / (block_size_sqr / 2)) * block_size;
			col = ((pixel_index % (block_size / 2)) % block_size) * 2;

			return_blocks[block_index][row]    [col]     = pixel_value;
			return_blocks[block_index][row]    [col + 1] = pixel_value;
			return_blocks[block_index][row + 1][col]     = pixel_value;
			return_blocks[block_index][row + 1][col + 1] = pixel_value;
		}
	}
}
