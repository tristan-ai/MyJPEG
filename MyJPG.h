#pragma once

#include <vector>
#include <string>
#include <thread>
#include "Image.h"
#include "Gauss.h"

/*
	format:

		cos() * cos() + cos() * cos() + ... + cos() * cos()
		cos() * cos() + cos() * cos() + ... + cos() * cos()
		.
		.
		.
		cos() * cos() + cos() * cos() + ... + cos() * cos()
*/
std::array<std::array<float, block_size_sqr + 1>, block_size_sqr> cosblocks;

constexpr size_t num_threads = { 8 };

class MyJPG
{
	Elem<WORD> width{ 0 };
	Elem<WORD> height{ 2 };
	Elem<WORD> final_width{ 4 };
	Elem<WORD> final_height{ 6 };
	Elem<BYTE> quality{ 8 };
	Elem<WORD> cb_cr_width{ 9 };
	Elem<WORD> cb_cr_height{ 11 };
	Elem<WORD> cb_cr_final_width{ 13 };
	Elem<WORD> cb_cr_final_height{ 15 };

	IImage* img_src;

	std::array<std::vector<PIXEL_BLOCK_8>, 3> y_cb_cr_blocks;
	std::array<std::vector<std::array<float, block_size_sqr>>, 3> y_cb_cr_results;

	static const char* fext;

	FileManager export_file;
	FileManager import_file;

	std::array<std::array<std::thread, num_threads>, 3> y_cb_cr_calc_threads;

private:

	void CalcProc(size_t i, size_t end, size_t channel, size_t thread_index)
	{
		std::array<std::array<float, block_size_sqr + 1>, block_size_sqr> thread_cb = cosblocks;
		for (; i < end; i++)
		{
			for (size_t j = 0; j < block_size_sqr; j++)
				thread_cb[j][block_size_sqr] = ((float)(y_cb_cr_blocks[channel][i][j / block_size][j % block_size]) / 127.5f) - 1.f;
			gauss(thread_cb, y_cb_cr_results[channel][i]);
		}
	}

