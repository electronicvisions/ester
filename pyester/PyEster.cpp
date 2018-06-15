#include "pyester/PyEster.h"

std::string
PyEster::name() const
{
	return "ester";
}

PyEster::PyEster()
{}

boost::shared_ptr<PyEster> PyEster::create()
{
	return boost::shared_ptr<PyEster>(new PyEster);
}
