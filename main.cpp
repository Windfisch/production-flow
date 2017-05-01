#include <cassert>
#include <iostream>

#include "factory.hpp"
#include "flowgraph.hpp"
#include "actiongraph.hpp"
#include "read_factory.h"

using namespace std;

int main(int argc, const char** argv)
{
	if (argc != 2)
	{
		cout << "Usage: " << argv[0] << " factory.tgf" << endl;
		exit(1);
	}
	
	Factory factory = read_factory(argv[1]);
	factory.initialize();

	Factory::FactoryConfiguration conf;
	for (size_t i=0; i<factory.facilities.size(); i++)
		conf.facility_levels.push_back(0);
	for (size_t i=0; i<factory.transport_lines.size(); i++)
		conf.transport_levels.push_back(0);

	FlowGraph graph = factory.build_flowgraph(COAL, conf);

	graph.dump("initial");
	graph.calculate();
	graph.dump("final");

	ActionGraph actiongraph(&factory);
	actiongraph.dijkstra(conf);

	factory.simulate_debug(conf);

	return 0;
}
