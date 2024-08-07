#include "pch.h"
#include "CImageGen.h"

#include <fstream> // for exporting
//#include <iostream> // for debugging
#include <random>
#include <cmath>


//
// Color Stuff
//
ColorImageGen::ColorImageGen() :  r(0), g(0), b(0)
{
}

ColorImageGen::ColorImageGen(float r, float g, float b) : r(r), g(g), b(b)
{
}

ColorImageGen::~ColorImageGen()
{
}



//
// Image stuff
//

/*

PARTS OF A BITMAP
1. File header - general info
2. Information header - detailed info
3. Pixel array - color data

each color is 3 bytes
with 3 color channels you get 9 bytes which is not divisible by 4
need to add padding

*/


CImageGen::CImageGen(int width, int height) : m_width(width), m_height(height), m_colors(std::vector<ColorImageGen>(width * height))
{
	srand(time(NULL));
}

CImageGen::~CImageGen()
{
}

ColorImageGen CImageGen::GetColor(int x, int y) const
{
	return m_colors[y * m_width + x]; // colors in the grid stored in one long vector
}

void CImageGen::SetColor(const ColorImageGen& color, int x, int y)
{
	m_colors[y * m_width + x].r = color.r;
	m_colors[y * m_width + x].g = color.g;
	m_colors[y * m_width + x].b = color.b;
}

void CImageGen::ColorfulDebugImage()
{
	for (int y = 0; y < m_height; y++) {
		for (int x = 0; x < m_width; x++) {

			float xPercentOfWidth = (float)x / (float)m_width;
			float yPercentOfHeight = (float)y / (float)m_height;

			SetColor(ColorImageGen(xPercentOfWidth, 1.0f - xPercentOfWidth, yPercentOfHeight), x, y);
		}
	}
}

float CImageGen::RandomFloat(float min, float max) 
{
	// use a uniform distribution for generating random values
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distribution(min, max);
	return distribution(gen);
}

std::vector<std::vector<float>> CImageGen::DiamondSquareHeightmap(int size, float scale) 
{
	// Initialize empty heightmap
	std::vector<std::vector<float>> heightmap(size, std::vector<float>(size, 0.0f));

	// Set random values for corners
	heightmap[0][0] = RandomFloat(-scale, scale);
	heightmap[0][size - 1] = RandomFloat(-scale, scale);
	heightmap[size - 1][0] = RandomFloat(-scale, scale);
	heightmap[size - 1][size - 1] = RandomFloat(-scale, scale);

	// diamond square process
	for (int step = size / 2; step >= 1; step /= 2) { // keep splitting

		// Diamond
		int increment = step * 2;
		for (int y = step; y < size - step; y += increment) {
			for (int x = step; x < size - step; x += increment) {

				float average = (heightmap[y][x - step] + heightmap[y][x + step] + heightmap[y - step][x] + heightmap[y + step][x]) / 4.0f;
				heightmap[y][x] = average + RandomFloat(-scale, scale);
			}
		}

		// Square 
		for (int y = 0; y < size; y += step) {
			for (int x = 0; x < size; x += step) {
				int offsetX = (x == 0) ? step : ((x == size - 1) ? -step : 0); // calculate offset relative to current square
				int offsetY = (y == 0) ? step : ((y == size - 1) ? -step : 0);

				float average = (heightmap[y + offsetY][x] + heightmap[y][x + offsetX]) / 2.0f;
				heightmap[y][x] = average + RandomFloat(-scale, scale);
			}
		}
	}

	return heightmap;
}


//
// C++: Perlin Noise Tutorial
// https://www.youtube.com/watch?v=kCIaHqb60Cw
//

std::vector<float> CImageGen::RandomGradient(int ix, int iy) {

	// Hashing function
	// random but deterministic

	const unsigned w = 8 * sizeof(unsigned);
	const unsigned s = w / 2;
	unsigned a = ix, b = iy;
	a *= 3284157443;

	b ^= a << s | a >> w - s;
	b *= 1911520717;

	a ^= b << s | b >> w - s;
	a *= 2048419325;
	float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

	// Create the vector from the angle
	return { sin(random) , cos(random) };
}

