#pragma once
#include "Systems/Blocks/Block.h"

//#define SETBIT

//class GridX
//{
//public:
//	inline GridX() {
//		for (UINT i = 0; i < MAX_BLOCKS; i++)
//		{
//			blocks[i] = Block();
//		}
//	}
//	inline ~GridX() {};
//
//	std::array<Block, MAX_BLOCKS> blocks;
//
//	Block operator[](int idx) {
//		return blocks[idx];
//	}
//
//};
//
//class GridZ
//{
//public:
//	inline GridZ() {
//		for (UINT i = 0; i < MAX_BLOCKS; i++)
//		{
//			X[i] = GridX();
//		}
//	}
//	inline ~GridZ() {};
//	std::array<GridX, MAX_BLOCKS> X;
//
//	GridX operator[](int idx) {
//		return X[idx];
//	}
//};


//class GridY
//{
//public:
//	std::vector<GridZ> Z;
//
//	GridZ operator[](int idx) {
//		return Proxy(_arrayofarrays[index]);
//	}
//};

class Block2DGrid
{
public:
	inline Block2DGrid() : zmask(0), xmask(0) {
		for (UINT i = 0; i < MAX_BLOCKS * MAX_BLOCKS; i++)
		{
			blocks[i] = Block();
		}
	};
	inline ~Block2DGrid() {};

	inline void AddBlock(int z, int x, const Block& block)
	{
#ifdef _DEBUG
		assert(z < MAX_BLOCKS - 1);
		assert(x < MAX_BLOCKS - 1);
#endif
		blocks[z * MAX_BLOCKS + x] = block;
		if (block.opaque)
		{
			Bit::Set(zmask, z);
			Bit::Set(xmask, x);
		}
	}
	inline void AddBlock(int z, int x, const Block* block)
	{
#ifdef _DEBUG
		assert(z < MAX_BLOCKS - 1);
		assert(x < MAX_BLOCKS - 1);
#endif
		blocks[z * MAX_BLOCKS + x] = *block;
		if (block->opaque)
		{
			Bit::Set(zmask, z);
			Bit::Set(xmask, x);
		}
	}

	constexpr Block* Get(int z, int x)
	{
		return &blocks[z * MAX_BLOCKS + x];
	}

	Block2DGrid(const Block2DGrid&) = delete;
	Block2DGrid& operator=(const Block2DGrid&) = delete;

	// Move constructor
	Block2DGrid(Block2DGrid&& grid) noexcept
		: zmask(grid.zmask), xmask(grid.xmask), blocks(std::move(grid.blocks)) { };

	UINT zmask;
	UINT xmask;
	std::array<Block, MAX_BLOCKS * MAX_BLOCKS> blocks;

};

//////// Y is free floating grid, as no Y min/max is defined, atlest it was
class BlockGrid
{
public:
	inline BlockGrid() {

	};
	inline ~BlockGrid() {};

	inline void AddSubgrid(int y) {
		subgrids.emplace_back();
		_lookups.emplace(y, subgrids.size() - 1);
	}
	inline void Add(int y, int z, int x, const Block& block) {
		GetSubgrid(y)->AddBlock(z, x, block);
	}
	inline void Add(int y, int z, int x, const Block* block) {
		GetSubgrid(y)->AddBlock(z, x, block);
	}

	inline Block* Get(int y, int z, int x)
	{
		return GetSubgrid(y)->Get(z, x);
	}

	inline Block* Data()
	{
		return &subgrids[0].blocks[0];
	}

	inline UINT Size()
	{
		return subgrids.size() * MAX_BLOCKS * MAX_BLOCKS;
	}

	std::vector<Block2DGrid> subgrids;

private:

	Block2DGrid* GetSubgrid(int y)
	{
		auto res = _lookups.find(y);
		if (res == _lookups.end())
		{
			AddSubgrid(y); // Insert new one
			return &subgrids[_lookups[y]];
		}
		return &subgrids[res->second];
	}

	std::map<int, int> _lookups;
};
