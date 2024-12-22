#pragma once
#include <chrono>
#include <unordered_map>

class Timer {
public:
	Timer() = default;

	void startMeasure(uint64_t timerID, float stop) {
		timerMap[timerID] = { std::chrono::system_clock::now(), stop };
	}

	float getElapsedTime(uint64_t timerID) {
		auto end = std::chrono::system_clock::now();
		float elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - timerMap[timerID].start).count();
		return elapsed;
	}

	bool hasTimeElapsed(uint64_t timerID) {
		return timerMap[timerID].stop <= getElapsedTime(timerID);
	}

	void resetTimer(uint64_t timerID) {
		timerMap[timerID].stop = 0;
	}

private:
	struct timeData {
		std::chrono::system_clock::time_point start;
		float stop = 0;
		//std::chrono::system_clock::time_point end;
	};
	std::unordered_map<uint64_t, timeData> timerMap;
};