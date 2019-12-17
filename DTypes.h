#pragma once

#include <stdint.h>
#include <stdexcept>
#include <vector>

typedef unsigned char BYTE;
typedef unsigned short WORD;

struct RGB
{
	uint8_t r, g, b;
	uint8_t& operator[](size_t index)
	{
		switch (index)
		{
		case 0: return r;
		case 1: return g;
		case 2: return b;
		default: throw std::out_of_range("error occurred in [DTypes.h]RGB::operator[](): index should be between 0 and 2");
		}
	}
	const uint8_t& operator[](size_t index) const
	{
		switch (index)
		{
		case 0: return r;
		case 1: return g;
		case 2: return b;
		default: throw std::out_of_range("error occurred in [DTypes.h]RGB::operator[](): index should be between 0 and 2");
		}
	}
	RGB(uint8_t _r = 0, uint8_t _g = 0, uint8_t _b = 0) : r(_r), g(_g), b(_b) {}
	~RGB() {}
	RGB(const RGB&) = delete;
	RGB(RGB&&) = delete;
	RGB& operator=(const RGB& rgb)
	{
		r = rgb.r;
		g = rgb.g;
		b = rgb.b;
		return *this;
	}
	RGB& operator=(RGB&& rgb) noexcept
	{
		r = std::move(rgb.r);
		g = std::move(rgb.g);
		b = std::move(rgb.b);
		return *this;
	};
	void Customize()
	{
		constexpr static uint8_t min = 25;
		constexpr static uint8_t max = 230;
		for (size_t i = 0; i < 3; i++)
		{
			if (this->operator[](i) < min) this->operator[](i) = min;
			else if (this->operator[](i) > max) this->operator[](i) = max;
		}
	}
};

struct YCbCr
{
	uint8_t y = 0, cb = 0, cr = 0;
	uint8_t& operator[](size_t index)
	{
		switch (index)
		{
		case 0: return y;
		case 1: return cb;
		case 2: return cr;
		default: throw std::out_of_range("error occurred in [DTypes.h]YCbCr::operator[](): index should be between 0 and 2");
		}
	}
	const uint8_t& operator[](size_t index) const
	{
		switch (index)
		{
		case 0: return y;
		case 1: return cb;
		case 2: return cr;
		default: throw std::out_of_range("error occurred in [DTypes.h]YCbCr::operator[](): index should be between 0 and 2");
		}
	}
	YCbCr(uint8_t _y = 0, uint8_t _cb = 0, uint8_t _cr = 0) : y(_y), cb(_cb), cr(_cr) {}
	~YCbCr() {}
	YCbCr(const YCbCr&) = delete;
	YCbCr(YCbCr&&) = delete;
	YCbCr& operator=(const YCbCr& ycbcr)
	{
		y = ycbcr.y;
		cb = ycbcr.cb;
		cr = ycbcr.cr;
		return *this;
	}
	YCbCr& operator=(YCbCr&& ycbcr) noexcept
	{
		y = std::move(ycbcr.y);
		cb = std::move(ycbcr.cb);
		cr = std::move(ycbcr.cr);
		return *this;
	};
	void Customize()
	{
		constexpr static uint8_t min = 25;
		constexpr static uint8_t max = 230;
		for (size_t i = 0; i < 3; i++)
		{
			if (this->operator[](i) < min) this->operator[](i) = min;
			else if (this->operator[](i) > max) this->operator[](i) = max;
		}
	}
};

class SubsampledData
{
	std::vector<uint8_t> data;

	size_t width, height;
	size_t final_width, final_height;

public:
	SubsampledData() : width(0), height(0), final_width(0), final_height(0) {}
	~SubsampledData() {}

	SubsampledData(const SubsampledData&) = delete;
	SubsampledData(SubsampledData&&) = delete;

	SubsampledData& operator=(const SubsampledData&) = delete;
	SubsampledData& operator=(SubsampledData&&) = delete;

	uint8_t& operator[](size_t index)
	{
		if (index >= data.size()) throw std::out_of_range("error occurred in [DTypes.h]SubsampledData::operator[](): index is out of range");
		return data[index];
	}

	const uint8_t& operator[](size_t index) const
	{
		if (index >= data.size()) throw std::out_of_range("error occurred in [DTypes.h]SubsampledData::operator[](): index is out of range");
		return data[index];
	}

	void Resize(size_t _width, size_t _height, size_t _final_width, size_t _final_height)
	{
		width = _width;
		height = _height;
		final_width = _final_width;
		final_height = _final_height;

		data.clear();
		data.assign(final_width * final_height, 0);
	}

	size_t GetWidth() const { return width; }
	size_t GetHeight() const { return height; }

	size_t GetFinalWidth() const { return final_width; }
	size_t GetFinalHeight() const { return final_height; }

	size_t GetWidthLeftover() const { return final_width - width; }
	size_t GetHeightLeftover() const { return final_height - height; }

	size_t GetSize() const { return width * height; }
	size_t GetFinalSize() const { return final_width * final_height; }
};

template <typename _Data>
struct Elem
{
	std::streampos pos;
	_Data data;
	Elem(const std::streampos& _pos) : pos(_pos), data(_Data{}) {}
	~Elem() {}
	Elem(const Elem&) = delete;
	Elem(Elem&&) = delete;
	Elem& operator=(const Elem&) = delete;
	Elem& operator=(Elem&&) = delete;
};
