#include <iostream>
#include <boost/python.hpp>
#include <Python.h>

namespace py = boost::python;

int call_pyfile() {
    Py_Initialize();
    py::object main_module = py::import("__main__");
    py::object main_namespace = main_module.attr("__dict__");
    py::exec("print 'Hello, world'", main_namespace);
    py::exec("print 'Hello, world'[3:5]", main_namespace);
    py::exec("print '.'.join(['1','2','3'])", main_namespace);

    py::exec("import hello", main_namespace);

	// spackt?
	//py::object rand = py::eval("random.random()", main_namespace);
    //std::cout << py::extract<double>(rand) << std::endl;

    // import the random module
    py::object rand_mod = py::import("random");
    // extract the random function from the random module (random.random())
    py::object rand_func = rand_mod.attr("random");
    // call the function and extract the result
    py::object rand2 = rand_func();
    std::cout << py::extract<double>(rand2) << std::endl;
}
