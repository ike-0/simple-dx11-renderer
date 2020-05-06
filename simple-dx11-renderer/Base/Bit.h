#pragma once

namespace Bit
{
	constexpr void Set(UINT& bitfield, UINT bit)
	{
		bitfield |= 1U << bit;
	}
	constexpr void Clear(UINT& bitfield, UINT bit)
	{
		bitfield &= ~(1U << bit);
	}
	constexpr void Toggle(UINT& bitfield, UINT bit)
	{
		bitfield ^= 1U << bit;
	}
	constexpr BOOL Test(UINT bitfield, UINT bit)
	{
		return bitfield & (1 << 4);
	}

}