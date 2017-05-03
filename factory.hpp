#pragma once
#include <vector>
#include <unordered_set>
#include <set>
#include <map>
#include <string>

#include "flowgraph.hpp"

enum item_t
{
	DONE = -1,
	FIRST_ITEM = 0,

	COAL = 0,
	IRON_ORE,
	COPPER_ORE,
	IRON_PLATE,
	COPPER_PLATE,
	STEEL_PLATE,
	PIPE,
	CIRCUIT,
	RED_POT,
	GREEN_POT,
	PUMPJACK,

	MAX_ITEM // must be last
};

extern std::map<item_t, std::string> item_name;

struct Factory
{
	struct FacilityConfiguration
	{
		std::map<item_t, int> production_or_consumption;
		double incremental_cost; // cost for upgrading from one level lower to this one.
		// more data goes here. possibly as a pointer for efficiency
	};

	struct Facility
	{
		Facility(std::vector<FacilityConfiguration> upgrade_plan_) :
			upgrade_plan(std::move(upgrade_plan_))
		{
			for (const auto& conf : upgrade_plan)
				for (const auto& itemprod : conf.production_or_consumption)
					if (itemprod.second != 0)
						items.insert(itemprod.first);
		}

		std::vector<FacilityConfiguration> upgrade_plan;
		item_t most_advanced_item_involved;
		std::set<item_t> items; // this facility is relevant for these items.
		                        // either because it produces/consumes them, or
		                        // because it has edges of that type.
	};

	struct TransportLineConfiguration
	{
		int capacity;
		double incremental_cost;
		// more data goes here.
	};

	struct TransportLine
	{
		TransportLine(item_t it, size_t from_, size_t to_, std::vector<TransportLineConfiguration> upgrade_plan_) :
			item_type(it), from(from_), to(to_), upgrade_plan(std::move(upgrade_plan_)) {}

		item_t item_type;
		size_t from; // index in facilities[]
		size_t to;   // index in facilities[]

		std::vector<TransportLineConfiguration> upgrade_plan;
	};

	struct FactoryConfiguration
	{
		std::vector<size_t> facility_levels;
		std::vector<size_t> transport_levels;
	};

	std::vector<Facility> facilities;
	std::vector<TransportLine> transport_lines;

	// useful methods

	void initialize(); // must be called after filling in the data to initialize dependent data!

	FlowGraph build_flowgraph(item_t item, const Factory::FactoryConfiguration& conf) const;
	void simulate_debug(const FactoryConfiguration& conf) const; // calculates the flow and outputs a graphviz-dot-graph.


	// dependent / redundant data follows

	// facility_toposort[item_level][i] = index_in_facilities, such that
	// the Facilities referenced by i are topologically sorted.
	std::vector< std::vector<size_t> > facility_toposort;
	std::vector< std::vector<size_t> > facility_toposort_inv;
	
	// edge_table_per_item[item_level][i] = index_in_edges
	std::vector< std::vector<size_t> > edge_table_per_item;
	std::vector< std::vector<size_t> > edge_table_per_item_inv;

	private:
		std::vector<size_t> collect_relevant_facilities(item_t item) const;
		void build_topological_sort();
		void build_edge_table();
		void build_facility_itemset();
};
