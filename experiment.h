#pragma once

#include <functional>

#include "euter/objectstore.h"
#include "pyester/PyEster.h"
#include "typedefs.h"

// TODO define in Euter
struct Results {};
class MappingInterface;

// interface to pyHMF
// Starts the experiment and allows to query results and so on...
struct Experiment
{
	struct NoopProgress
	{
		void operator()(state_t, double) {}
	};

	struct ExperimentInfo
	{
		std::string name;
		std::string username;
		std::string email;
		std::string certificate;
		// ...
	};

	struct ExperimentSettings
	{
		ExperimentSettings() :
			rcf_timeout_ms(600 * 1000)
		{}

		size_t rcf_timeout_ms;
	} settings;

	//Experiment(handle_type);
	Experiment(const ExperimentInfo & info = ExperimentInfo());

	// Asynchronous execution
	job_handle_t Submit(
		const euter::ObjectStore & store,
		std::function<void(state_t, double)> progress = NoopProgress());

	euter::ObjectStore ReceiveResults(job_handle_t job);

	// Block with timeout
	Results GetResults();

	// TODO: implement
	void Abort();

	state_t State() const;

	void setEster(boost::shared_ptr<PyEster const> const& ester);

private:
	job_handle_t mHandle;
	ExperimentInfo mInfo;
	boost::shared_ptr<PyEster const> mEster;
};
