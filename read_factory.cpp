#include "read_factory.h"
#include "factory.hpp"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <map>

using namespace std;

constexpr int SCALING_FACTOR = 1000;

static map<string, map<item_t, double>> recipes = { // FIXME: this is a misnomer
	{"coal", {{COAL,1}}},
	{"iron-ore", {{IRON_ORE,1}}},
	{"iron-plate", {{IRON_PLATE,1}, {IRON_ORE,-1}, {COAL, -0.2}}},
	{"copper-ore", {{COPPER_ORE,1}}},
	{"copper-plate", {{COPPER_PLATE,1}, {COPPER_ORE,-1}, {COAL, -0.2}}},
	{"circuit", {{CIRCUIT,1}, {COPPER_PLATE, -1.5}, {IRON_PLATE, -1}}},
	{"red-pot", {{RED_POT,1}, {IRON_PLATE,-2}, {COPPER_PLATE,-1}}},
	{"green-pot", {{GREEN_POT,1}, {IRON_PLATE,-5}, {CIRCUIT,-1}}},
	{"stuff", {{IRON_PLATE, -4}, {COPPER_PLATE, -2}, {CIRCUIT, -3}}},
	{"pumpjack", {{IRON_PLATE, -4}, {COPPER_PLATE, -1}, {CIRCUIT, -3}, {STEEL_PLATE, -2}}},
	{"steel-plate", {{IRON_PLATE, -3.5}, {STEEL_PLATE, 1}}},
	{"pipe", {{PIPE, 1}, {IRON_PLATE, -1}}},
	{"pumpjack-sink", {}},
	{"pipe-sink", {}}
};

static map<string, item_t> item_lookup = {
	{"coal", COAL},
	{"iron-ore", IRON_ORE},
	{"iron-plate", IRON_PLATE},
	{"copper-ore", COPPER_ORE},
	{"copper-plate", COPPER_PLATE},
	{"steel-plate", STEEL_PLATE},
	{"pipe", PIPE},
	{"circuit", CIRCUIT},
	{"red-pot", RED_POT},
	{"green-pot", GREEN_POT},
	{"pumpjack", PUMPJACK}
};

Factory read_factory(string file)
{
	ifstream f(file);
	if (!f.good())
		throw runtime_error("could not open file '"+file+"'");
	
	Factory factory;
	string line;

	// parse facilities
	while (getline(f, line))
	{
		if (line == "#") break;

		size_t p1 = line.find(' ');
		if (p1 == string::npos)
			throw runtime_error("invalid format");
	
		if (p1 == line.length()-1)
		{
			vector<Factory::FacilityConfiguration> upgrade_plan;
			Factory::FacilityConfiguration conf;
			upgrade_plan.push_back(conf); // push an empty configuration. this is a splitter node

			factory.facilities.emplace_back(upgrade_plan);
		}
		else
		{
			size_t p2 = line.find(' ', p1+1);
			if (p2 == string::npos)
				throw runtime_error("invalid format");

			size_t p3 = line.find('/', p2+1);
			if (p3 == string::npos)
				throw runtime_error("invalid format");

			string recipe = line.substr(p1+1, p2-p1-1);
			double current = stod(line.substr(p2+1, p3-p2-1));
			double maximum = stod(line.substr(p3+1));

			cout << "recipe '" << recipe << "' with " << current << "/" << maximum << endl;

			vector<Factory::FacilityConfiguration> upgrade_plan;
			
			for (int i=0; i<5; i++)
			{
				Factory::FacilityConfiguration conf;
				
				for (auto iter : recipes.at(recipe))
					conf.production_or_consumption[iter.first] = int(SCALING_FACTOR * iter.second * (current + (maximum-current)*i/5.));
				upgrade_plan.push_back(conf);
			}

			factory.facilities.emplace_back(upgrade_plan);
		}
	}

	if (line != "#")
		throw runtime_error("invalid format: could not find edge list");

	while (getline(f, line))
	{
		size_t p1 = line.find(' ');
		if (p1 == string::npos)
			throw runtime_error("invalid format");
		size_t p2 = line.find(' ',p1+1);
		if (p2 == string::npos)
			throw runtime_error("invalid format");
		size_t p3 = line.find(' ',p2+1);
		if (p3 == string::npos)
			throw runtime_error("invalid format");

		int from = stoi(line.substr(0, p1));
		int to = stoi(line.substr(p1+1,p2-p1-1));
		string item = line.substr(p2+1,p3-p2-1);
		string dist_x = line.substr(p3+1);

		bool exists = (dist_x.substr(dist_x.length()-1) != "x");
		double dist = stod(dist_x);

		cout << "transport of " << item << " " << from << "->" << to << ", distance " << dist << " (line exists=" << exists << ")" << endl;

		vector<Factory::TransportLineConfiguration> upgrade_plan;

		upgrade_plan.emplace_back(Factory::TransportLineConfiguration{int(SCALING_FACTOR * 1*13.3), dist}); // one yellow
		upgrade_plan.emplace_back(Factory::TransportLineConfiguration{int(SCALING_FACTOR * 2*13.3), dist}); // two yellow
		upgrade_plan.emplace_back(Factory::TransportLineConfiguration{int(SCALING_FACTOR * 3*13.3), 4*dist}); // red+yellow
		upgrade_plan.emplace_back(Factory::TransportLineConfiguration{int(SCALING_FACTOR * 4*13.3), 4*dist}); // red+red
		upgrade_plan.emplace_back(Factory::TransportLineConfiguration{int(SCALING_FACTOR * 5*13.3), 15*dist}); // blue+red
		upgrade_plan.emplace_back(Factory::TransportLineConfiguration{int(SCALING_FACTOR * 6*13.3), 15*dist}); // blue+blue

		factory.transport_lines.emplace_back(item_lookup.at(item), from-1, to-1, upgrade_plan);
	}

	return factory;
}
