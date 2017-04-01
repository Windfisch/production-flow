#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <assert.h>

using namespace std;


template <typename T>
std::string str(const T a_value, const int n = 3)
{
    std::ostringstream out;
    out << std::setprecision(n) << a_value;
    return out.str();
}

struct Edge
{
	Edge(int from_, int to_, double cap) : from(from_), to(to_), capacity(cap), actual_capacity(cap) {}
	int from;
	int to;
	double capacity;
	double actual_capacity;
	double actual_flow = 0.;

	string label() const
	{
		string color;
		if (actual_flow > actual_capacity)
			color = "color=red,";
		string label = "label=\""s + str(actual_flow) + "/"s + str(actual_capacity) + "("s+str(capacity)+")\""s;
		return color+label;
	}
};

struct Node
{
	Node(double max_prod) : max_production(max_prod) {}

	double max_production; // negative production = consumption, zero = splitter
	double actual_production = 0.;
	double excess = 0.;

	vector<Edge*> incoming_edges;
	vector<Edge*> outgoing_edges; // max capacity on outgoing = splitter speed.

	string label() const
	{
		string color;
		if (incoming() < -max_production)
			color = "color=red,";
		else if (excess > 0)
			color = "color=blue,";
		string label("label=\""s + str(incoming())+"in, "s + str(actual_production)+"/"s+str(max_production)+"prod\\n"s + str(available()) + "avail, "s +  str(excess)+"exc\""s);
		return color+label;
	}

	double incoming() const
	{
		double result = 0.;

		for (Edge* edge : incoming_edges)
			result += edge->actual_flow;

		return result;
	}

	double available() const // amount available for pushing out
	{
		return max(0., incoming() + actual_production);
	}

	void update_forward()
	{
		if (max_production > 0.)
			actual_production = max_production;
		else
			actual_production = -min(incoming(), -max_production); // never consume more than incoming
	}

	void update_backward()
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

	void shove_out(double amount) // try to output. will update outgoing_edges.*->actual_flow and this->excess
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

};

struct Graph
{
	vector<Node> nodes;
	vector<Edge> edges;

	void build()
	{
		for (auto& edge : edges)
		{
			nodes[edge.from].outgoing_edges.push_back(&edge);
			nodes[edge.to].incoming_edges.push_back(&edge);
		}
	}

	void dump(string name)
	{
		cout << "digraph \""<<name<<"\" {" << endl;

		for (size_t i=0; i<nodes.size(); i++)
			cout << "\t" << i << " [" << nodes[i].label() << "];" << endl;

		cout << endl;

		for (Edge& edge : edges)
			cout << "\t" << edge.from << " -> " << edge.to << " [" << edge.label() << "];" << endl;

		cout << "}" << endl;
	}
};

int main()
{
	Graph graph;

	graph.nodes.emplace_back(10.);
	graph.nodes.emplace_back(20.);
	graph.nodes.emplace_back(-1.);
	graph.nodes.emplace_back(-24);
	graph.nodes.emplace_back(-3);

	graph.edges.emplace_back(0,2, 50.);
	graph.edges.emplace_back(0,3, 50.);
	graph.edges.emplace_back(1,3, 50.);
	graph.edges.emplace_back(3,4, 50.);

	graph.build();

	graph.dump("initial");

	for (int i=0; i<5; i++)
	{
		for (auto& node : graph.nodes)
		{
			node.update_forward();
			node.shove_out(node.available());
		}

		graph.dump("it"+to_string(i));

		for (auto& node : graph.nodes)
			node.update_backward();

		graph.dump("it"+to_string(i)+".5");
	}
}
