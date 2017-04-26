#include <vector>
#include <cassert>
#include "actiongraph.hpp"

using namespace std;

bool ActionGraph::Node::equals(const ActionGraph::Node& other, const Factory* factory) const
{
	if (current_item_type != other.current_item_type)
		return false;

	// all facilities, that can still be found in one of the flowgraphs
	// which we will consider, must match.
	assert(conf.facility_levels.size() == factory->facilities.size());
	assert(other.conf.facility_levels.size() == factory->facilities.size());
	for (size_t i=0; i<conf.facility_levels.size(); i++)
	{
		if (factory->facilities[i].most_basic_item_involved > current_item_type)
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

vector<ActionGraph::Node> ActionGraph::Node::successors(const Factory* factory) const
{
	vector<ActionGraph::Node> result;

	#ifndef NDEBUG
	// all flowgraphs for items that are more advanced than the current_item_type
	// are valid (i.e., no bottlenecks). This is guaranteed by design.

	for (int type = current_item_type+1; type < item_t::MAX_ITEM; type++)
	{
		FlowGraph flow = factory->build_flowgraph(item_t(type), conf);
		flow.calculate();
		assert(flow.is_valid());
	}
	#endif
	
	// construct and simulate flow graph for current_item_type
	FlowGraph flow = factory->build_flowgraph(current_item_type, conf);
	flow.calculate();

	if (flow.is_valid())
	{
		if (current_item_type > 0)
		{
			result.emplace_back(*this);
			result.back().current_item_type = item_t(current_item_type-1);
		}
		else
		{
			// we're in a goal state :)
			// FIXME: hm. not sure. how to differentiate between this and "dead end"?
		}
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

			if ( (node.max_production > 0. && node.actual_production == node.max_production) && // a producing node is at max capacity
				conf.facility_levels[facility_idx] < facility.upgrade_plan.size() ) // and we can actually upgrade the node
			{
				result.emplace_back(*this);
				result.back().conf.facility_levels[facility_idx]++;
				// TODO: cost at facility->upgrade_plan[ conf.facility_levels[facility_idx] ]
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

			if ( (edge.actual_flow == edge.capacity) && // an edge is at max capacity
				conf.transport_levels[transport_line_idx] < transport_line.upgrade_plan.size() ) // and we can actually upgrade the edge
			{
				result.emplace_back(*this);
				result.back().conf.transport_levels[transport_line_idx]++;
				// TODO: cost at transport_line->upgrade_plan[ conf.transport_levels[transport_line_idx] ]
			}
		}
	}

	return result;
}

/*
void ActionGraph::dijkstra(Factory::FactoryConfiguration initial_config)
{
	struct OpenlistEntry
	{
		Node node;
		double f;

		OpenlistEntry(Node&& node_, double f_) : node(node_), f(f_) {}
	};

	typedef boost::heap::binomial_heap<OpenlistEntry> openlist_t;
	typedef openlist::handle_type openlist_handle_t;

	openlist_t openlist;
	unordered_map<Node, openlist_handle_t>
}*/