float CImageGen::DotGridGradiant(int ix, int iy, float x, float y) {
	// dot product between distance vector and gradiant vector

	// get random gradiant but try to have a rhyme or reason to it
	// i.e. given a particular seed, (4, 5) will always yield 9 

	std::vector<float> gradiant = RandomGradient(ix, iy);

	// distance vector
	float dx = x - (float)ix;
	float dy = y - (float)iy;

	// dot product
	return (dx * gradiant[0] + dy*gradiant[1]);
}

float CImageGen::InterpolateCubic(float value1, float value2, float weight) {
	return (value2 - value1) * (3.0 - weight * 2.0) * weight * weight + value1;
}

float CImageGen::Perlin(float x, float y) {
	// sample perlin at coords

	// grid cell corners
	int leftSide = (int)x; // acts as a floor function
	int bottomSide = (int)y;
	int rightSide = leftSide + 1;
	int topSide = bottomSide + 1;

	// distance to bottom left corner
	float distanceToLeft = x - (float)leftSide;
	float distanceTobottom = y - (float)bottomSide;

	// each corner
	float bottomLeftCorner = DotGridGradiant(leftSide, bottomSide, x, y);
	float bottomRightCorner = DotGridGradiant(rightSide, bottomSide, x, y);
	float topLeftCorner = DotGridGradiant(leftSide, topSide, x, y);
	float topRightCorner = DotGridGradiant(rightSide, topSide, x, y);

	// interpolate horizontals
	float interpBottom = InterpolateCubic(bottomLeftCorner, bottomRightCorner, distanceToLeft);
	float interpTop = InterpolateCubic(topLeftCorner, topRightCorner, distanceToLeft);

	// interpolate vertically between the two previous horizontal points
	return InterpolateCubic(interpBottom, interpTop, distanceTobottom);
}


std::vector<std::vector<float>> CImageGen::PerlinMap(int size) {
	const int GRID_SIZE = 30;

	std::vector<std::vector<float>> result;

	for (int x = 0; x < size; x++) {
		std::vector<float> column;
		for (int y = 0; y < size; y++) {

			float value = 0;

			float frequency = 1;
			float amplitude = 1; // determines weight of successive octaves

			// for loop for multiple octaves
			for (int i = 0; i < 12; i++) {
				value += Perlin(x * frequency / GRID_SIZE, y * frequency / GRID_SIZE) * amplitude;

				frequency *= 2;
				amplitude /= 2;
			}

			// enhance extremity / contrast
			value *= 1.2;
			if (value > 1) value = 1;
			if (value < 0) value = 0;

			column.push_back(value);
		}
		result.push_back(column);
	}

	return result;
}


void CImageGen::GenerateStoneTexture()
{
	// make sure the image is the correct size

	// Generate heightmap
	float scale = 20.0f;
	std::vector<std::vector<float>> heightmap = DiamondSquareHeightmap(m_width, scale);

	// Generate Perlin noise
	std::vector<std::vector<float>> perlinNoise = PerlinMap(m_width);
	
	// smooth by getting the average of four neighbors
	for(int i=0; i<4; i++) // more iterations makes it fuzzier
	{
		for (int y = 0; y < m_height; y++) {
			for (int x = 0; x < m_width; x++) {
				int below = y - 1;
				if (below < 0) below = m_height - 1; // wrap

				int left = x - 1;
				if (left < 0) left = m_width - 1; // wrap

				float avg = (heightmap[below][x] + heightmap[(y + 1) % m_height][x] + heightmap[y][left] + heightmap[y][(x + 1) % m_width]) / 4.0f;
				heightmap[y][x] = (heightmap[y][x] + avg) / 2.0f; // Blend original and average
			}
		}
	}


	// convert to 0-1 scale
	for (int y = 0; y < m_height; y++) {
		for (int x = 0; x < m_width; x++) {

			double contrast = 1.0;
			double currentVal = heightmap[x][y] * contrast;

			currentVal = (currentVal + scale) / (scale*2);//0;//0.9f; // assuming the max values are at around -20 to 20
			
			if (currentVal < 0) currentVal = 0;
			if (currentVal > 1) currentVal = 1;

			// add perlin noise
			//currentVal = ((perlinNoise[x][y]/2.4) + currentVal) / (2);
			currentVal += (perlinNoise[x][y]/1.2) / 2;
			//currentVal = perlinNoise[x][y];

			//heightmap[x][y] = currentVal;

			// tone down artifacts
			if (currentVal < 0) currentVal = 0;
			if (currentVal > 1) currentVal = 1;

			SetColor(ColorImageGen(currentVal, currentVal, currentVal), x, y);
		}
	}
}