	void CalculateResultsFromSubsampledData()
	{
		if (img_src->GetWidth() % block_size || img_src->GetHeight() % block_size
			|| img_src->GetSubsampledData(1).GetFinalWidth() % block_size || img_src->GetSubsampledData(1).GetFinalHeight() % block_size
			|| img_src->GetSubsampledData(2).GetFinalWidth() % block_size || img_src->GetSubsampledData(2).GetFinalHeight() % block_size)
			throw std::runtime_error("error occurred in [MyJPG.h]CalculateResultsFromSubsampledData(): can't subdivide image, check its size and the block_size");

		size_t num_y_blocks_width = img_src->GetSubsampledData(0).GetFinalWidth() / block_size;
		size_t num_y_blocks_height = img_src->GetSubsampledData(0).GetFinalHeight() / block_size;
		size_t num_y_blocks = num_y_blocks_width * num_y_blocks_height;
		size_t num_cb_blocks_width = img_src->GetSubsampledData(1).GetFinalWidth() / block_size; // same
		size_t num_cb_blocks_height = img_src->GetSubsampledData(1).GetFinalHeight() / block_size; // same
		size_t num_cb_blocks = num_cb_blocks_width * num_cb_blocks_height; // same
		size_t num_cr_blocks_width = img_src->GetSubsampledData(2).GetFinalWidth() / block_size; // same
		size_t num_cr_blocks_height = img_src->GetSubsampledData(2).GetFinalHeight() / block_size; // same
		size_t num_cr_blocks = num_cr_blocks_width * num_cr_blocks_height; // same

		size_t channel;
		size_t index;
		size_t num_pixels_x;
		size_t num_blocks_x;
		size_t i, j;

		int res;

		for (channel = 0; channel < 3; channel++) y_cb_cr_blocks[channel].clear();
		y_cb_cr_blocks[0].assign(num_y_blocks, {});
		y_cb_cr_blocks[1].assign(num_cb_blocks, {});
		y_cb_cr_blocks[2].assign(num_cr_blocks, {});

		// subdivide image into blocks
		for (channel = 0; channel < 3; channel++)
		{
			num_pixels_x = img_src->GetSubsampledData(channel).GetFinalWidth();
			num_blocks_x = num_pixels_x / block_size;
			for (size_t y_cb_cr_block_index = 0; y_cb_cr_block_index < y_cb_cr_blocks[channel].size(); y_cb_cr_block_index++)
				for (size_t line = 0; line < block_size; line++)
					for (size_t col = 0; col < block_size; col++)
					{
						index = ((line + block_size * (y_cb_cr_block_index / num_blocks_x)) * num_pixels_x + col) + ((y_cb_cr_block_index % num_blocks_x) * block_size);
						y_cb_cr_blocks[channel][y_cb_cr_block_index][line][col] = img_src->GetSubsampledData(index, channel);
					}
		}

		for (channel = 0; channel < 3; channel++) y_cb_cr_results[channel].clear();
		y_cb_cr_results[0].assign(num_y_blocks, {});
		y_cb_cr_results[1].assign(num_cb_blocks, {});
		y_cb_cr_results[2].assign(num_cr_blocks, {});

		std::array<std::array<float, block_size_sqr + 1>, block_size_sqr> main_cb = cosblocks;

		size_t blocks_per_thread;

		// calculate results
		for (channel = 0; channel < 3; channel++)
		{
			blocks_per_thread = y_cb_cr_results[channel].size() / num_threads;
			for (i = 0; i < num_threads; i++)
				y_cb_cr_calc_threads[channel][i] = std::thread{ &MyJPG::CalcProc, this, i * blocks_per_thread, i * blocks_per_thread + blocks_per_thread, channel, i };

			for (i *= blocks_per_thread; i < y_cb_cr_results[channel].size(); i++)
			{
				for (j = 0; j < block_size_sqr; j++)
					main_cb[j][block_size_sqr] = ((float)(y_cb_cr_blocks[channel][i][j / block_size][j % block_size]) / 127.5f) - 1.f;
				res = gauss(main_cb, y_cb_cr_results[channel][i]);
				/*switch (res) /////////////////////////////////////////
				{
				case 0: throw std::runtime_error("error occurred in [MyJPG.h]CalculateResultsFromSubsampledData(): there are no solutions to the equation"); break; ////////////
				case INF: throw std::runtime_error("error occurred in [MyJPG.h]CalculateResultsFromSubsampledData(): there are infinite solutions to the equation"); break;
				}*/
			}
		}

		for (channel = 0; channel < 3; channel++)
			for (i = 0; i < num_threads; i++)
				if (y_cb_cr_calc_threads[channel][i].joinable())
					y_cb_cr_calc_threads[channel][i].join();
	}

	void CalculateBlocksFromResults()
	{
		float sum;
		for (size_t channel = 0; channel < 3; channel++)
			for (size_t block_index = 0; block_index < y_cb_cr_blocks[channel].size(); block_index++)
				for (size_t pixel_index = 0; pixel_index < block_size_sqr; pixel_index++)
				{
					sum = 0.f;
					for (size_t value_index = 0; value_index < quality.data; value_index++)
						sum += y_cb_cr_results[channel][block_index][value_index] * cosblocks[pixel_index][value_index];
					y_cb_cr_blocks[channel][block_index][pixel_index / block_size][pixel_index % block_size] = (uint8_t)((sum + 1.f) * 127.5f);
				}
	}

	void MakeFile(const std::string& fname)
	{
		export_file.SetPath(fname + fext);
		if (!export_file.Open(std::ios::out | std::ios::trunc | std::ios::binary)) throw std::runtime_error("error occurred in [MyJPG.h]MakeFile(): can't open file for writing");
		uintmax_t new_size = 4 * sizeof(WORD) + 1 * sizeof(BYTE);
		export_file.Resize(new_size);
		export_file.WriteElem(quality);
		export_file.WriteElem(width);
		export_file.WriteElem(height);
		export_file.WriteElem(final_width);
		export_file.WriteElem(final_height);
		export_file.WriteElem(cb_cr_width);
		export_file.WriteElem(cb_cr_height);
		export_file.WriteElem(cb_cr_final_width);
		export_file.WriteElem(cb_cr_final_height);
		export_file.SetWritePosToEnd();
	}

