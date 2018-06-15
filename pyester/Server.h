#pragma once

#include <string>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>

class Server
{
public:
	std::string  host;
	size_t       port;

	Server();

private:
	friend class boost::serialization::access;
	template<typename Archive>
	void serialize(Archive& ar, const unsigned int version);
};

// implementation
template<typename Archive>
void Server::serialize(Archive& ar, const unsigned int)
{
	ar & BOOST_SERIALIZATION_NVP(host)
		& BOOST_SERIALIZATION_NVP(port);
}
