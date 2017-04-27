#include <cassert>

#include "factory.hpp"
#include "flowgraph.hpp"
#include "actiongraph.hpp"

using namespace std;

int main()
{
	Factory factory;

	typedef vector<Factory::FacilityConfiguration> fcv;
	typedef vector<Factory::TransportLineConfiguration> tcv;

	auto plates = fcv{
		{ { {IRON_PLATE, 2.}, {COAL, -0.5}, {IRON_ORE,  -4.} }, 1. },
		{ { {IRON_PLATE, 4.}, {COAL, -1.0}, {IRON_ORE,  -8.} }, 1. },
		{ { {IRON_PLATE, 6.}, {COAL, -1.5}, {IRON_ORE, -12.} }, 1.5 },
		{ { {IRON_PLATE, 8.}, {COAL, -2.0}, {IRON_ORE, -16.} }, 2.0 }
	};
	auto ore = fcv{
		{ { {COAL, -0.5}, {IRON_ORE, 3.} }, 1. },
		{ { {COAL, -1.0}, {IRON_ORE, 6.} }, 1. },
		{ { {COAL, -1.5}, {IRON_ORE, 9.} }, 1. },
		{ { {COAL, -2.0}, {IRON_ORE, 12.} }, 1. },
		{ { {COAL, -2.5}, {IRON_ORE, 15.} }, 2. }
	};
	auto coal = fcv{
		{ { {COAL, 1} }, 1. },
		{ { {COAL, 2} }, 1. },
		{ { {COAL, 3} }, 1. },
		{ { {COAL, 4} }, 1. },
		{ { {COAL, 5} }, 2. }
	};

	auto cheap_edge = tcv{ {2.5, 1.}, {5.0, 1.}, {7.5, 1.}, {10., 1.}, {15., 2.}, {20., 3.} };
	auto expensive_edge = tcv{ {2.5, 1.}, {5.0, 2.}, {7.5, 5.} };

	factory.facilities.push_back(plates);
	factory.facilities.push_back(plates);
	factory.facilities.push_back(plates);
	factory.facilities.push_back(ore);
	factory.facilities.push_back(ore);
	factory.facilities.push_back(ore);
	factory.facilities.push_back(ore);
	factory.facilities.push_back(coal);
	factory.facilities.push_back(coal);
	factory.facilities.push_back(coal);
	factory.facilities.push_back(coal);
	factory.facilities.push_back(coal);
	
	factory.transport_lines.emplace_back(IRON_ORE, 3,0, cheap_edge);
	factory.transport_lines.emplace_back(IRON_ORE, 4,0, expensive_edge);
	factory.transport_lines.emplace_back(IRON_ORE, 4,1, cheap_edge);
	factory.transport_lines.emplace_back(IRON_ORE, 5,2, expensive_edge);
	factory.transport_lines.emplace_back(IRON_ORE, 6,2, expensive_edge);

	factory.transport_lines.emplace_back(COAL, 11, 3, cheap_edge);
	factory.transport_lines.emplace_back(COAL, 11, 5, expensive_edge);
	factory.transport_lines.emplace_back(COAL, 10, 4, cheap_edge);
	factory.transport_lines.emplace_back(COAL,  9, 5, cheap_edge);
	factory.transport_lines.emplace_back(COAL,  9, 4, expensive_edge);
	factory.transport_lines.emplace_back(COAL,  8, 6, expensive_edge);
	factory.transport_lines.emplace_back(COAL,  7, 6, cheap_edge);

	factory.transport_lines.emplace_back(COAL, 3, 2, cheap_edge);
	factory.transport_lines.emplace_back(COAL, 3, 0, cheap_edge);
	factory.transport_lines.emplace_back(COAL, 5, 1, cheap_edge);
	factory.transport_lines.emplace_back(COAL, 5, 0, cheap_edge);
	factory.transport_lines.emplace_back(COAL, 7, 1, cheap_edge);

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
}
