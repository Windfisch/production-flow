#pragma once

#include <vector>
#include <memory>
#include "factory.hpp"

struct ActionGraph
{
	ActionGraph(const Factory* factory_) : factory(factory_) {}

	struct Edge
	{
		enum { NONE, UPGRADE_FACILITY, UPGRADE_TRANSPORT_LINE, DECREMENT_CURRENT_ITEM_TYPE } type = NONE;
		size_t index = 0; // of the upgraded facility or transport_line
		int diff = 0;
	};

	struct Node
	{
		Factory::FactoryConfiguration conf;
		item_t current_item_type;
		Edge coming_from;
		double total_cost;

		bool equals(const ActionGraph::Node& other, const Factory* factory) const;
		std::vector< std::unique_ptr<Node> > successors(const Factory* factory) const;
	};

	const Factory* factory;

	void dijkstra(Factory::FactoryConfiguration initial_config);
};
