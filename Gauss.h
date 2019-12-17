#pragma once

#include <array>

constexpr size_t block_size = { 6 };
constexpr size_t block_size_sqr = { block_size * block_size };

typedef std::array<std::array<uint8_t, block_size>, block_size> PIXEL_BLOCK_8;

constexpr float EPS = { 0.0000001f }; ////
constexpr int INF = { 2 };

/*
Original is from: https://cp-algorithms.com/linear_algebra/linear-system-gauss.html
*/

int gauss(std::array<std::array<float, block_size_sqr + 1>, block_size_sqr> matrix, std::array<float, block_size_sqr>& result)
{
	std::vector<int> where(block_size_sqr, -1);
	for (int col = 0, row = 0; (col < block_size_sqr) && (row < block_size_sqr); ++col) {
		int sel = row;
		for (int i = row; i < block_size_sqr; ++i)
			if (abs(matrix[i][col]) > abs(matrix[sel][col]))
				sel = i;
		if (abs(matrix[sel][col]) < EPS)
			continue;
		for (int i = col; i <= block_size_sqr; ++i)
			std::swap(matrix[sel][i], matrix[row][i]);
		where[col] = row;

		for (int i = 0; i < block_size_sqr; ++i)
			if (i != row) {
				float c = matrix[i][col] / matrix[row][col];
				for (int j = col; j <= block_size_sqr; ++j)
					matrix[i][j] -= matrix[row][j] * c;
			}
		++row;
	}

	for (int i = 0; i < block_size_sqr; ++i)
		if (where[i] != -1)
			result[i] = matrix[where[i]][block_size_sqr] / matrix[where[i]][i];
	for (int i = 0; i < block_size_sqr; ++i) {
		float sum = 0;
		for (int j = 0; j < block_size_sqr; ++j)
			sum += result[j] * matrix[i][j];
		if (abs(sum - matrix[i][block_size_sqr]) > EPS)
			return 0;
	}

	for (int i = 0; i < block_size_sqr; ++i)
		if (where[i] == -1)
			return INF;
	return 1;
}
