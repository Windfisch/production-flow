#pragma once
#include <string>
#include <vector>

struct FlowGraph
{
	// dependent data structures

	struct Edge
	{
		Edge(double cap) : capacity(cap), actual_capacity(cap) {}

		double capacity;
		double actual_capacity;
		double actual_flow = 0.;

		std::string label() const;
	};

	struct Node
	{
		Node(double max_prod) : max_production(max_prod) {}

		double max_production; // negative production = consumption, zero = splitter
		double actual_production = 0.;
		double excess = 0.;

		std::vector<Edge*> incoming_edges;
		std::vector<Edge*> outgoing_edges; // max capacity on outgoing = splitter speed.

		double incoming() const;
		double available() const; // amount available for pushing out
		void update_forward();
		void update_backward();
		void shove_out(double amount); // try to output. will update outgoing_edges.*->actual_flow and this->excess

		std::string label() const;
	};


	// members

	std::vector<Node> nodes;
	std::vector<Edge> edges;


	// methods

	void build();
	void calculate();
	void dump(std::string name) const;
	int edge_from(const Edge* edge) const;
	int edge_to(const Edge* edge) const;
};
