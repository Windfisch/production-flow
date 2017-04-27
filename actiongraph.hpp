#pragma once

#include <vector>
#include <memory>
#include <utility>
#include "factory.hpp"

struct ActionGraph
{
	ActionGraph(const Factory* factory_) : factory(factory_) {}

	struct Node
	{
		Factory::FactoryConfiguration conf;
		item_t current_item_type;
		double total_cost;

		bool equals(const ActionGraph::Node& other, const Factory* factory) const;
		std::vector< std::unique_ptr<Node> > successors(const Factory* factory) const;
	};

	const Factory* factory;

	// finds the cheapest upgraded configuration that satisfies all demands.
	// pair.first will contain the configuration, and pair.second the cost.
	// if pair.second is negative, this signifies that no solution could be found.
	std::pair<Factory::FactoryConfiguration, double> dijkstra(Factory::FactoryConfiguration initial_config);
};
