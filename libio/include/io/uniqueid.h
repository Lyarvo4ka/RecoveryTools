#pragma once

#include <stdint.h>

namespace IO
{
	using UniqueID = uint32_t;

	class IdGenerator
	{
	public:
		IdGenerator(IdGenerator&) = delete;
		void operator=(const IdGenerator&) = delete;

		static IdGenerator* getGenerator();
		UniqueID generateID();

	protected:
		static IdGenerator* idGenerator_;
		IdGenerator() 
		{
		}

	};
}