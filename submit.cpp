#include "submit.h"
#include "euter/objectstore.h"

// include from ester
#include "experiment.h"


void submit(ObjectStore& local)
{
    auto ester = local.getMetaData<PyEster>("ester");

    Experiment ex;
    ex.setEster(ester);

    job_handle_t handle;
    handle = ex.Submit(local);

    // receive "patched" ObjectStore containing results from ester server
    // after mapping has been carried out and experiment has run.

    // this function blocks, until everything is over
    ObjectStore remote = ex.ReceiveResults(handle);

    // check the result
    local.check();

    // patch local MetaData instances
    for (auto const& md : remote.getMetaData())
    {
        try {
#ifdef __ESTER_BREACHED__
			std::string const& key = md.first;
			auto mdptr = boost::get<boost::shared_ptr<MetaData>>(local.getMetaData(key));
			mdptr = boost::get<boost::shared_ptr<MetaData>>(md.second);
#else
			auto mdptr = boost::get<boost::shared_ptr<MetaData>>(local.getMetaData(md.first));
			mdptr->fromString(boost::get<std::string>(md.second));
#endif
        } catch (std::out_of_range const&) {}
    }

    // patch spikes
	for (size_t pop_idx = 0; pop_idx < remote.populations().size(); pop_idx++) {
		auto const& pop = remote.populations().at(pop_idx);
		// We don't submit back the spike trains of spike sources.
		if (!pop->parameters().is_source()) {
			auto& local_pop = local.populations().at(pop_idx);
			for (size_t neuron = 0; neuron < pop->size(); ++neuron)
				local_pop->getSpikes(neuron) = pop->getSpikes(neuron);
		}
	}
}
