#pragma once
#include "types.h"

namespace rnd
{
	class RandomState
	{
	public:
		RandomState()
		{}

		explicit RandomState(uint64_t seed): seedA(seed >> 32U), seedB(seed)
		{
		    for (int i=0;i<10;++i)
		    {
		    	gen();
		    }
		}

		uint32_t irand()
		{
			return uint32_t(gen());
		}

		float rand1()
		{
			uint32_t x = (gen()>>9U) | 0x3f800000U;
			return reinterpret_cast<const float&>(x) - 1.0;
		}

		vec2 rand2()
		{
			uint32_t a = (gen()>>9U) | 0x3f800000U;
			uint32_t b = (gen()>>9U) | 0x3f800000U;
			return vec2(reinterpret_cast<const float&>(a), reinterpret_cast<const float&>(b)) - 1.0;
		}

		vec3 rand3()
		{
			uint32_t a = (gen()>>9U) | 0x3f800000U;
			uint32_t b = (gen()>>9U) | 0x3f800000U;
			uint32_t c = (gen()>>9U) | 0x3f800000U;
			return vec3(reinterpret_cast<const float&>(a), reinterpret_cast<const float&>(b), reinterpret_cast<const float&>(c)) - 1.0;
		}

		uint32_t gen()
		{
			uint32_t x = seedA;
			uint32_t y = seedB;
			seedA = y;
			x ^= x << 23U;
			seedB = x ^ y ^ (x >> 17U) ^ (y >> 26U);
		    uint32_t n = seedB + y;
			return n * (n * n * 15731U + 789221U) + 1376312589U;
		}

		float rand(float a, float b)
		{
			return rand1() * (b - a) + a;
		}

	private:
		uint32_t seedA = 0x9c127997U;
		uint32_t seedB = 0x140b75b2U;
	};
}
