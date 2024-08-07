#pragma once
#include <vector>
#include <cmath>

// This tutorial helped a lot with bmp generation:
// Creating a Bitmap Image (.bmp) using C++ | Tutorial
// https://www.youtube.com/watch?v=vqT5j38bWGg

struct ColorImageGen {
	float r, g, b;

	ColorImageGen();
	ColorImageGen(float r, float g, float b);
	~ColorImageGen();
};

class CImageGen
{
public:
	CImageGen(int width, int height);
	~CImageGen();

	ColorImageGen GetColor(int x, int y) const;
	void SetColor(const ColorImageGen& color, int x, int y);
	void Export(const char* path) const;

	void ColorfulDebugImage();
	void GenerateStoneTexture();

	float RandomFloat(float min, float max);
	std::vector<std::vector<float>> DiamondSquareHeightmap(int size, float scale);

	std::vector<float> RandomGradient(int ix, int iy);
	float DotGridGradiant(int ix, int iy, float x, float y);
	float InterpolateCubic(float value1, float value2, float weight);
	float Perlin(float x, float y);
	std::vector<std::vector<float>> PerlinMap(int size);


private:
	int m_width;
	int m_height;
	std::vector<ColorImageGen> m_colors;
};

