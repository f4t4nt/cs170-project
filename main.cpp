#include <algorithm>
#include <assert.h>
#include <cmath>
#include <ctime>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <list>
#include <map>
#include <math.h>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stack>
#include <stdio.h>
#include <string>
#include <string.h>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

#include "json.hpp"
using json = nlohmann::json;

using ll = long long;
using ull = unsigned long long;
using ld = long double;
using ch = char;
using str = string;

#define pb push_back
#define elif else if
#define sz(C) (ll) C.size()
#define mp make_pair
#define mt make_tuple
#define flip(C) reverse(C.begin(), C.end())
#define ssort(C) sort(C.begin(), C.end())
#define rsort(C) sort(C.begin(), C.end(), greater<>())
#define all(C) C.begin(), C.end()

#define FOR(x, e) for(ll x = 0; x < (ll) e; x++)
#define FORR(x, e) for(ll x = (ll) e - 1; x >= 0; x--)
#define FOB(x, b, e) for(auto x = b; x < e; x++)
#define FOI(x, e, i) for(ll x = 0; x < (ll) e; x += (ll) i)
#define FOBI(x, b, e, i) for(ll x = (ll) b; x < (ll) e; x += (ll) i)
#define FORE(x, C) for(auto &x : C)

str IN_FILE = "tests/large/random_2.in";
ifstream fin(IN_FILE);

constexpr ll MAX_WEIGHT = 1e3;
constexpr ll MAX_EDGES = 1e4;
constexpr ld K_EXP = 0.5;
constexpr ld K_COEFFICIENT = 1e2;
constexpr ld B_EXP = 70;

constexpr ld INF = 1e18;

struct DSU {
	vector<ll> e;
	DSU(ll N) { e = vector<ll>(N, -1); }
	ll get(int x) { return e[x] < 0 ? x : e[x] = get(e[x]); }
	bool same_set(ll a, ll b) { return get(a) == get(b); }
	ll size(ll x) { return -e[get(x)]; }
	bool unite(ll x, ll y) {
		x = get(x), y = get(y);
		if (x == y) return false;
		if (e[x] > e[y]) swap(x, y);
		e[x] += e[y]; e[y] = x;
		return true;
	}
};

struct Node {
	ll team;
	vector<ll> links;
};

struct Link {
	ll weight;
	ll source, target;
	ll get_other(ll node) {
		if (node == source) return target;
		elif (node == target) return source;
		else return -1;
	}
};

struct Team {
	vector<ll> nodes;
};

struct Graph {
	// given:
	bool directed, multigraph;
    vector<Node> nodes;
	vector<Link> links;
	// calculated:
    ll V, E;
    vector<vector<ll>> weights;
	vector<Team> teams;
};

tuple<ld, ld, ld> score_separated(Graph &G) {
	ld total_nodes_processed = 0;
	FOR (i, G.V) {
		if (G.nodes[i].team != 0) {
			total_nodes_processed++;
		}
	}
	ld k = sz(G.teams) - 1;
	ld b = 0;
	assert(sz(G.teams[0].nodes) == 0);
	FOB (i, 1, sz(G.teams)) {
		ld norm = sz(G.teams[i].nodes) / total_nodes_processed - 1 / k;
		b += norm * norm;
	}
	b = sqrt(b);
	ld C_w = 0;
	FOR (i, sz(G.links)) {
		Link &link = G.links[i];
		if (G.nodes[link.source].team == G.nodes[link.target].team &&
			G.nodes[link.source].team != 0) {
			C_w += link.weight;
		}
	}
	return {C_w, K_COEFFICIENT * exp(K_EXP * k), exp(B_EXP * b)};
}

ld get_score(Graph &G) {
	ld C_w, K, B;
	tie(C_w, K, B) = score_separated(G);
	return C_w + K + B;
}

