#pragma once
#include <vector>
#include <unordered_set>
#include <set>
#include <map>

#include "flowgraph.hpp"

enum item_t
{
	FIRST_ITEM = 0,

	COAL = 0,
	IRON_ORE,
	IRON_PLATE,

	MAX_ITEM // must be last
};


struct Factory
{
	struct FacilityConfiguration
	{
		std::map<item_t, double> production_or_consumption;
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
					if (itemprod.second > 0.)
						products.insert(itemprod.first);
					else
						ingredients.insert(itemprod.first);
		}

		std::vector<FacilityConfiguration> upgrade_plan;

		// dependent / redundant data

		std::set<item_t> ingredients;
		std::set<item_t> products;

		item_t most_advanced_ingredient;
		item_t most_basic_item_involved;
	};

	struct TransportLineConfiguration
	{
		double capacity;
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
};
