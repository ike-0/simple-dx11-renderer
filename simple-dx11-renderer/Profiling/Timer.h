#pragma once

class Timer
{
public:
	inline Timer() {
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		_freq = static_cast<double>(li.QuadPart) / 1000.0;
	};
	inline ~Timer() {};

	// Starts timer
	inline void Start() {
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		_time = li.QuadPart;
	};
	// Stops timer
	inline float Stop() {
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return static_cast<float>(static_cast<double>(li.QuadPart - _time) / _freq);
	};
private:
	LONGLONG _time;
	double _freq = 0.0;
};