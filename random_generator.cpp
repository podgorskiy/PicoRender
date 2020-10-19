#include <inttypes.h>
#include <random>
#include <mutex>

namespace rnd
{
	std::mt19937 gen(5);
	std::mutex m;

	uint32_t _rand()
	{
		std::lock_guard<std::mutex> g(m);
		return uint32_t(gen());
	}

	double rand()
	{
		uint32_t a = uint32_t(gen()) >> 5U;
		uint32_t b = uint32_t(gen()) >> 6U;
		return ((a * 67108864.0 + b) / 4503599627370496.0 - 1.0) * 0.5 + 0.5;
	}

	double rand(float a, float b)
	{

		return rand() * (b - a) + a;
	}
}
