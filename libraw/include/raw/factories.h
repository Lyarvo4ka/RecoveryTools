#pragma once

//#include "iolibrary_global.h"
#include "io/iodevice.h"
#include <map>
#pragma warning(disable:4251)

namespace RAW
{
	class RawAlgorithm;

	class RawFactory
	{
	public:
		virtual ~RawFactory() {}
		virtual RawAlgorithm * createRawAlgorithm(IO::IODevicePtr) = 0;
	};

	using RawFactoryPtr = std::unique_ptr<RawFactory>;

	class RawFactoryManager
	{
		using FactoryMap = std::map<std::string, RawFactoryPtr>;
	private:
		FactoryMap factories_;
	public:

		void Register(const std::string & algorithmName, RawFactoryPtr  rawFactory);
		RawFactory * Lookup(const std::string & algorithmName);
	};
};