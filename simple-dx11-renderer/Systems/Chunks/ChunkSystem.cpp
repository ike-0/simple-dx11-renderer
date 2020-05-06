#include "ChunkSystem.h"

ChunkSystem::ChunkSystem(DataSystem* datasys, Renderer* renderer) : _datasys(datasys), _renderer(renderer)
{
	activeChunks.push_back(new Chunk());
	Block* b = new Block(1);
	activeChunks[0]->blocks.Add(1, 2, 3, b);

	Block* block = activeChunks[0]->blocks.Get(0, 2, 16);

}

ChunkSystem::~ChunkSystem()
{
	for (UINT i = 0; i < activeChunks.size(); i++)
	{
		delete activeChunks[i];
	}
}

void ChunkSystem::DrawChunks()
{
	//_renderer->DrawBlockGrid(nullptr,0,0,0);


	//for (UINT i = 0; i < activeChunks.size(); i++)
	//{
	//	for(const auto& subgrid : activeChunks[i]->blocks.subgrids)
	//	{
	//		if ((subgrid.xmask == 0U) && (subgrid.zmask == 0U))
	//		{
	//			// Empty grid
	//			continue;
	//		}

	//		//_renderer->DrawBlockGrid()


	//		//for (UINT i = 0; i < subgrid.blocks.size(); i++)
	//		//{
	//		//	DrawBlock(subgrid.blocks[i]);
	//		//}

	//	}

	//}
}

//void ChunkSystem::DrawBlock(const Block* block, UINT count)
//{
//	_renderer->DrawBlocks(block, count);
//}
