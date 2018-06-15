#include "experiment.h"

#ifdef __ESTER_BREACHED__
#include <mpi.h>
#include <boost/make_shared.hpp>
#include "marocco/mapping.h"

boost::shared_ptr<ObjectStore> gMapResult;
#else
#include "jobmgrif.h"
#endif


Experiment::Experiment(const ExperimentInfo & info) :
    mInfo(info)
{
	// TODO: shouldn't the hole connect code be somewhere within the
	//       constructor?
}

void Experiment::setEster(boost::shared_ptr<PyEster const> const& ester)
{
	mEster = ester;
}

state_t Experiment::State() const
{
#ifdef __ESTER_BREACHED__
	if (gMapResult) {
		return FINISHED;
	} else {
		return UNINITIALIZED; // FIXME
	}
#else // __ESTER_BREACHED__
#ifdef __ESTER_SERVER_MODE__
	if (!mEster)
	{
		throw std::runtime_error("No experiment submitted yet, call Submit first.");
	}
	RcfClient<I_JobManagerInterface> client(RCF::TcpEndpoint(mEster->settings.host,
	                                                         mEster->settings.port));
	client.getClientStub().ping();
	std::cout << "Connected to RCF server" << std::endl;

	client.getClientStub().getTransport().setMaxMessageLength(ESTER_RCF_MAXMSGSZ);
	//client->getClientStub().requestTransportFilters(RCF::FilterPtr(new RCF::ZlibStatefulCompressionFilter()));
	//client->getClientStub().disableBatching();
	//client->getClientStub().setMaxBatchMessageLength(0);

	// create session object/job handler on the server
	client.getClientStub().createRemoteSessionObject("JobManagerInterface");
#else
	JobManagerInterface client;
#endif

	return client.state(mHandle);
#endif // __ESTER_BREACHED__
}

job_handle_t Experiment::Submit(const ObjectStore & store,
                                std::function<void(state_t, double)> /*progress*/)
{
#ifdef __ESTER_BREACHED__
	gMapResult = boost::make_shared<ObjectStore>(store);
	marocco::mapping::run(gMapResult);

	return 0;
#else // __ESTER_BREACHED__
#ifdef __ESTER_SERVER_MODE__
	mEster = store.getMetaData<PyEster>("ester");
	RcfClient<I_JobManagerInterface> client(RCF::TcpEndpoint(mEster->settings.host,
	                                                         mEster->settings.port));

	client.getClientStub().ping();
	std::cout << "Connected to RCF server" << std::endl;

	client.getClientStub().getTransport().setMaxMessageLength(ESTER_RCF_MAXMSGSZ);
	//client->getClientStub().requestTransportFilters(RCF::FilterPtr(new RCF::ZlibStatefulCompressionFilter()));
	//client->getClientStub().disableBatching();
	//client->getClientStub().setMaxBatchMessageLength(0);

	// create session object/job handler on the server
	client.getClientStub().createRemoteSessionObject("JobManagerInterface");
	client.getClientStub().setRemoteCallTimeoutMs(settings.rcf_timeout_ms);
#else
	JobManagerInterface client;
#endif

	mHandle = client.createJob(store);
	return mHandle;
#endif // __ESTER_BREACHED__
}

ObjectStore Experiment::ReceiveResults(job_handle_t job)
{
	static_cast<void>(job); // might be unused (depends on defines)
#ifdef __ESTER_BREACHED__
	if (!gMapResult) {
		throw std::runtime_error("No results available yet.");
	}
	boost::shared_ptr<ObjectStore> tmp = gMapResult;
	gMapResult.reset();
	return *tmp;
#else // __ESTER_BREACHED__
#ifdef __ESTER_SERVER_MODE__
	if (!mEster) {
		throw std::runtime_error("No experiment submitted yet, call Submit first.");
	}
	RcfClient<I_JobManagerInterface> client(RCF::TcpEndpoint(mEster->settings.host,
	                                                         mEster->settings.port));

	client.getClientStub().ping();
	std::cout << "Connected to RCF server" << std::endl;

	client.getClientStub().getTransport().setMaxMessageLength(ESTER_RCF_MAXMSGSZ);

	// create session object/job handler on the server
	client.getClientStub().createRemoteSessionObject("JobManagerInterface");
	client.getClientStub().setRemoteCallTimeoutMs(settings.rcf_timeout_ms);
#else
	JobManagerInterface client;
#endif

	return client.getJob(job);
#endif // __ESTER_BREACHED__
}
