#include "TimeSystem.h"

TimeSystem* TimeSystem::_instance = nullptr;

TimeSystem::TimeSystem() : _lastTimeStep(0.0f)
{
	_instance = this;
	_startTime = std::chrono::high_resolution_clock::now();
	_lastTime = _startTime;
}

TimeSystem::~TimeSystem()
{

}

void TimeSystem::Update() noexcept
{
	auto t = std::chrono::high_resolution_clock::now();
	float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(t - _lastTime).count() / 1000000.0f;
	if (deltaTime < 1.0f)
	{
		_lastTimeStep = deltaTime;
	}
	else
	{
		WARN("Lagging with delta time over 1s (Probably resize or refresh event)");
	}

	_lastTime = t;

}

double TimeSystem::GetTotalTime() noexcept
{
	auto t = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(t - _startTime).count() / 1000000.0;
}

float TimeSystem::GetDeltaTime() noexcept
{
	return _lastTimeStep;
}

Time TimeSystem::GetTime() noexcept
{
	Update();
	return Time{ _lastTimeStep, GetTotalTime() };
}
