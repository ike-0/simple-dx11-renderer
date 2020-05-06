#pragma once
#include "Systems/DataSystem.h"
#include "Graphics/Objects/Renderer.h"
#include "Systems/Chunks/Chunk.h"

class ChunkSystem
{
public:
	ChunkSystem(DataSystem* datasys, Renderer* renderer);
	~ChunkSystem();

	void DrawChunks();

	std::vector<Chunk*>	activeChunks;
private:
	//void DrawBlock(const Block& block);
	DataSystem* _datasys;
	Renderer* _renderer;

};