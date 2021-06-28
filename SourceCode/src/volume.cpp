#include "volume.h"
#include "extra/pvmparser.h"
#include "extra/PerlinNoise.hpp"

Volume::Volume() {
	width = height = depth = 0;
	widthSpacing = heightSpacing = depthSpacing = 1.0; 
	data = NULL;
	channels = 1; 
	bytes_per_channel = 1;
}

Volume::Volume(int w, int h, int d, int channels, int bytes_per_channel) {
	widthSpacing = heightSpacing = depthSpacing = 1.0;
	data = NULL;
	resize(w, h, d, channels, bytes_per_channel);
}

Volume::~Volume() {
	if (data) delete[]data;
	data = NULL;
}

void Volume::resize(int w, int h, int d, int channels, int bytes_per_channel) {
	if (data) delete[] data;
	width = w;
	height = h;
	depth = d;
	this->channels = channels;
	this->bytes_per_channel = bytes_per_channel;
	data = new Uint8[w*h*d*channels*bytes_per_channel];
	memset(data, 0, w*h*d*channels*bytes_per_channel);
}

void Volume::clear() {
	if (data) delete[]data;
	data = NULL;
	width = height = depth = 0;
}

void Volume::fillSphere() {
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			for (int k = 0; k < depth; k++) {
				float f = 0;
				float x = 2.0*(((float)i / width) - 0.5);
				float y = 2.0*(((float)j / height) - 0.5);
				float z = 2.0*(((float)k / depth) - 0.5);

				f = (1.0 - (x*x + y * y + z * z) / 3.0);
				f = f < 0.5 ? 0.0 : f;

				data[i + width*j + width*height*k] = (Uint8)(f* 255.0);
			}
		}
	}
}

void Volume::fillNoise(float frequency, int octaves, unsigned int seed) {
	float f = frequency > 0.1 ? frequency < 64.0 ? frequency : 64.0 : 0.1;
	int o = octaves > 1 ? octaves < 16 ? octaves : 16 : 1;

	const siv::PerlinNoise perlin(seed);
	const float fx = (float)width / f;
	const float fy = (float)height / f;
	const float fz = (float)depth / f;


	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			for (int k = 0; k < depth; k++) {
				float v = perlin.octaveNoise0_1(i / fx, j / fy, k / fz, o);
				data[VOLPOS(i, j, k, width, height, depth, 1)] = (Uint8)(255 * v);
			}
		}
	}
}

bool Volume::loadVL(const char* filename)
{
	FILE * file = fopen(filename, "rb");
	if (file == NULL)
	{
		return false;
	}

	GLuint version;
	fread(&version, 1, 4, file);
	if (version == 1)
	{
		fread(&width, 1, 4, file);
		fread(&height, 1, 4, file);
		fread(&depth, 1, 4, file);
		fread(&widthSpacing, 1, 4, file);
		fread(&heightSpacing, 1, 4, file);
		fread(&depthSpacing, 1, 4, file);
		fread(&channels, 1, 4, file);
		GLuint voxelDepth;
		fread(&voxelDepth, 1, 4, file);
		bytes_per_channel = voxelDepth / (8 * channels);

		resize(width, height, depth, channels, bytes_per_channel);

		fread(data, bytes_per_channel, width*height*depth*channels*bytes_per_channel, file);
	}
	else
	{
		std::cerr << "Version not supported: " << version << std::endl;
		fclose(file);
		return false;
	}

	fclose(file);
	return true;
}

// http://paulbourke.net/dataformats/pvm/
// samples: http://schorsch.efi.fh-nuernberg.de/data/volume/
bool Volume::loadPVM(const char* filename)
{
	data = parsePVM(filename, &width, &height, &depth, &channels, &widthSpacing, &heightSpacing, &depthSpacing);
	bytes_per_channel = 1;

	if (data == NULL) return false;
	return true;
}