	void ReadFile(const std::string& fname)
	{
		import_file.SetPath(fname + fext);
		if (!import_file.Open(std::ios::in | std::ios::binary)) throw std::runtime_error("error occurred in [MyJPG.h]ReadFile(): can't open file for reading");
		quality.data = 8;
		cb_cr_final_width.data = 9;
		import_file.ReadElem(quality);
		import_file.ReadElem(final_width);
		import_file.ReadElem(final_height);
		import_file.ReadElem(width);
		import_file.ReadElem(height);
		import_file.ReadElem(cb_cr_width);
		import_file.ReadElem(cb_cr_height);
		import_file.ReadElem(cb_cr_final_width);
		import_file.ReadElem(cb_cr_final_height);
		import_file.file.seekg(17);
	}

	void SetSize()
	{
		width.data = static_cast<WORD>(img_src->GetSubsampledData(0).GetWidth()); //// datentypen size_t unnötig?!!
		height.data = static_cast<WORD>(img_src->GetSubsampledData(0).GetHeight());
		final_width.data = static_cast<WORD>(img_src->GetSubsampledData(0).GetFinalWidth());
		final_height.data = static_cast<WORD>(img_src->GetSubsampledData(0).GetFinalHeight());
		cb_cr_width.data = static_cast<WORD>(img_src->GetSubsampledData(1).GetWidth());
		cb_cr_height.data = static_cast<WORD>(img_src->GetSubsampledData(1).GetHeight());
		cb_cr_final_width.data = static_cast<WORD>(img_src->GetSubsampledData(1).GetFinalWidth());
		cb_cr_final_height.data = static_cast<WORD>(img_src->GetSubsampledData(1).GetFinalHeight());
	}

	void SetQuality(float _quality)
	{
		if (_quality < 0.f || _quality > 1.f) throw std::length_error("error occurred in [MyJPG.h]SetQuality(): quality should be a value between 0 and 1");
		if (block_size_sqr > 255) throw std::length_error("error occurred in [MyJPG.h]SetQuality(): block_size is too big, please change it to e.g. 8");
		BYTE value = _quality * block_size_sqr;
		quality.data = (BYTE)((float)(_quality * (float)block_size_sqr));
	}

public:

	MyJPG(IImage* _img_src)
	{
		if (!_img_src) throw std::runtime_error("error occurred in [MyJPG.h]MyJPG(): image source shouldn't be empty");
		img_src = _img_src;
		quality.data = 255;
		SetSize();
	}
	~MyJPG() {}

	MyJPG(const MyJPG&) = delete;
	MyJPG(MyJPG&&) = delete;

	MyJPG& operator=(const MyJPG&) = delete;
	MyJPG& operator=(MyJPG&&) = delete;

	void Compress(const char* fname, float _quality = 1.f)
	{
		SetQuality(_quality);
		CalculateResultsFromSubsampledData();
		MakeFile(fname);
		for (size_t channel = 0; channel < 3; channel++)
			for (size_t block_index = 0; block_index < y_cb_cr_results[channel].size(); block_index++)
				for (size_t i = 0; i < quality.data; i++)
					export_file.WriteByte((BYTE)((y_cb_cr_results[channel][block_index][i] + 1.f) * 127.5f));
		export_file.Close();
	}

