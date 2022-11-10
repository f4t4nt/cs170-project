#include <algorithm>
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

#define FOR(x, e) for(ll x = 0; x < (ll) e; x++)
#define FORR(x, e) for(ll x = (ll) e - 1; x >= 0; x--)
#define FOB(x, b, e) for(auto x = b; x < e; x++)
#define FOI(x, e, i) for(ll x = 0; x < (ll) e; x += (ll) i)
#define FOBI(x, b, e, i) for(ll x = (ll) b; x < (ll) e; x += (ll) i)
#define FORE(x, C) for(auto &x : C)

str IN_FILE = "tests/small/random_1.in";
ifstream fin(IN_FILE);

constexpr ll MAX_WEIGHT = 1e3;
constexpr ll MAX_EDGES = 1e4;
constexpr ld K_EXP = 0.5;
constexpr ll K_COEFFICIENT = 1e2;
constexpr ll B_EXP = 70;

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

struct Node;
struct Link;
struct Team;
struct Graph;

struct Node {
    ll id;
	Team* team;
	vector<Link*> links;
};

struct Link {
	ll weight;
	Node* source, * target;
	Node* get_other(Node* node) {
		if (node == source) return target;
		elif (node == target) return source;
		else return nullptr;
	}
};

struct Team {
	ll id;
	vector<Node*> nodes;
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
	ld k = sz(G.teams);
	ld b = 0;
	FOR (i, sz(G.teams)) {
		ld count = sz(G.teams[i].nodes);
		ld norm = count / G.V - 1 / k;
		b += norm * norm;
	}
	b = sqrt(b);
	ld C_w = 0;
	FOR (i, sz(G.links)) {
		Link &link = G.links[i];
		if (link.source->team == link.target->team) C_w += link.weight;
	}
	return {C_w, K_COEFFICIENT * exp(K_EXP * k), exp(B_EXP * b)};
}

ld score(Graph &G) {
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
	FOR (i, G.V) {
		G.nodes[i].id = data["nodes"][i]["id"];
	}
	G.E = sz(data["links"]);
	G.links = vector<Link>(G.E);
	G.weights = vector<vector<ll>>(G.V, vector<ll>(G.V, 0));
	FOR (i, G.E) {
		ll source = data["links"][i]["source"], target = data["links"][i]["target"];
		G.links[i].weight = data["links"][i]["weight"];
		G.links[i].source = &G.nodes[source];
		G.links[i].target = &G.nodes[target];
		G.nodes[source].links.pb(&G.links[i]);
		G.nodes[target].links.pb(&G.links[i]);
		G.weights[source][target] = G.weights[target][source] = data["links"][i]["weight"];
	}
}

str score_to_str(ld score) {
	if (score < 1000) return to_string((ll) score);
	elif (score < 1e6) return to_string((ll) (score / 1e3)) + "k";
	elif (score < 1e9) return to_string((ll) (score / 1e6)) + "M";
	else return to_string((ll) (score / 1e9)) + "B";
}

void write_output(Graph &G) {
	str OUT_FILE = IN_FILE.substr(0, sz(IN_FILE) - 3) + "_" + score_to_str(score(G)) + ".out";
	ofstream fout(OUT_FILE);
	fout << "[" << G.nodes[0].team->id;
	FOB (i, 1, sz(G.nodes)) {
		fout << ", " << G.nodes[i].team->id;
	}
	fout << "]" << endl;
}

int main() {
    Graph G;
    read_input(G);
	G.teams = vector<Team>(10);
	FOR (i, 10) {
		G.teams[i].id = i + 1;
	}
	FOR (i, G.V) {
		G.nodes[i].team = &G.teams[rand() % sz(G.teams)];
		G.nodes[i].team->nodes.pb(&G.nodes[i]);
	}
	write_output(G);
    return 0;
}