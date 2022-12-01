#include <algorithm>
#include <assert.h>
#include <cmath>
#include <ctime>
#include <deque>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <math.h>
#include <memory>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stack>
#include <stdio.h>
#include <string.h>
#include <string>
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
#define all(C) C.begin(), C.end()
#define flip(C) reverse(all(C))
#define ssort(C) sort(all(C))
#define rsort(C) sort(all(C), greater<>())
#define sshuffle(C) shuffle(all(C), default_random_engine(rand()))

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
	vector<ll> get_set(ll x) {
		vector<ll> res;
		FOR(i, e.size()) if (get(i) == get(x)) res.pb(i);
		return res;
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

struct OptimizedGraphInvariant {
	short V, E, T;
	vector<short> edge_indices;
	vector<tuple<short, short, short>> edges;
	vector<vector<short>> weights;
	OptimizedGraphInvariant(short V,
		short E,
		short T,
		vector<short> edge_indices,
		vector<tuple<short, short, short>> edges,
		vector<vector<short>> weights) :
		V(V), E(E), T(T), edge_indices(edge_indices), edges(edges), weights(weights) {}
	shared_ptr<const OptimizedGraphInvariant> change_T(short new_T)const {
		return make_shared<const OptimizedGraphInvariant>(OptimizedGraphInvariant(V, E, new_T, edge_indices, edges, weights));
	}
};

struct OptimizedGraph {
	ld score;
	shared_ptr<const OptimizedGraphInvariant> invariant;
	ld C_w, K, B_norm_squared;
	vector<ld> B_vec;
	vector<ch> node_teams;
	vector<short> team_sizes;
};

struct Result {
	str size;
	ll id;
	ll rank;
	ld best_score;
	ld submission_score;
	ld local_score;
	ld delta_score;
	str local_file;
	str url;
	str notes;
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

ld optimized_get_score(OptimizedGraph &G) {
	G.C_w = 0;
	FORE (edge, G.invariant->edges) {
		short source, target, weight;
		tie(source, target, weight) = edge;
		if (G.node_teams[source] == G.node_teams[target]) {
			G.C_w += weight;
		}
	}
	G.C_w /= 2;
	G.B_norm_squared = 0;
	FOR (i, G.invariant->T) {
		G.B_vec[i] = G.team_sizes[i] / (ld) G.invariant->V - 1.0 / G.invariant->T;
		G.B_norm_squared += G.B_vec[i] * G.B_vec[i];
	}
	G.score = G.C_w + G.K + exp(B_EXP * sqrt(G.B_norm_squared));
	return G.score;
}

tuple<ld, ld, ld, ld, ld> optimized_update_score(OptimizedGraph &G, short node, ch old_team, ch new_team) {
	ld B_old = G.B_vec[old_team] - 1.0 / G.invariant->V,
		B_new = G.B_vec[new_team] + 1.0 / G.invariant->V;
	ld B_norm_squared = G.B_norm_squared -
		G.B_vec[old_team] * G.B_vec[old_team] -
		G.B_vec[new_team] * G.B_vec[new_team] +
		B_old * B_old +
		B_new * B_new;
	ld C_w = G.C_w;
	short i = G.invariant->edge_indices[node], end = (node == G.invariant->V - 1 ? 2 * G.invariant->E : G.invariant->edge_indices[node + 1]);
	auto &edges = G.invariant->edges;
	while (i < end) {
		short target, weight;
		tie(ignore, target, weight) = edges[i];
		auto target_team = G.node_teams[target];
		if (target_team == old_team) {
			C_w -= weight;
		} elif (target_team == new_team) {
			C_w += weight;
		}
		i++;
	}
	return {C_w, exp(B_EXP * sqrt(B_norm_squared)), B_norm_squared, B_old, B_new};
}

ld optimized_update_score_batch_swaps(OptimizedGraph &G, vector<short> &nodes, vector<ch> &old_teams, vector<ch> &new_teams) {
	ld C_w = G.C_w;
	auto &edges = G.invariant->edges;
	FOR (j, sz(nodes)) {
		ch old_team = old_teams[j], new_team = new_teams[j];
		if (old_team != new_team) {
			short node = nodes[j];
			short i = G.invariant->edge_indices[node], end = (node == G.invariant->V - 1 ? 2 * G.invariant->E : G.invariant->edge_indices[node + 1]);
			while (i < end) {
				short target, weight;
				tie(ignore, target, weight) = edges[i];
				auto target_team = G.node_teams[target];
				if (target_team == old_team) {
					C_w -= weight;
				} elif (target_team == new_team) {
					C_w += weight;
				}
				i++;
			}
		}
	}
	return C_w;
}

ld optimized_update_swap_score(OptimizedGraph &G, short node1, short node2) {
	ld C_w = G.C_w;
	auto &edges = G.invariant->edges;
	short i = G.invariant->edge_indices[node1], end = (node1 == G.invariant->V - 1 ? 2 * G.invariant->E : G.invariant->edge_indices[node1 + 1]);
	while (i < end) {
		short target, weight;
		tie(ignore, target, weight) = edges[i];
		if (target == node2) {
			i++;
			continue;
		}
		auto target_team = G.node_teams[target];
		if (target_team == G.node_teams[node1]) {
			C_w -= weight;
		} elif (target_team == G.node_teams[node2]) {
			C_w += weight;
		}
		i++;
	}
	i = G.invariant->edge_indices[node2], end = (node2 == G.invariant->V - 1 ? 2 * G.invariant->E : G.invariant->edge_indices[node2 + 1]);
	while (i < end) {
		short target, weight;
		tie(ignore, target, weight) = edges[i];
		if (target == node1) {
			i++;
			continue;
		}
		auto target_team = G.node_teams[target];
		if (target_team == G.node_teams[node2]) {
			C_w -= weight;
		} elif (target_team == G.node_teams[node1]) {
			C_w += weight;
		}
		i++;
	}
	return C_w;
}

void optimized_cross(OptimizedGraph &a, const OptimizedGraph &b) {
	short start = rand() % a.invariant->V, end = rand() % a.invariant->V;
	if (start < end) {
		FOB (node, start, end) {
			ch old_team = a.node_teams[node];
			ch new_team = b.node_teams[node];
			a.node_teams[node] = new_team;
			a.team_sizes[old_team]--;
			a.team_sizes[new_team]++;
		}
	} else {
		FOB (node, start, a.invariant->V) {
			ch old_team = a.node_teams[node];
			ch new_team = b.node_teams[node];
			a.node_teams[node] = new_team;
			a.team_sizes[old_team]--;
			a.team_sizes[new_team]++;
		}
		FOR (node, end) {
			ch old_team = a.node_teams[node];
			ch new_team = b.node_teams[node];
			a.node_teams[node] = new_team;
			a.team_sizes[old_team]--;
			a.team_sizes[new_team]++;
		}
	}
	optimized_get_score(a);
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

void optimized_read_input(OptimizedGraph &G) {
	json data = json::parse(fin);
	auto edges_json = data["links"];
	short V = sz(data["nodes"]),
		E = sz(edges_json),
		T = 0;
	G.node_teams = vector<ch>(V, -1);
	vector<tuple<short, short, short>> edges;
	vector<vector<short>> weights(V, vector<short>(V, 0));
	FORE (edge_json, edges_json) {
		short source = edge_json["source"], target = edge_json["target"], weight = edge_json["weight"];
		edges.pb({source, target, weight});
		edges.pb({target, source, weight});
		weights[source][target] = weight;
		weights[target][source] = weight;
	}
	sort(all(edges));
	vector<short> edge_indices(V, 0);
	FOR (i, V) {
		while (edge_indices[i] < 2 * E && get<0>(edges[edge_indices[i]]) < i) {
			edge_indices[i]++;
		}
	}
	G.invariant = make_shared<const OptimizedGraphInvariant>(OptimizedGraphInvariant(V, E, T, edge_indices, edges, weights));
}

void init_teams(OptimizedGraph &G, short T) {
	auto last_invariant = G.invariant;
	G.invariant = last_invariant->change_T(T);
	G.node_teams = vector<ch>(G.invariant->V, -1);
	G.team_sizes = vector<short>(G.invariant->T, 0);
	G.C_w = 0.0;
	G.K = K_COEFFICIENT * exp(K_EXP * G.invariant->T);
	G.B_vec = vector<ld>(G.invariant->T, 0.0);
	G.B_norm_squared = 0.0;
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

void optimized_read_teams(OptimizedGraph &G, str file) {
	str TEAM_FILE = IN_FILE + file + ".out";
	ifstream fin(TEAM_FILE);
	str text;
	getline(fin, text);
	json teams_json = json::parse(text);
	short T = 0;
	FOR (i, sz(teams_json)) {
		T = max(T, (short) teams_json[i]);
	}
	init_teams(G, T);
	FOR (i, sz(teams_json)) {
		G.node_teams[i] = (ch) (teams_json[i] - 1);
		G.team_sizes[G.node_teams[i]]++;
	}
}

void read_graph(Graph &G, str test_sz, ll test_id, str run_type = "") {
	set_io("tests/" + test_sz + "/" + test_sz + to_string(test_id) + "/", run_type);
	read_input(G);
}

void optimized_read_graph(OptimizedGraph &G, str test_sz, ll test_id, str run_type = "") {
	set_io("tests/" + test_sz + "/" + test_sz + to_string(test_id) + "/", run_type);
	optimized_read_input(G);
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

void optimized_read_best_graph(OptimizedGraph &G, str test_sz, ll test_id, str run_type = "") {
	optimized_read_graph(G, test_sz, test_id, run_type);
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
			stoll(team_file_name.substr(0, team_file_name.find("_"))) <=
			stoll(best_team_file_name.substr(0, best_team_file_name.find("_")))) {
			best_team_file_name = team_file_name;
		}
	}
	optimized_read_teams(G, best_team_file_name);
}

vector<OptimizedGraph> optimized_read_local_graphs(OptimizedGraph &G, str test_sz, ll test_id, str run_type = "") {
	vector<OptimizedGraph> local_graphs;
	for (const auto & entry : filesystem::directory_iterator("tests/" + test_sz + "/" + test_sz + to_string(test_id) + "/")) {
		filesystem::path team_file;
		team_file = entry.path();
		if (team_file.extension() != ".out") {
			continue;
		}
		str team_file_name = team_file.filename().string();
		team_file_name = team_file_name.substr(0, team_file_name.find("."));
		if (team_file_name.find("_") == string::npos) {
			cout << "Unlabeled team file: " << test_sz << test_id << "/" << team_file_name << endl;
			continue;
		}
		OptimizedGraph local_graph = G;
		optimized_read_teams(local_graph, team_file_name);
		optimized_get_score(local_graph);
		local_graphs.pb(local_graph);
	}
	sort(all(local_graphs), [](OptimizedGraph &a, OptimizedGraph &b) {
		return a.score < b.score;
	});
	return local_graphs;
}

ld get_own_score(str test_sz, str test_id) {
	ll best_score = -1;
	for (const auto & entry : filesystem::directory_iterator("tests/" + test_sz + "/" + test_sz + test_id + "/")) {
		filesystem::path team_file;
		team_file = entry.path();
		if (team_file.extension() != ".out") {
			continue;
		}
		str team_file_name = team_file.filename().string();
		team_file_name = team_file_name.substr(0, team_file_name.find("."));
		if (best_score == -1 ||
			stoll(team_file_name.substr(0, team_file_name.find("_"))) < best_score) {
			best_score = stoll(team_file_name.substr(0, team_file_name.find("_")));
		}
	}
	return best_score;
}

vector<Result> read_queue() {
	vector<Result> results;
	ifstream fin("queue.txt");
	str line;
	while (getline(fin, line)) {
		stringstream ss(line);
		ll id, rank;
		ld best_score, submission_score, local_score, delta_score;
		str size, local_file, url, notes;
		ss >> size >> id >> rank >> best_score >> submission_score >> local_score >> local_file >> url >> notes >> delta_score;
		results.pb({
			size,
			id,
			rank,
			best_score,
			submission_score,
			local_score,
			delta_score,
			local_file,
			url,
			notes
		});
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

void optimized_write_output(OptimizedGraph &G) {
	str OUT_FILE;
	if (RUN_TYPE == "") {
		OUT_FILE = IN_FILE.substr(0, sz(IN_FILE) - 1) + "/" + score_to_str(optimized_get_score(G)) + ".out";
	} else {
		OUT_FILE = IN_FILE.substr(0, sz(IN_FILE) - 1) + "/" + score_to_str(optimized_get_score(G)) + "_" + RUN_TYPE + ".out";
	}
	ofstream fout(OUT_FILE);
	fout << "[" << G.node_teams[0] + 1;
	FOB (i, 1, G.invariant->V) {
		fout << ", " << G.node_teams[i] + 1;
	}
	fout << "]" << endl;
}

ch max_teams(ld score) {
	return (ch) floor(log(score / K_COEFFICIENT) / K_EXP);
}

ld distribution_score(vector<short> team_size_counts, short V, ch team_count) {
	ld score = 0;
	FOR (i, sz(team_size_counts)) {
		ld tmp = (ld) i / V - 1.0 / team_count;
		score += team_size_counts[i] * tmp * tmp;
	}
	score = exp(B_EXP * sqrt(score));
	return score;
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

vector<short> find_distribution(ld &target_score, short V, ch team_count) {
	ld K = K_COEFFICIENT * exp(K_EXP * team_count);
	target_score -= K;
	target_score -= floor(target_score);
	vector<short> team_size_counts(V + 1);
	ld avg_team_size = V / (ld) team_count;
	short avg_team_size_short = (short) avg_team_size;
	short remaining = V - avg_team_size_short * team_count;
	FOR (i, remaining) {
		team_size_counts[avg_team_size_short + 1]++;
	}
	FOR (i, team_count - remaining) {
		team_size_counts[avg_team_size_short]++;
	}
	ld score = distribution_score(team_size_counts, V, team_count);
	while (score - floor(score) != target_score) {
		// do {
		// 	i = rand() % V + 1;
		// } while (team_size_counts[i] == 0);
		// team_size_counts[i]--;
		// team_size_counts[i - 1]++;
		// do {
		// 	j = rand() % V;
		// } while (team_size_counts[j] == 0);
		// team_size_counts[j]--;
		// team_size_counts[j + 1]++;
		// make avg values of i and j be more likely to be chosen
		vector<ld> weights(V + 1);
		FOR (k, sz(weights)) {
			if (team_size_counts[k] > 0) {
				weights[k] = 1.0 / (abs((ld) k - avg_team_size) + 1) * team_size_counts[k];
			}
		}
		ll i = weighted_random(weights);
		team_size_counts[i]--;
		team_size_counts[i - 1]++;
		if (team_size_counts[i] == 0) {
			weights[i] = 0;
		}
		ll j = weighted_random(weights);
		team_size_counts[j]--;
		team_size_counts[j + 1]++;
		score = distribution_score(team_size_counts, V, team_count);
	}
	vector<short> team_sizes;
	FOR (i, sz(team_size_counts)) {
		FOR (j, team_size_counts[i]) {
			team_sizes.pb(i);
		}
	}
	return team_sizes;
}

bool vec_eq(vector<char> &a, vector<char> &b) {
	if (sz(a) != sz(b)) {
		return false;
	}
	FOR (i, sz(a)) {
		if (a[i] != b[i]) {
			return false;
		}
	}
	return true;
}

int rand2() {
	return rand() << 15 + rand();
}