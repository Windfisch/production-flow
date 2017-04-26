#pragma once

#include <vector>
#include "factory.hpp"

struct ActionGraph
{
	ActionGraph(const Factory* factory) {}

	struct Edge
	{
		enum { NONE, UPGRADE_FACILITY, UPGRADE_TRANSPORT_LINE, DECREMENT_CURRENT_ITEM_TYPE } type = NONE;
		int index = 0; // of the upgraded facility or transport_line
		int diff = 0;
	};

	struct Node
	{
		Factory::FactoryConfiguration conf;
		item_t current_item_type;
		Edge coming_from;

		bool equals(const ActionGraph::Node& other, const Factory* factory) const;
		std::vector<Node> successors(const Factory* factory) const;
	};

	void dijkstra(Factory::FactoryConfiguration initial_config);
};