void read_input(Graph &G) {
	json data = json::parse(fin);
	G.V = sz(data["nodes"]);
	G.directed = data["directed"];
	assert(!G.directed);
	G.multigraph = data["multigraph"];
	assert(!G.multigraph);
	G.nodes = vector<Node>(G.V);
	G.E = sz(data["links"]);
	G.links = vector<Link>(G.E);
	G.weights = vector<vector<ll>>(G.V, vector<ll>(G.V, 0));
	FOR (i, G.E) {
		ll source = data["links"][i]["source"], target = data["links"][i]["target"];
		G.links[i].weight = data["links"][i]["weight"];
		G.links[i].source = source;
		G.links[i].target = target;
		G.nodes[source].links.pb(i);
		G.nodes[target].links.pb(i);
		G.weights[source][target] = G.links[i].weight;
		G.weights[target][source] = G.links[i].weight;
	}
}

str score_to_str(ld score) {
	return to_string((ll) round(score));
}

void write_output(Graph &G) {
	str OUT_FILE = IN_FILE.substr(0, sz(IN_FILE) - 3) + "_" + score_to_str(get_score(G)) + ".out";
	ofstream fout(OUT_FILE);
	fout << "[" << G.nodes[0].team;
	FOB (i, 1, sz(G.nodes)) {
		fout << ", " << G.nodes[i].team;
	}
	fout << "]" << endl;
}

Graph round_table_assignment(Graph &G_in, ll team_count) {
	Graph G = G_in;
	G.teams = vector<Team>(team_count + 1);
	FOR (i, G.V) {
		G.nodes[i].team = i % (sz(G.teams) - 1) + 1;
		G.teams[G.nodes[i].team].nodes.pb(i);
	}
	return G;
}

Graph random_assignment(Graph &G_in, ll team_count) {
	Graph G = G_in;
	G.teams = vector<Team>(team_count + 1);
	FOR (i, G.V) {
		G.nodes[i].team = rand() % (sz(G.teams) - 1) + 1;
		G.teams[G.nodes[i].team].nodes.pb(i);
	}
	return G;
}

Graph greedy(Graph &G_in) {
	Graph G = G_in;
	vector<ll> node_order(G.V);
	FOR (i, G.V) {
		node_order[i] = i;
	}
	shuffle(all(node_order), default_random_engine());
	G.teams = vector<Team>(2);
	G.nodes[node_order[0]].team = 1;
	G.teams[1].nodes.pb(node_order[0]);
	FOB (i, 1, G.V) {
		ll node = node_order[i];
		ll best_team = -1;
		ld best_score = INF;
		FOB (team, 1, sz(G.teams)) {
			G.nodes[node].team = team;
			G.teams[team].nodes.pb(node);
			ld score = get_score(G);
			if (score < best_score) {
				best_score = score;
				best_team = team;
			}
			G.teams[team].nodes.pop_back();
		}
		Graph G_copy = G;
		G_copy.teams.pb(Team());
		G_copy.nodes[node].team = sz(G_copy.teams) - 1;
		G_copy.teams[sz(G_copy.teams) - 1].nodes.pb(node);
		ld score = get_score(G_copy);
		if (score < best_score) {
			G = G_copy;
		} else {
			G.nodes[node].team = best_team;
			G.teams[best_team].nodes.pb(node);
		}
	}
	return G;
}

struct simulated_annealing_agent {
	Graph G;
	ld T;
	void init(Graph &G_in) {
		G = G_in;
		T = 1;
	}
	void step() {
		ll node = rand() % G.V;
		ll current_team = G.nodes[node].team;
		ll team = rand() % (sz(G.teams) - 1) + 1;
		ld score = get_score(G);
		G.nodes[node].team = team;
		G.teams[team].nodes.pb(node);
		G.teams[current_team].nodes.erase(find(all(G.teams[current_team].nodes), node));
		ld new_score = get_score(G);
		if (new_score < score) {
			return;
		}
		ld p = exp((score - new_score) / T);
		if (rand() % 1000000 < p * 1000000) {
			return;
		}
		G.nodes[node].team = current_team;
		G.teams[team].nodes.pop_back();
		G.teams[current_team].nodes.pb(node);
	}
};

Graph simulated_annealing(Graph &G_in) {
	simulated_annealing_agent agent;
	agent.init(G_in);
	while (agent.T > 0.0001) {
		FOR (i, 1000) {
			agent.step();
		}
		agent.T *= 0.99;
	}
	return agent.G;
}

int main() {
    Graph G;
    read_input(G);
	G = random_assignment(G, 9);
	G = simulated_annealing(G);
	write_output(G);
	return 0;
}