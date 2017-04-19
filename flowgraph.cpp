#include <string>
#include <vector>
#include <map>
#include <cassert>

#include <sstream>
#include <iomanip>
#include <iostream>

#include "flowgraph.hpp"

using namespace std;

template <typename T>
std::string str(const T a_value, const int n = 3)
{
	std::ostringstream out;
	out << std::setprecision(n) << a_value;
	return out.str();
}

double FlowGraph::Node::incoming() const
{
	double result = 0.;

	for (Edge* edge : incoming_edges)
		result += edge->actual_flow;

	return result;
}

double FlowGraph::Node::available() const // amount available for pushing out
{
	return max(0., incoming() + actual_production);
}

void FlowGraph::Node::update_forward()
{
	if (max_production > 0.)
		actual_production = max_production;
	else
		actual_production = -min(incoming(), -max_production); // never consume more than incoming
}

void FlowGraph::Node::shove_out(double amount) // try to output. will update outgoing_edges.*->actual_flow and this->excess
{
	multimap<double, Edge*> sorted_edges;
	for (Edge* edge : outgoing_edges)
		sorted_edges.insert( std::pair<double, Edge*>(edge->actual_capacity, edge) );

	size_t edges_remaining = sorted_edges.size();
	double amount_remaining = amount;

	for (auto& it : sorted_edges)
	{
		Edge* edge = it.second;
		auto capacity = it.first;

		double fair_share = amount_remaining / edges_remaining;
		if (fair_share < capacity)
			edge->actual_flow = fair_share;
		else
		{
			edge->actual_flow = capacity; // set edge flow to maximum possible value
			amount_remaining -= capacity; // the remaining amount must now be shared over even less edges
			edges_remaining--;
		}

		assert(amount_remaining >= 0.);
	}

	if (edges_remaining == 0)
	{
		excess = amount_remaining;
		if (actual_production > 0.)
		{
			double reduction = min(excess, actual_production);
			actual_production -= reduction;
			excess -= reduction;
		}
	}
	else
		excess = 0.;
}

void FlowGraph::Node::update_backward()
{
	if (excess <= 0.)
		return;

	double amount = incoming() - excess;

	multimap<double, Edge*> sorted_edges;
	for (Edge* edge : incoming_edges)
		sorted_edges.insert( std::pair<double, Edge*>(edge->actual_flow, edge) );

	size_t edges_remaining = sorted_edges.size();
	double amount_remaining = amount;

	for (auto& it : sorted_edges)
	{
		Edge* edge = it.second;
		auto capacity = it.first;

		double fair_share = amount_remaining / edges_remaining;
		if (fair_share < capacity)
			edge->actual_capacity = fair_share;
		else
		{
			edge->actual_capacity = capacity; // set edge flow to maximum possible value
			amount_remaining -= capacity; // the remaining amount must now be shared over even less edges
			edges_remaining--;
		}

		assert(amount_remaining >= 0.);
	}

	assert(edges_remaining > 0);
}

void FlowGraph::calculate()
{
	for (int i=0; i<20; i++) // TODO: automatically stop upon convergence, not after 20 iterations
	{
		for (auto& node : nodes)
		{
			node.update_forward();
			node.shove_out(node.available());
		}

		dump("it"+to_string(i));

		for (auto& node : nodes)
			node.update_backward();

		dump("it"+to_string(i)+".5");
	}
}




// printing functions

string FlowGraph::Edge::label() const
{
	string color;
	if (actual_flow > actual_capacity)
		color = "color=red,";
	string label = "label=\""s + str(actual_flow) + "/"s + str(actual_capacity) + "("s+str(capacity)+")\""s;
	return color+label;
}

string FlowGraph::Node::label() const
{
	string color;
	if (incoming() < -max_production)
		color = "color=red,";
	else if (excess > 0)
		color = "color=blue,";
	string label("label=\""s + str(incoming())+"in, "s + str(actual_production)+"/"s+str(max_production)+"prod\\n"s + str(available()) + "avail, "s +  str(excess)+"exc\""s);
	return color+label;
}

void FlowGraph::dump(string name) const
{
	cout << "digraph \""<<name<<"\" {" << endl;

	for (size_t i=0; i<nodes.size(); i++)
		cout << "\t" << i << " [" << nodes[i].label() << "];" << endl;

	cout << endl;

	for (const Edge& edge : edges)
		cout << "\t" << edge_from(&edge) << " -> " << edge_to(&edge) << " [" << edge.label() << "];" << endl;

	cout << "}" << endl;
}

size_t FlowGraph::edge_from(const Edge* edge) const
{
	for (size_t i=0; i<nodes.size(); i++)
	{
		for (const Edge* e : nodes[i].outgoing_edges)
			if (e == edge)
				return i;
	}

	throw runtime_error("FlowGraph is corrupt");
}

size_t FlowGraph::edge_to(const Edge* edge) const
{
	for (size_t i=0; i<nodes.size(); i++)
	{
		for (const Edge* e : nodes[i].incoming_edges)
			if (e == edge)
				return i;
	}

	throw runtime_error("FlowGraph is corrupt");
}
