#pragma once

#include "FileManager.h"
#include "DTypes.h"

class IImage
{
protected:
	RGB* rgb_data;
	YCbCr* ycbcr_data;
	WORD width, height; /// 
	FileManager file_manager;

	SubsampledData y_data;
	SubsampledData subsampled_cb_data;
	SubsampledData subsampled_cr_data;

public:
	IImage() : rgb_data(nullptr), ycbcr_data(nullptr), width(0), height(0) {}
	virtual ~IImage()
	{
		if (rgb_data) delete[] rgb_data;
		if (ycbcr_data) delete[] ycbcr_data;
	}

	IImage(const IImage&) = delete;
	IImage(IImage&&) = delete;
	IImage& operator=(const IImage&) = delete;
	IImage& operator=(IImage&&) = delete;

	virtual void Load(const char* filename) = 0;

	const RGB* GetRGBData() const { return rgb_data; }
	const YCbCr* GetYCbCrData() const { return ycbcr_data; }
	const SubsampledData& GetSubsampledData(size_t channel) const
	{
		switch (channel)
		{
		case 0: return y_data;
		case 1: return subsampled_cb_data;
		case 2: return subsampled_cr_data;
		default: throw std::logic_error("error occurred in [Image.h]GetSubsampledData(): the given channel isn't subsampled");
		}
	}
	const uint8_t& GetSubsampledData(size_t index, size_t channel) const
	{
		switch (channel)
		{
		case 0: return y_data[index];
		case 1: return subsampled_cb_data[index];
		case 2: return subsampled_cr_data[index];
		default: throw std::logic_error("error occurred in [Image.h]GetSubsampledData(): the given channel isn't subsampled");
		}
	}
	
	WORD GetWidth() const { return width; }
	WORD GetHeight() const { return height; }
	size_t GetSize() const { return width * height; }

	void CreateData()
	{
		if (rgb_data) delete[] rgb_data;
		if (ycbcr_data) delete[] ycbcr_data;
		rgb_data = new RGB[GetSize()];
		ycbcr_data = new YCbCr[GetSize()];
	}

	void SetSize(WORD _width, WORD _height) { width = _width; height = _height; }
	void SetRGBData(size_t index, size_t channel, uint8_t value)
	{
		if (index >= GetSize())
			throw std::out_of_range("error occurred in [Image.h]IImage::SetRGBData(): index is out of range");
		if (channel >= 3)
			throw std::out_of_range("error occurred in [Image.h]IImage::SetRGBData(): channel should be between 0 and 2");
		rgb_data[index][channel] = value;
	}
	void SetRGBData(size_t index, const RGB& values)
	{
		if (index >= GetSize())
			throw std::out_of_range("error occurred in [Image.h]IImage::SetRGBData(): index is out of range");
		rgb_data[index] = values;
	}
	void SetYCbCrData(size_t index, size_t channel, uint8_t value)
	{
		if (index >= GetSize())
			throw std::out_of_range("error occurred in [Image.h]IImage::SetYCbCrData(): index is out of range");
		if (channel >= 3)
			throw std::out_of_range("error occurred in [Image.h]IImage::SetYCbCrData(): channel should be between 0 and 2");
		ycbcr_data[index][channel] = value;
	}
	void SetYCbCrData(size_t index, const YCbCr& values)
	{
		if (index >= GetSize())
			throw std::out_of_range("error occurred in [Image.h]IImage::SetYCbCrData(): index is out of range");
		ycbcr_data[index] = values;
	}
};
