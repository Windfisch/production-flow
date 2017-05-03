#include <cassert>
#include <iostream>

#include "factory.hpp"
#include "flowgraph.hpp"
#include "actiongraph.hpp"
#include "read_factory.h"

using namespace std;

static const string delimiter = "\n\n=====================================================================\n\n\n";

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

	ActionGraph actiongraph(&factory);
	auto result = actiongraph.dijkstra(conf);

	cout << endl << endl << endl << endl;
	
	cout << "it follows the node/edge map. for item i, a->b means that flowgraph-node/edge a\n"
	        "corresponds to factory-facility/transportline b. Note that b is given as zero\n"
		"indexed number, but the .tgf file is one-based-indexed. Add 1 before looking it up" << endl;
	for (int i=0; i<MAX_ITEM; i++)
	{
		cout << "item " << i << ": facilities ";
		for (int j=0; j<factory.facility_toposort[i].size(); j++)
			cout << j<<"->"<<factory.facility_toposort[i][j] << ", ";
		cout << "transport lines ";
		for (int j=0; j<factory.edge_table_per_item[i].size(); j++)
			cout << j<<"->"<<factory.edge_table_per_item[i][j] << ", ";
		cout << endl;
	}

	cout << delimiter;
	cout << "now simulating the entire factory in its before-state" << endl;
	factory.simulate_debug(conf);
	
	cout << delimiter;
	cout << "now simulating the entire factory in its after-state" << endl;
	factory.simulate_debug(result.first);
	cout << delimiter;
	cout << "bye." << endl;

	return 0;
}
