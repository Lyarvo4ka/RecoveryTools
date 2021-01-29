
#include "io/uniqueid.h"

namespace IO
{
	IdGenerator* IdGenerator::idGenerator_ = nullptr;

	IdGenerator* IdGenerator::getGenerator()
	{
		if (!idGenerator_)
			idGenerator_ = new IdGenerator;
		return idGenerator_;
	}

	UniqueID IdGenerator::generateID()
	{
		static UniqueID id = 0;
		//++id;
		return id++;

	}
}