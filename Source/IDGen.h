#pragma once

#include <atomic>

#include "Common.h"

using UniqueID = uintptr_t;
using NodeID = UniqueID;
using PinID = UniqueID;
using LinkID = UniqueID;

namespace IDGen
{
	extern std::atomic<UniqueID> Allocator;

	inline void Init(UniqueID initialValue)
	{
		initialValue = std::max((UniqueID) 1, initialValue);
		Allocator = initialValue;
	}

	inline UniqueID Generate()
	{
		return Allocator.fetch_add(1);
	}
}