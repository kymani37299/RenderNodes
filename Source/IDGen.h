#pragma once

#include <atomic>

#include "Common.h"

using UniqueID = uintptr_t;
using NodeID = UniqueID;
using PinID = UniqueID;
using LinkID = UniqueID;
using VariableID = UniqueID;

namespace IDGen
{
	extern std::atomic<UniqueID> Allocator;

	static constexpr UniqueID ReservedIDs = 1024;

	inline void Init(UniqueID initialValue)
	{
		initialValue = std::max((UniqueID) ReservedIDs, initialValue);
		Allocator = initialValue;
	}

	inline UniqueID Generate()
	{
		return Allocator.fetch_add(1);
	}
}