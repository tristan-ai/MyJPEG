// Header for TGA file operations
#pragma once

#include "Image.h"
#include "Color.h"

#ifndef NO_RESIZE
#define NO_RESIZE false
#endif
#ifndef RESIZE
#define RESIZE true
#endif

class TGAImage final : public IImage
{
	Elem<BYTE> _elem_idlen				{  0 };
	Elem<BYTE> _elem_clrmaptype			{  1 };
	Elem<BYTE> _elem_imgtype			{  2 };
	Elem<BYTE> _elem_clrmapentrysize	{  7 };
	Elem<BYTE> _elem_bpp				{ 16 };
	Elem<BYTE> _elem_imgdesc			{ 17 };
	Elem<WORD> _elem_fstentryidx		{  3 };
	Elem<WORD> _elem_clrmaplen			{  5 };
	Elem<WORD> _elem_xorigin			{  8 };
	Elem<WORD> _elem_yorigin			{ 10 };
	Elem<WORD> _elem_width				{ 12 };
	Elem<WORD> _elem_height				{ 14 };

	bool SetAndCheckElems()
	{
		file_manager.ReadElem(_elem_idlen);
		file_manager.ReadElem(_elem_clrmaptype);
		file_manager.ReadElem(_elem_imgtype);
		file_manager.ReadElem(_elem_clrmapentrysize);
		file_manager.ReadElem(_elem_bpp);
		file_manager.ReadElem(_elem_imgdesc);
		file_manager.ReadElem(_elem_fstentryidx);
		file_manager.ReadElem(_elem_clrmaplen);
		file_manager.ReadElem(_elem_xorigin);
		file_manager.ReadElem(_elem_yorigin);
		file_manager.ReadElem(_elem_width);
		file_manager.ReadElem(_elem_height);

		if (_elem_idlen.data != 0x00 ||
			_elem_clrmaptype.data != 0x00 ||
			_elem_imgtype.data != 0x02 ||
			_elem_fstentryidx.data != 0x0000 ||
			_elem_clrmaplen.data != 0x0000 ||
			_elem_clrmapentrysize.data != 0x00 ||
			_elem_xorigin.data != 0x0000 ||
			_elem_yorigin.data != 0x0000 ||
			_elem_imgdesc.data != 0x00 ||
			_elem_bpp.data != 0x18 ||
			_elem_width.data <= 0x0000 ||
			_elem_height.data <= 0x0000)
		return false;

		return true;
	}

	void SetAndWriteElems(bool resize)
	{
		if (resize) file_manager.Resize((uintmax_t)(6 * sizeof(WORD) + 6 * sizeof(BYTE)));

		_elem_idlen.data = 0x00;
		_elem_clrmaptype.data = 0x00;
		_elem_imgtype.data = 0x02;
		_elem_fstentryidx.data = 0x0000;
		_elem_clrmaplen.data = 0x0000;
		_elem_clrmapentrysize.data = 0x00;
		_elem_xorigin.data = 0x0000;
		_elem_yorigin.data = 0x0000;
		_elem_imgdesc.data = 0x00;
		_elem_bpp.data = 0x18;
		_elem_width.data = GetWidth();
		_elem_height.data = GetHeight();

		file_manager.WriteElem(_elem_idlen);
		file_manager.WriteElem(_elem_clrmaptype);
		file_manager.WriteElem(_elem_imgtype);
		file_manager.WriteElem(_elem_clrmapentrysize);
		file_manager.WriteElem(_elem_bpp);
		file_manager.WriteElem(_elem_imgdesc);
		file_manager.WriteElem(_elem_fstentryidx);
		file_manager.WriteElem(_elem_clrmaplen);
		file_manager.WriteElem(_elem_xorigin);
		file_manager.WriteElem(_elem_yorigin);
		file_manager.WriteElem(_elem_width);
		file_manager.WriteElem(_elem_height);

		file_manager.SetWritePosToEnd();
	}

	void Load(const char* fname) override
	{
		file_manager.SetPath(fname);
		if (!file_manager.Open(std::ios::in | std::ios::binary)) throw std::runtime_error("error occurred in [TGA.h]Load(): can't open TGA-image for reading");
		if (!SetAndCheckElems()) throw std::runtime_error("error occurred in [TGA.h]Load(): incorrect file format");
		size_t width_leftover = ((_elem_width.data % block_size) ? block_size - (_elem_width.data % block_size) : 0);
		size_t height_leftover = ((_elem_height.data % block_size) ? block_size - (_elem_height.data % block_size) : 0);
		SetSize(_elem_width.data + width_leftover, _elem_height.data + height_leftover);
		y_data.Resize(_elem_width.data, _elem_height.data, width, height);
		CreateData();
		file_manager.file.seekg(18);
		RGB rgb_pixel;
		YCbCr ycbcr_pixel;
		size_t index;
		long long i;
		size_t j;
		for (i = _elem_height.data - 1; i >= 0; i--)
		{
			for (j = 0; j < _elem_width.data; j++)
			{
				file_manager.file.read(reinterpret_cast<char*>(&rgb_pixel.b), sizeof(uint8_t));
				file_manager.file.read(reinterpret_cast<char*>(&rgb_pixel.g), sizeof(uint8_t));
				file_manager.file.read(reinterpret_cast<char*>(&rgb_pixel.r), sizeof(uint8_t));
				color::transform_rgb_to_ycbcr(rgb_pixel, ycbcr_pixel);
				index = i * width + j;
				SetRGBData(index, rgb_pixel);
				SetYCbCrData(index, ycbcr_pixel);
			}
			for (; j < width; j++)
			{
				index = i * width + j;
				SetRGBData(index, rgb_pixel);
				SetYCbCrData(index, ycbcr_pixel);
			}
		}
		for (i = _elem_height.data; i < height; i++)
		{
			for (j = 0; j < width; j++)
			{
				index = i * width + j;
				SetRGBData(index, GetRGBData()[index - width]);
				SetYCbCrData(index, GetYCbCrData()[index - width]);
			}
		}
		file_manager.Close();
	}

public:
	TGAImage(const char* filename) :
		IImage()
	{
		Load(filename);
		color::chroma_subsampling(*this, y_data, 0);
		color::chroma_subsampling(*this, subsampled_cb_data, 1);
		color::chroma_subsampling(*this, subsampled_cr_data, 2);
	}
	TGAImage() :
		IImage()
	{
		//
	}

	void Make(const char* filename)
	{
		file_manager.SetPath(filename);
		if (!file_manager.Open(std::ios::out | std::ios::trunc | std::ios::binary)) throw std::runtime_error("error occurred in [TGA.h]Make(): can't open TGA-image for writing");
		SetAndWriteElems(RESIZE);
		
		for (long long row = GetHeight() - 1; row >= 0; row--)
		{
			for (size_t col = 0; col < GetWidth(); col++)
			{
				file_manager.WriteByte(GetRGBData()[row * GetWidth() + col].b);
				file_manager.WriteByte(GetRGBData()[row * GetWidth() + col].g);
				file_manager.WriteByte(GetRGBData()[row * GetWidth() + col].r);
			}
		}

		file_manager.Close();
	}
};	
