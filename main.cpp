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

	factory.facilities.emplace_back(fcv{ {{{COAL, 10.}}, 1.}  });
	factory.facilities.emplace_back(fcv{ {{{COAL, 20.}}, 1.}  });
	factory.facilities.emplace_back(fcv{ {{{COAL, 25.}}, 1.}  }); // was: 20.
	factory.facilities.emplace_back(fcv{ {{{COAL, 15.}}, 1.}  });

	factory.facilities.emplace_back(fcv{ {{{COAL, -2.}}, 1.}  });
	factory.facilities.emplace_back(fcv{ {{{COAL, 0}}, 1.}  });
	factory.facilities.emplace_back(fcv{ {{{COAL, -24}}, 1.}  });
	factory.facilities.emplace_back(fcv{ {{{COAL, -2}}, 1.}  });
	factory.facilities.emplace_back(fcv{ {{{COAL, -4}}, 1.}  });
	factory.facilities.emplace_back(fcv{ {{{COAL, -3}}, 1.}  });
	factory.facilities.emplace_back(fcv{ {{{COAL, 0}}, 1.}  });
	factory.facilities.emplace_back(fcv{ {{{COAL, 0}}, 1.}  });
	factory.facilities.emplace_back(fcv{ {{{COAL, -8}}, 1.}  });
	factory.facilities.emplace_back(fcv{ {{{COAL, -7}}, 1.}  });
	factory.facilities.emplace_back(fcv{ {{{COAL, -14}}, 1.}  });
	
	factory.transport_lines.emplace_back(COAL, 0,4,   tcv{ {50., 1.} });
	factory.transport_lines.emplace_back(COAL, 4,5,   tcv{ {50., 1.} });
	factory.transport_lines.emplace_back(COAL, 1,5,   tcv{ {50., 1.} });
	factory.transport_lines.emplace_back(COAL, 5,6,   tcv{ {50., 1.} });
	factory.transport_lines.emplace_back(COAL, 6,7,   tcv{ {50., 1.} });
	factory.transport_lines.emplace_back(COAL, 7,11,  tcv{ {50., 1.} });
	
	factory.transport_lines.emplace_back(COAL, 2,8,   tcv{ {50., 1.} });
	factory.transport_lines.emplace_back(COAL, 2,9,   tcv{ {50., 1.} });
	factory.transport_lines.emplace_back(COAL, 8,10,  tcv{ {50., 1.} });
	factory.transport_lines.emplace_back(COAL, 10,11, tcv{ {50., 1.} });
	
	factory.transport_lines.emplace_back(COAL, 11,12, tcv{ {50., 1.} });
	factory.transport_lines.emplace_back(COAL, 11,13, tcv{ {50., 1.} });
	factory.transport_lines.emplace_back(COAL, 11,14, tcv{ {50., 1.} });
	factory.transport_lines.emplace_back(COAL, 3,14,  tcv{ {50., 1.} });

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