	void Decompress(const char* comp_fname, IImage* image)
	{
		img_src = image;

		ReadFile(comp_fname);

		const size_t num_y_values = (final_width.data * final_height.data);
		const size_t num_y_blocks = num_y_values / block_size_sqr;
		const size_t num_cb_cr_values = (cb_cr_final_width.data * cb_cr_final_height.data);
		const size_t num_cb_cr_blocks = num_cb_cr_values / block_size_sqr;

		size_t channel;

		for (channel = 0; channel < 3; channel++) y_cb_cr_results[channel].clear();
		y_cb_cr_results[0].assign(num_y_blocks, {});
		y_cb_cr_results[1].assign(num_cb_cr_blocks, {});
		y_cb_cr_results[2].assign(num_cb_cr_blocks, {});

		for (channel = 0; channel < 3; channel++) y_cb_cr_blocks[channel].clear();
		y_cb_cr_blocks[0].assign(num_y_blocks, {});
		y_cb_cr_blocks[1].assign(num_cb_cr_blocks, {});
		y_cb_cr_blocks[2].assign(num_cb_cr_blocks, {});

		BYTE b;

		// read results
		for (channel = 0; channel < 3; channel++)
			for (size_t block_index = 0; block_index < y_cb_cr_results[channel].size(); block_index++)
				for (size_t i = 0; i < quality.data; i++)
				{
					import_file.ReadByte(b);
					y_cb_cr_results[channel][block_index][i] = ((float)b) / 127.5f - 1.f;
				}

		import_file.Close();

		CalculateBlocksFromResults();

		const size_t num_cb_cr_blocks_width = (2 * cb_cr_final_width.data) / block_size;
		const size_t num_cb_cr_blocks_height = (2 * cb_cr_final_height.data) / block_size;

		std::array<std::vector<std::vector<PIXEL_BLOCK_8>>, 2> scaled_cb_cr_blocks =
		{
			std::vector<std::vector<PIXEL_BLOCK_8>>(num_cb_cr_blocks_height, std::vector<PIXEL_BLOCK_8>(num_cb_cr_blocks_width)),
			std::vector<std::vector<PIXEL_BLOCK_8>>(num_cb_cr_blocks_height, std::vector<PIXEL_BLOCK_8>(num_cb_cr_blocks_width))
		};

		std::array<PIXEL_BLOCK_8, 4> return_blocks;

		const size_t num_blocks_width = cb_cr_final_width.data / block_size;

		for (channel = 1; channel < 3; channel++)
			for (size_t cb_cr_block_index = 0; cb_cr_block_index < num_cb_cr_blocks; cb_cr_block_index++)
			{
				color::scale_up_subsampled_block(y_cb_cr_blocks[channel][cb_cr_block_index], return_blocks);
				scaled_cb_cr_blocks[channel - 1][(cb_cr_block_index / num_blocks_width) * 2][(cb_cr_block_index % num_blocks_width) * 2] = return_blocks[0];
				scaled_cb_cr_blocks[channel - 1][(cb_cr_block_index / num_blocks_width) * 2][(cb_cr_block_index % num_blocks_width) * 2 + 1] = return_blocks[1];
				scaled_cb_cr_blocks[channel - 1][(cb_cr_block_index / num_blocks_width) * 2 + 1][(cb_cr_block_index % num_blocks_width) * 2] = return_blocks[2];
				scaled_cb_cr_blocks[channel - 1][(cb_cr_block_index / num_blocks_width) * 2 + 1][(cb_cr_block_index % num_blocks_width) * 2 + 1] = return_blocks[3];
			}

		img_src->SetSize(width.data, height.data);
		img_src->CreateData();

		size_t row, col;
		size_t index;

		size_t num_y_blocks_width = final_width.data / block_size;

		uint8_t value;

		for (row = 0; row < img_src->GetHeight(); row++)
			for (col = 0; col < img_src->GetWidth(); col++)
			{
				index = row * img_src->GetWidth() + col;
				value = y_cb_cr_blocks[0][(col / block_size) + ((row / block_size) * num_y_blocks_width)][row % block_size][col % block_size];
				img_src->SetYCbCrData(index, 0, value);
			}

		for (channel = 1; channel < 3; channel++)
			for (row = 0; row < img_src->GetHeight(); row++)
				for (col = 0; col < img_src->GetWidth(); col++)
				{
					index = row * img_src->GetWidth() + col;
					value = scaled_cb_cr_blocks[channel - 1][row / block_size][col / block_size][row % block_size][col % block_size];
					img_src->SetYCbCrData(index, channel, value);
				}

		RGB rgb;
		for (size_t i = 0; i < img_src->GetSize(); i++)
		{
			color::transform_ycbcr_to_rgb(img_src->GetYCbCrData()[i], rgb);
			img_src->SetRGBData(i, rgb);
		}
	}
};

const char* MyJPG::fext = ".myj";
