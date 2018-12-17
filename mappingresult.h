#pragma once

#include <cstdlib>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "euter/objectstore.h"

class MappingResult
{
public:
	MappingResult() :
		error(-1),
		store()
	{}

	int error;
	boost::shared_ptr<euter::ObjectStore> store;

private:
	friend class boost::serialization::access;
	template<typename Archiver>
	void serialize(Archiver& ar, unsigned int const)
	{
		using boost::serialization::make_nvp;
		ar & make_nvp("error", error)
		   & make_nvp("objectstore", store);
	}
};
