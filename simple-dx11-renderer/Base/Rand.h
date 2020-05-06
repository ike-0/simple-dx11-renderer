#pragma once

class Rand
{
public:
    inline void Initialize() {
        seed = 51241412;
    }
    inline UINT FastUINT(UINT limit)
    {
        // Make a mask that has all 1s in the bottom few bits.
        // This reduces the number of iterations of the loop to ~1
        int leading_zeros = __builtin_clz(limit);
        int mask = 0xffffffff >> leading_zeros;

        // Loop until our result is in range using rotate and xor.
        do {
            seed = (seed << 1) ^ ((seed >> 31) & 0xa53a9be9);
        } while ((seed & mask) >= limit);

        return seed & mask;
    }
private:
    UINT seed;

};