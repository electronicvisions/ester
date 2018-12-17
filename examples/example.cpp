#include <iostream>

#include <boost/make_shared.hpp>

#include "euter/interface.h"
#include "euter/objectstore.h"
#include "euter/fixedprobabilityconnector.h"
#include "euter/alltoallconnector.h"
#include "euter/nativerandomgenerator.h"

#include "pynn/api.h"

using namespace std;
using namespace euter;

int main(int /*argc*/, char** /*argv*/) {

	ObjectStore::Settings settings;
	ObjectStore os;
	os.setup(settings);

	auto pop0 = Population::create(os, 1, CellType::IF_brainscales_hardware);
	auto pop1 = Population::create(os, 1, CellType::IF_brainscales_hardware);
	auto pop2 = Population::create(os, 12, CellType::IF_brainscales_hardware);
	auto pop3 = Population::create(os, 15, CellType::IF_brainscales_hardware);
	auto pop4 = Population::create(os, 7 , CellType::IF_brainscales_hardware);
	auto popI = Population::create(os, 7 , CellType::SpikeSourcePoisson);

	cout << pop4 << endl;
	// FIXME PyPopulation(pop4).set("tau_m", 5.0);
	cout << pop4 << endl;
	map<string, double> newparams;
	newparams["tau_m"] = 4.5;
	newparams["mist"] = 3.0;
	// PyPopulation(pop4).set(newparams);
	cout << pop4 << endl;

	auto c1 = boost::make_shared<FixedProbabilityConnector>(0.3, true, 1, 0);
	auto c2 = boost::make_shared<FixedProbabilityConnector>(1.0, true, 1, 0);
	auto c3 = boost::make_shared<FixedProbabilityConnector>(0.1, true, 1, 0);
	auto c4 = boost::make_shared<FixedProbabilityConnector>(0.9, true, 1, 0);
	auto c5 = boost::make_shared<FixedProbabilityConnector>(0.2, true, 1, 0);
	auto c6 = boost::make_shared<FixedProbabilityConnector>(0.4, true, 1, 0);
	auto c7 = boost::make_shared<FixedProbabilityConnector>(0.3, true, 1, 0);
	auto c8 = boost::make_shared<FixedProbabilityConnector>(0.3, true, 1, 0);

	boost::shared_ptr<RandomGenerator> rng =
	boost::make_shared<NativeRandomGenerator>(22949142706);

	Projection::create(os, popI, pop0, c1, rng);
	Projection::create(os, pop0, pop1, c2, rng);
	Projection::create(os, pop0, pop2, c3, rng);
	Projection::create(os, pop0, pop3, c4, rng);
	Projection::create(os, pop0, pop4, c5, rng);
	Projection::create(os, pop2, pop3, c6, rng);
	Projection::create(os, pop3, pop2, c7, rng);
	Projection::create(os, pop3, pop4, c8, rng);

	// TODO: test some more connectors

	//// a few more populations
	int const N = 2;
	auto c = boost::make_shared<FixedProbabilityConnector>(0.314, true, 1, 0);
	boost::shared_ptr<Population> p0 = Population::create(os, 17, CellType::IF_brainscales_hardware);
	boost::shared_ptr<Population> p1;

	for (int ii=0; ii<N; ++ii) {
		p1 = Population::create(os, 72, CellType::IF_brainscales_hardware);
		Projection::create(os, p0, p1, c, rng);
		Projection::create(os, p1, p0, c, rng);
		p0 = p1;
	}

	{
		Projection::create(os, p0, p0, boost::make_shared<AllToAllConnector>(), rng);
		Projection::create(os, p0, p0, boost::make_shared<FixedProbabilityConnector>(0.5), rng);
	}

	os.run(1000);
	os.run(1000);
	os.run(1000);
	os.run(1000);

	end();
}
