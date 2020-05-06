#pragma once

class TimeSystem
{
public:
	TimeSystem();
	~TimeSystem();


	Time GetTime() noexcept;

	static inline TimeSystem* GetInstance() noexcept
	{
		return _instance;
	}

private:
	void Update() noexcept;

	double GetTotalTime() noexcept;
	float GetDeltaTime() noexcept;

	std::chrono::time_point<std::chrono::steady_clock> _startTime;
	std::chrono::time_point<std::chrono::steady_clock> _lastTime;
	float _lastTimeStep;

	static TimeSystem* _instance;
};