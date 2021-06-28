
#ifndef VOLUME_H
#define VOLUME_H

#include "includes.h"
#include "framework.h"

#define VOLPOS(x,y,z,w,h,d,c) (c*((x>0?x<w?x:w-1:0)+(y>0?y<h?y:h-1:0)*w+(z>0?z<d?z:d-1:0)*w*h))

//Class to represent a volume
class Volume
{
public:
	unsigned int width;
	unsigned int height;
	unsigned int depth;
	float widthSpacing;
	float heightSpacing;
	float depthSpacing;
	unsigned int channels;
	unsigned int bytes_per_channel;

	Uint8* data; //bytes with the pixel information

	Volume();
	Volume(int w, int h, int d, int channels = 1, int bytes_per_channel = 1);
	~Volume();

	void resize(int w, int h, int d, int channels = 1, int bytes_per_channel = 1);
	void clear();

	void fillSphere();
	void fillNoise(float frequency, int octaves, unsigned int seed);

	bool loadVL(const char* filename);
	bool loadPVM(const char* filename);
};

#endif