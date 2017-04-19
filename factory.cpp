#include "flowgraph.hpp"
#include "factory.hpp"

#include <string>
#include <cassert>

using namespace std;


vector<int> Factory::collect_relevant_facilities(item_t item) const
{
	vector<int> relevant_facilities;

	for (int facility_id = 0; facility_id < facilities.size(); facility_id++)
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

static int pop_root_node(vector<int>& relevant_facilities, const vector< unordered_set<int> >& incident_edges)
{
	for (int& facility_id : relevant_facilities)
		if (incident_edges[facility_id].empty())
		{
			int result = facility_id;

			facility_id = relevant_facilities.back();
			relevant_facilities.pop_back();

			return result;
		}

	return -1;
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
		vector<int>& toposort = facility_toposort[item];
		vector<int>& toposort_inv = facility_toposort_inv[item];
		toposort.clear();
		toposort_inv.resize(facilities.size());


		vector<int> relevant_facilities = collect_relevant_facilities(item_t(item));


		// calculate incident / outgoing edges
		vector< unordered_set<int> > incident_edges(facilities.size());
		vector< unordered_set<int> > outgoing_edges(facilities.size());
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
		int root;
		while ((root = pop_root_node(relevant_facilities, incident_edges)) != -1)
		{
			toposort.push_back(root);
			toposort_inv[root] = toposort.size()-1;

			for (int edge_id : outgoing_edges[root])
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

	for (int item = 0; item < MAX_ITEM; item++)
	{
		edge_table_per_item[item].clear();
		edge_table_per_item_inv[item].resize(transport_lines.size());

		for (int i = 0; i < transport_lines.size(); i++)
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
	for (int facility_index : toposort)
	{
		const auto& facility = facilities[facility_index];
		int level = conf.facility_levels[facility_index];
		double production_rate = facility.upgrade_plan[level].production_or_consumption.at(item);

		flowgraph.nodes.emplace_back(production_rate);
	}

	// insert all transport lines that are relevant for `item`
	for (int edge_index = 0; edge_index < edge_table.size(); edge_index++)
	{
		const auto& edge = transport_lines[edge_table[edge_index]];
		assert(edge.item_type == item);
		int level = conf.transport_levels[edge_table[edge_index]];
		double capacity = edge.upgrade_plan[level].capacity;
	
		flowgraph.edges.emplace_back(capacity);
	}

	// fill the nodes' edgetables.
	for (int edge_index = 0; edge_index < edge_table.size(); edge_index++)
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