void CImageGen::Export(const char* path) const
{
	std::ofstream f; // new filestream
	f.open(path, std::ios::out | std::ios::binary); // out means writing, binary specifies this is writing in binary

	if (!f.is_open()) {
		//std::cout << "File could not be opened" << std::endl;
		return;
	}

	unsigned char bmpPadding[3] = {0,0,0}; // padding per color
	const int paddingAmount = ((4 - (m_width * 3) % 4) % 4);

	const int fileHeaderSize = 14;
	const int informationHeaderSize = 40;
	const int fileSize = fileHeaderSize + informationHeaderSize + (m_width * m_height * 3) + paddingAmount * m_height;

	unsigned char fileHeader[fileHeaderSize];

	// file type
	fileHeader[0] = 'B';
	fileHeader[1] = 'M';

	// file size
	fileHeader[2] = fileSize;
	fileHeader[3] = fileSize >> 8;
	fileHeader[4] = fileSize >> 16;
	fileHeader[5] = fileSize >> 24;

	// Reserved 1 & 2
	fileHeader[6] = 0;
	fileHeader[7] = 0;
	fileHeader[8] = 0;
	fileHeader[9] = 0;

	// pixel data offset
	fileHeader[10] = fileHeaderSize + informationHeaderSize;
	fileHeader[11] = 0;
	fileHeader[12] = 0;
	fileHeader[13] = 0;

	unsigned char informationHeader[informationHeaderSize];

	// header size
	informationHeader[0] = informationHeaderSize;
	informationHeader[1] = 0;
	informationHeader[2] = 0;
	informationHeader[3] = 0;

	// image width
	informationHeader[4] = m_width;
	informationHeader[5] = m_width >> 8;
	informationHeader[6] = m_width >> 16;
	informationHeader[7] = m_width >> 24;

	// image height
	informationHeader[8] = m_height;
	informationHeader[9] = m_height >> 8;
	informationHeader[10] = m_height >> 16;
	informationHeader[11] = m_height >> 24;

	// planes
	informationHeader[12] = 1;
	informationHeader[13] = 0;

	// bits per pixel (RGB)
	informationHeader[14] = 24;
	informationHeader[15] = 0;

	// compression
	informationHeader[16] = 0;
	informationHeader[17] = 0;
	informationHeader[18] = 0;
	informationHeader[19] = 0;

	// image size for compression
	informationHeader[20] = 0;
	informationHeader[21] = 0;
	informationHeader[22] = 0;
	informationHeader[23] = 0;

	// x pixels per meter
	informationHeader[24] = 0;
	informationHeader[25] = 0;
	informationHeader[26] = 0;
	informationHeader[27] = 0;

	// y pixels per meter
	informationHeader[28] = 0;
	informationHeader[29] = 0;
	informationHeader[30] = 0;
	informationHeader[31] = 0;

	// total colors
	informationHeader[32] = 0;
	informationHeader[33] = 0;
	informationHeader[34] = 0;
	informationHeader[35] = 0;

	// important colors
	informationHeader[36] = 0;
	informationHeader[37] = 0;
	informationHeader[38] = 0;
	informationHeader[39] = 0;

	f.write(reinterpret_cast<char*>(fileHeader), fileHeaderSize);
	f.write(reinterpret_cast<char*>(informationHeader), informationHeaderSize);

	for (int y = 0; y < m_height; y++) {
		for (int x = 0; x < m_width; x++) {
			unsigned char r = static_cast<unsigned char>(GetColor(x, y).r * 255.0f);
			unsigned char g = static_cast<unsigned char>(GetColor(x, y).g * 255.0f);
			unsigned char b = static_cast<unsigned char>(GetColor(x, y).b * 255.0f);

			unsigned char color[] = { b, g, r };

			f.write(reinterpret_cast<char*>(color), 3);
		}

		f.write(reinterpret_cast<char*>(bmpPadding), paddingAmount);
	}

	f.close();

	//std::cout << "file created" << std::endl;
}






