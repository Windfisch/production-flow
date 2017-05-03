#pragma once
#include <string>
#include <vector>

struct FlowGraph
{
	// dependent data structures

	struct Edge
	{
		Edge(int cap) : capacity(cap), actual_capacity(cap) {}

		int capacity;
		int actual_capacity;
		int actual_flow = 0;

		std::string label() const;
	};

	struct Node
	{
		Node(int max_prod) : max_production(max_prod) {}

		int max_production; // negative production = consumption, zero = splitter
		int actual_production = 0;
		int excess = 0;

		std::vector<Edge*> incoming_edges;
		std::vector<Edge*> outgoing_edges; // max capacity on outgoing = splitter speed.

		int incoming() const;
		int available() const; // amount available for pushing out
		void update_forward();
		void update_backward();

		std::string label() const;
	};


	// members

	std::vector<Node> nodes;
	std::vector<Edge> edges;


	// methods

	void build();
	void calculate();
	void dump(std::string name) const;
	bool is_valid() const;
	size_t edge_from(const Edge* edge) const;
	size_t edge_to(const Edge* edge) const;
};

