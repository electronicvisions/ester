#pragma once

#include <string>

#include "euter/metadata.h"
#include "pyester/Server.h"

class PyEster : public euter::DerivedMetaData<PyEster>
{
public:
	Server settings;

	virtual std::string name() const;

	static boost::shared_ptr<PyEster> create();
private:
	PyEster();

	friend class boost::serialization::access;
	template<typename Archive>
	void serialize(Archive& ar, unsigned int const);

};

// implementation
template<typename Archive>
void PyEster::serialize(Archive& ar, unsigned int const)
{
	ar & BOOST_SERIALIZATION_NVP(settings);
}
