#pragma once
#include "Systems/Blocks/BlockGrid.h"


class Chunk
{
public:
	inline Chunk() {
		blocks.AddSubgrid(0);
	};
	inline ~Chunk() {};

	BlockGrid blocks;
	DirectX::XMINT2 pos;
};