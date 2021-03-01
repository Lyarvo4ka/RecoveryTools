

#include "raw\factories.h"

namespace RAW
{
	void RawFactoryManager::Register(const std::string & algorithmName, RawFactoryPtr rawFactory)
	{
		if (factories_.find(algorithmName) == factories_.end())
		{
			factories_.emplace(algorithmName, std::move(rawFactory));
		}
	}

	RAW::RawFactory * RawFactoryManager::Lookup(const std::string & algorithmName)
	{
		auto findIter = factories_.find(algorithmName);
		if (findIter != factories_.end())
		{
			return findIter->second.get();
		}
		return nullptr;
	}

};
