#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "jobmgrif.h"
#include "jobmgr.h"

extern boost::filesystem::path mapper_cmd;

#ifdef __ESTER_SERVER_MODE__
void init_rcf(size_t port) {
	using namespace RCF;


    RcfServer* server = new RCF::RcfServer(RCF::TcpEndpoint(port));
	std::cout << "starting RCF server" << std::endl;

	// set max msg size
	server->getServerTransport().setMaxMessageLength(ESTER_RCF_MAXMSGSZ);

	// create threadpool if more than one thread shall be started
	if (ESTER_RCF_THREADS > 1) {
		std::cout << "starting up to " << ESTER_RCF_THREADS << " threads." << std::endl;
		server->setThreadPool(RCF::ThreadPoolPtr(new ThreadPool(1, ESTER_RCF_THREADS, "Ester RCF Server", 100)));
	}

	SessionObjectFactoryServicePtr sofs(new SessionObjectFactoryService());
	server->addService(sofs);

	// bind functions to service
    sofs->bind<I_JobManagerInterface, JobManagerInterface>("JobManagerInterface");

	// compression?
	if (ESTER_RCF_ZLIB) {
		RCF::FilterServicePtr filterServicePtr(new RCF::FilterService());
		filterServicePtr->addFilterFactory( RCF::FilterFactoryPtr(new RCF::ZlibStatefulCompressionFilterFactory()));
		server->addService(filterServicePtr);
	}

	server->start();
}
#endif // __ESTER_SERVER_MODE__

void find_mapper_executable()
{
	std::string cmd = std::string("which ") + mapper_cmd.string() + std::string(" >> /dev/null");
	if (std::system(cmd.c_str()))
		throw std::runtime_error("mapper executable not found in $PATH");
}

void main_loop() {
	JobManager& jm = JobManager::instance();
	static_cast<void>(jm);

	while (true) {

		boost::this_thread::yield();
		boost::this_thread::sleep(boost::posix_time::seconds(1));

		// do something meaningfull
	}
}

int main(int argc, char** argv)
{
	size_t port;
	// command line parsing
	namespace po = boost::program_options;
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce this help message")
		("np",   po::value<size_t>(&n_processes_per_spawn)->default_value(1),
			 "number of processes per job")
		("max", po::value<size_t>(&max_concurrent_jobs)->default_value(20),
			 "number of max concurrent jobs")
		("port", po::value<size_t>(&port)->default_value(ESTER_RCF_PORT),
			 "port to use")
		("job_starter", po::value<size_t>(&n_concurrent_job_starter)->default_value(1),
			"number of concurrent job starter (DANGEROUS!)")
		("cmd", po::value<boost::filesystem::path>(&mapper_cmd)->default_value("mapper"),
			"mapper command to use")
		("margs", po::bool_switch(), "pass unknown commands to mapper")
	;

	po::variables_map vm;
	auto parser = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
	po::store(parser, vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 1;
	}

	// if margs is present, forward all unknown arguments to mapper
	if (vm["margs"].as<bool>())
		margv = po::collect_unrecognized(parser.options, po::include_positional);

	find_mapper_executable();

	// initialize MPI
	int const thread_level = MPI::Init_thread(argc, argv,
		n_concurrent_job_starter > 1 ? MPI_THREAD_MULTIPLE : MPI_THREAD_SERIALIZED);
	int const comm_size    = MPI::COMM_WORLD.Get_size();

	// ester server doesn't make use of multiple MPI process for various
	// reasons, e.g. rcf can bind only once to server port and more complicated
	// distributed job management.
	if (comm_size != 1)
		throw std::runtime_error("Ester server doesn't support multiple MPI processes");

	// check available MPI thread model
	if (n_concurrent_job_starter > 1 && thread_level < MPI_THREAD_MULTIPLE)
		throw std::runtime_error("your MPI installation doesn't support threads");

	// set errorhandler to throw exceptions
	MPI::COMM_WORLD.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);

#ifdef __ESTER_SERVER_MODE__
	init_rcf(port);
#endif // __ESTER_SERVER_MODE__

	main_loop();

	MPI::Finalize();
	return EXIT_SUCCESS;
}
