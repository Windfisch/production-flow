#include "flowgraph.hpp"
#include "factory.hpp"

#include <string>
#include <cassert>

using namespace std;

const size_t INVALID_INDEX = SIZE_MAX;

vector<size_t> Factory::collect_relevant_facilities(item_t item) const
{
	vector<size_t> relevant_facilities;

	for (size_t facility_id = 0; facility_id < facilities.size(); facility_id++)
	{
		const auto& facility = facilities[facility_id];

		if (facility.ingredients.find(item) != facility.ingredients.end() ||
			facility.products.find(item) != facility.products.end())
		{
			relevant_facilities.push_back(facility_id);
		}
		#ifndef NDEBUG
		else // assert that there is no `item`-edge from or to that facility
		{
			for (const auto& transport_line : transport_lines)
				if (transport_line.item_type == item)
					assert(transport_line.from != facility_id
						&& transport_line.to != facility_id);
		}
		#endif
	}
	
	return relevant_facilities;
}

static size_t pop_root_node(vector<size_t>& relevant_facilities, const vector< unordered_set<size_t> >& incident_edges)
{
	for (size_t& facility_id : relevant_facilities)
		if (incident_edges[facility_id].empty())
		{
			size_t result = facility_id;

			facility_id = relevant_facilities.back();
			relevant_facilities.pop_back();

			return result;
		}

	return INVALID_INDEX;
}

void Factory::initialize()
{
	build_topological_sort();
	build_edge_table();
}

void Factory::build_topological_sort()
{
	facility_toposort.resize(MAX_ITEM);
	facility_toposort_inv.resize(MAX_ITEM);

	for (int item = 0; item < MAX_ITEM; item++)
	{
		vector<size_t>& toposort = facility_toposort[item];
		vector<size_t>& toposort_inv = facility_toposort_inv[item];
		toposort.clear();
		toposort_inv.resize(facilities.size());


		vector<size_t> relevant_facilities = collect_relevant_facilities(item_t(item));


		// calculate incident / outgoing edges
		vector< unordered_set<size_t> > incident_edges(facilities.size());
		vector< unordered_set<size_t> > outgoing_edges(facilities.size());
		for (size_t edge_id = 0; edge_id < transport_lines.size(); edge_id++)
		{
			const auto& edge = transport_lines[edge_id];

			if (edge.item_type == item)
			{
				incident_edges[edge.to].insert(edge_id);
				outgoing_edges[edge.from].insert(edge_id);
			}
		}


		// do the actual topological sorting
		size_t root;
		while ((root = pop_root_node(relevant_facilities, incident_edges)) != INVALID_INDEX)
		{
			toposort.push_back(root);
			toposort_inv[root] = toposort.size()-1;

			for (size_t edge_id : outgoing_edges[root])
				incident_edges[transport_lines[edge_id].to].erase(edge_id);
		}

		for (const auto& edges : incident_edges)
			if (!edges.empty())
				throw runtime_error("Factory is not a directed acyclic graph for item " + to_string(item));
	}
}

void Factory::build_edge_table()
{
	edge_table_per_item.resize(MAX_ITEM);
	edge_table_per_item_inv.resize(MAX_ITEM);

	for (size_t item = 0; item < MAX_ITEM; item++)
	{
		edge_table_per_item[item].clear();
		edge_table_per_item_inv[item].resize(transport_lines.size());

		for (size_t i = 0; i < transport_lines.size(); i++)
			if (transport_lines[i].item_type == item)
			{
				edge_table_per_item[item].push_back(i);
				edge_table_per_item_inv[item][i] = edge_table_per_item[item].size()-1;
			}
	}
}

FlowGraph Factory::build_flowgraph(item_t item, const Factory::FactoryConfiguration& conf) const
{
	const auto& toposort = facility_toposort[item];
	const auto& toposort_inv = facility_toposort_inv[item];
	const auto& edge_table = edge_table_per_item[item];

	FlowGraph flowgraph;
	flowgraph.edges.reserve(edge_table.size());
	flowgraph.nodes.reserve(toposort.size());


	// insert all facilities that are relevant for `item` into the flowgraph,
	// topologically sorted from producers to consumers.
	for (size_t facility_index : toposort)
	{
		const auto& facility = facilities[facility_index];
		size_t level = conf.facility_levels[facility_index];
		double production_rate = facility.upgrade_plan[level].production_or_consumption.at(item);

		flowgraph.nodes.emplace_back(production_rate);
	}

	// insert all transport lines that are relevant for `item`
	for (size_t edge_index = 0; edge_index < edge_table.size(); edge_index++)
	{
		const auto& edge = transport_lines[edge_table[edge_index]];
		assert(edge.item_type == item);
		size_t level = conf.transport_levels[edge_table[edge_index]];
		double capacity = edge.upgrade_plan[level].capacity;
	
		flowgraph.edges.emplace_back(capacity);
	}

	// fill the nodes' edgetables.
	for (size_t edge_index = 0; edge_index < edge_table.size(); edge_index++)
	{
		const auto& edge = transport_lines[edge_table[edge_index]];
		assert(edge.item_type == item);

		flowgraph.nodes[toposort_inv[edge.from]].outgoing_edges.push_back(
			&flowgraph.edges[edge_index]);
		flowgraph.nodes[toposort_inv[edge.to  ]].incoming_edges.push_back(
			&flowgraph.edges[edge_index]);
	}

	return flowgraph;
}
