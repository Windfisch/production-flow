#include <vector>
#include <cassert>
#include <memory>
#include <iostream>
#include <boost/heap/binomial_heap.hpp>
#include "actiongraph.hpp"

using namespace std;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
bool ActionGraph::Node::equals(const ActionGraph::Node& other, const Factory* factory) const
#pragma GCC diagnostic pop
{
	if (current_item_type != other.current_item_type)
		return false;

	// all facilities, that can still be found in one of the flowgraphs
	// which we will consider, must match.
	assert(conf.facility_levels.size() == factory->facilities.size());
	assert(other.conf.facility_levels.size() == factory->facilities.size());
	for (size_t i=0; i<conf.facility_levels.size(); i++)
	{
		if (factory->facilities[i].most_advanced_item_involved > current_item_type)
			continue;
		else
			if (conf.facility_levels[i] != other.conf.facility_levels[i])
				return false;
	}

	// all edges, that can still be found in one of the flowgraphs
	// which we will consider, must match.
	assert(conf.transport_levels.size() == factory->transport_lines.size());
	assert(other.conf.transport_levels.size() == factory->transport_lines.size());
	for (size_t i=0; i<conf.transport_levels.size(); i++)
	{
		if (factory->transport_lines[i].item_type > current_item_type)
			continue;
		else
			if (conf.transport_levels[i] != other.conf.transport_levels[i])
				return false;
	}

	return true;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
vector< unique_ptr<ActionGraph::Node> > ActionGraph::Node::successors(const Factory* factory) const
#pragma GCC diagnostic pop
{
	vector< unique_ptr<ActionGraph::Node> > result;

	#ifndef NDEBUG
	// all flowgraphs for items that are more advanced than the current_item_type
	// are valid (i.e., no bottlenecks). This is guaranteed by design.

	for (int type = current_item_type+1; type < item_t::MAX_ITEM; type++)
	{
		cout << "checking assertion for itemtype " << type << endl;
		FlowGraph flow = factory->build_flowgraph(item_t(type), conf);
		flow.calculate();
		assert(flow.is_valid());
	}
	cout << "done with assertion-checking" << endl;
	#endif
	
	cout << "simulating flow for current item type " << current_item_type << endl;
	// construct and simulate flow graph for current_item_type
	FlowGraph flow = factory->build_flowgraph(current_item_type, conf);
	flow.calculate();

	if (flow.is_valid())
	{
		auto nodeptr = make_unique<ActionGraph::Node>(*this);
		nodeptr->current_item_type = item_t(current_item_type-1); // if this reaches '-1', then we're done.
		nodeptr->total_cost += 0.;
		result.emplace_back(move(nodeptr));
	}
	else
	{
		// upgradeable nodes
		const auto& toposort = factory->facility_toposort.at(current_item_type);
		assert(toposort.size() == flow.nodes.size());
		for (size_t i=0; i<flow.nodes.size(); i++)
		{
			const auto& node = flow.nodes[i];
			const size_t facility_idx = toposort[i];
			const auto& facility = factory->facilities[facility_idx];

			if ( (node.max_production >= 0 && node.actual_production >= node.max_production) && // a producing node is at max capacity
				conf.facility_levels[facility_idx]+1 < facility.upgrade_plan.size() ) // and we can actually upgrade the node
			{
				auto nodeptr = make_unique<ActionGraph::Node>(*this);
				nodeptr->conf.facility_levels[facility_idx]++;
				nodeptr->total_cost += facility.upgrade_plan[ conf.facility_levels[facility_idx] ].incremental_cost;
				result.emplace_back(move(nodeptr));
			}
		}
		
		// upgradeable edges
		const auto& edgetable = factory->edge_table_per_item.at(current_item_type);
		assert(edgetable.size() == flow.edges.size());
		for (size_t i=0; i<flow.edges.size(); i++)
		{
			const auto& edge = flow.edges[i];
			const size_t transport_line_idx = edgetable[i];
			const auto& transport_line = factory->transport_lines[transport_line_idx];

			if ( (edge.actual_flow >= edge.capacity) && // an edge is at max capacity
				conf.transport_levels[transport_line_idx]+1 < transport_line.upgrade_plan.size() ) // and we can actually upgrade the edge
			{
				auto nodeptr = make_unique<ActionGraph::Node>(*this);
				nodeptr->conf.transport_levels[transport_line_idx]++;
				nodeptr->total_cost += transport_line.upgrade_plan[ conf.transport_levels[transport_line_idx] ].incremental_cost;
				result.emplace_back(move(nodeptr));
			}
		}
	}

	return result;
}


struct node_comparator
{
	bool operator() (const ActionGraph::Node& a, const ActionGraph::Node& b) const
	{
		return a.total_cost < b.total_cost;
	}
};

pair<Factory::FactoryConfiguration, double> ActionGraph::dijkstra(Factory::FactoryConfiguration initial_config)
{
	auto start_node = make_unique<ActionGraph::Node>();
	start_node->conf = initial_config;
	start_node->current_item_type = item_t(MAX_ITEM-1);
	start_node->total_cost = 0.;

	vector< unique_ptr< ActionGraph::Node> > openlist;
	vector< unique_ptr< ActionGraph::Node> > closedlist;
	openlist.push_back(move(start_node));

	while (!openlist.empty())
	{
		cout << "openlist has size " << openlist.size() << ", total expanded = " << openlist.size() + closedlist.size() << endl;

		// find and remove smallest element
		auto smallest = openlist.begin();
		for (auto it = openlist.begin(); it != openlist.end(); it++)
			if ((*it)->total_cost < (*smallest)->total_cost)
				smallest=it;
		
		closedlist.push_back(move(*smallest));
		auto& nodeptr = closedlist.back();
		*smallest = move(openlist.back());
		openlist.pop_back();

		cout << "inspecting item: " << nodeptr->current_item_type << ", ";
		cout << "nodes:";
		for (auto lvl : nodeptr->conf.facility_levels)
			cout << " " << lvl;
		cout << ", edges:";
		for (auto lvl : nodeptr->conf.transport_levels)
			cout << " " << lvl;
		cout << endl;


		// expand node
		auto successor_nodes = nodeptr->successors(factory);
		for (auto& successor : successor_nodes)
		{
			cout << "  -> successor item: " << successor->current_item_type << ", ";
			cout << "nodes:";
			for (auto lvl : successor->conf.facility_levels)
				cout << " " << lvl;
			cout << ", edges:";
			for (auto lvl : successor->conf.transport_levels)
				cout << " " << lvl;
			if (successor->current_item_type == DONE)
			{
				// we've found a goal state! :)
				cout << endl << "success, cost = " << successor->total_cost << ", expanded " << closedlist.size() + openlist.size() << " nodes" << endl;
				return pair<Factory::FactoryConfiguration, double>(successor->conf, successor->total_cost);
			}

			bool found = false;
			for (auto& openlist_node : openlist)
				if (openlist_node->equals(*successor, factory))
				{
					openlist_node = move(successor);
					cout << "; already in openlist" << endl;
					found = true;
					break;
				}
			if (!found)
				for (auto& closedlist_node : closedlist)
					if (closedlist_node->equals(*successor, factory))
					{
						found = true;
						cout << "; already in closedlist" << endl;
						break;
					}

			if (!found)
			{
				openlist.push_back(move(successor));
				cout << "; not seen yet, adding to openlist" << endl;
			}
		}
	}

	cout << "could not find a solution :(" << endl;
	return pair<Factory::FactoryConfiguration, double>(initial_config, -1.);
}
