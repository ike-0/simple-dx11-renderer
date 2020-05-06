#pragma once

#define MAX_BLOCKS 32
#define MAX_BLOCKS_Y 64

class Block
{
public:
	inline Block(BOOL _opaque = 0) : opaque(_opaque) { };
	inline ~Block() {};

	BOOL opaque;
};