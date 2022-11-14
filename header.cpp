  //////////////////////////////////////////
 // helper data structures and functions //
//////////////////////////////////////////

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

#include "json.hpp"

using namespace std;

using ll = long long;
using ull = unsigned long long;
using ld = long double;
using ch = char;
using str = string;
using json = nlohmann::json;

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

constexpr ll MAX_WEIGHT = 1e3;
constexpr ll MAX_EDGES = 1e4;
constexpr ld K_EXP = 0.5;
constexpr ld K_COEFFICIENT = 1e2;
constexpr ld B_EXP = 70;

constexpr ld INF = 1e18;

str IN_FILE;
ifstream fin;

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

void set_io(str file) {
	IN_FILE = file;
	fin = ifstream(IN_FILE + "graph.in");
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
	str OUT_FILE = IN_FILE.substr(0, sz(IN_FILE) - 1) + "/" + score_to_str(get_score(G)) + ".out";
	ofstream fout(OUT_FILE);
	fout << "[" << G.nodes[0].team;
	FOB (i, 1, sz(G.nodes)) {
		fout << ", " << G.nodes[i].team;
	}
	fout << "]" << endl;
}

ll weighted_random(vector<ld> &weights, ll range = 0) {
	ld total = 0;
	FOB (i, range, sz(weights)) {
		total += weights[i];
	}
	ld r = (ld) rand() / RAND_MAX * total;
	ld sum = 0;
	FOB (i, range, sz(weights)) {
		sum += weights[i];
		if (sum >= r) {
			return i;
		}
	}
	assert(false);
	return -1;
}