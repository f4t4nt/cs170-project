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

constexpr ld INF = 1e30;

str IN_FILE;
str RUN_TYPE;
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

struct Result {
	str size;
	ll id;
	ll rank;
	ld score;
	str path;
	str url;
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

void set_io(str file, str run_type = "") {
	IN_FILE = file;
	RUN_TYPE = run_type;
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

void read_teams(Graph &G, str file) {
	str TEAM_FILE = IN_FILE + file + ".out";
	ifstream fin(TEAM_FILE);
	str text;
	getline(fin, text);
	json data = json::parse(text);
	ll num_teams = 0;
	FOR (i, sz(data)) {
		num_teams = max(num_teams, (ll) data[i]);
	}
	G.teams = vector<Team>(num_teams + 1);
	FOR (i, sz(data)) {
		G.nodes[i].team = data[i];
		G.teams[data[i]].nodes.pb(i);
	}
}

void read_graph(Graph &G, str test_sz, ll test_id, str run_type = "") {
	set_io("tests/" + test_sz + "/" + test_sz + to_string(test_id) + "/", run_type);
	read_input(G);
}

void read_best_graph(Graph &G, str test_sz, ll test_id, str run_type = "") {
	read_graph(G, test_sz, test_id, run_type);
	str best_team_file_name = "";
	for (const auto & entry : filesystem::directory_iterator("tests/" + test_sz + "/" + test_sz + to_string(test_id) + "/")) {
		filesystem::path team_file;
		team_file = entry.path();
		if (team_file.extension() != ".out") {
			continue;
		}
		str team_file_name = team_file.filename().string();
		team_file_name = team_file_name.substr(0, team_file_name.find("."));
		if (best_team_file_name == "" ||
			stoll(team_file_name.substr(0, team_file_name.find("_"))) <
			stoll(best_team_file_name.substr(0, best_team_file_name.find("_")))) {
			best_team_file_name = team_file_name;
		}
	}
	read_teams(G, best_team_file_name);
}

vector<Result> read_queue() {
	vector<Result> results;
	ifstream fin("queue.txt");
	str line;
	while (getline(fin, line)) {
		stringstream ss(line);
		str size, id, rank, score, url;
		ss >> size >> id >> rank >> score >> url;
		results.pb({size, stoll(id), stoll(rank), stold(score), url, "tests/" + size + "/" + size + id + "/"});
	}
	return results;
}

str score_to_str(ld score) {
	return to_string((ll) round(score));
}

void write_output(Graph &G) {
	str OUT_FILE;
	if (RUN_TYPE == "") {
		OUT_FILE = IN_FILE.substr(0, sz(IN_FILE) - 1) + "/" + score_to_str(get_score(G)) + ".out";
	} else {
		OUT_FILE = IN_FILE.substr(0, sz(IN_FILE) - 1) + "/" + score_to_str(get_score(G)) + "_" + RUN_TYPE + ".out";
	}	
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