#include "extras.cpp"

struct simulated_annealing_agent_basic {
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

struct simulated_annealing_agent_weighted {
	Graph G;
	ld T;
	void init(Graph &G_in) {
		G = G_in;
		T = 1;
	}
	void step() {
		vector<ld> node_weights(G.V);
		FOR (i, G.V) {
			ll team = G.nodes[i].team;
			for (ll link : G.nodes[i].links) {
				ll neighbor = G.links[link].source + G.links[link].target - i;
				if (G.nodes[neighbor].team == team) {
					node_weights[i] += G.links[link].weight;
				}
			}
		}
		// FOR (i, G.V) {
		// 	weights[i] = 1 / (weights[i] + 1);
		// }
		ll node = weighted_random(node_weights);
		ll current_team = G.nodes[node].team;
		vector<ld> team_weights(sz(G.teams));
		FOR (i, G.V) {
			ll team = G.nodes[i].team;
			for (ll link : G.nodes[i].links) {
				ll neighbor = G.links[link].source + G.links[link].target - i;
				if (G.nodes[neighbor].team == team) {
					team_weights[team] += G.links[link].weight;
				}
			}
		}
		FOB (i, 1, sz(team_weights)) {
			team_weights[i] = 1 / (team_weights[i] + 1);
		}
		ll team = weighted_random(team_weights, 1);
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

struct simulated_annealing_agent_timed_weight {
	Graph G;
	ld T;
	ld a, b;
	vector<ld> node_t;
	ld get_weight(ll node) {
		// return 1 / ((node_t[node] / a - b) * (node_t[node] / a - b) + 1);
		return 0.5 * (1 + node_t[node] / a);
	}
	void init(Graph &G_in, ld a = 10000, ld b = 1) {
		G = G_in;
		T = 1;
		this->a = a;
		this->b = b;
		node_t = vector<ld>(G.V);
	}
	void step() {
		vector<ld> node_weights(G.V);
		FOR (i, G.V) {
			node_weights[i] = get_weight(i);
			node_t[i] += 1;
		}
		ll node = weighted_random(node_weights);
		ll current_team = G.nodes[node].team;
		ll team = rand() % (sz(G.teams) - 1) + 1;
		ld score = get_score(G);
		G.nodes[node].team = team;
		G.teams[team].nodes.pb(node);
		G.teams[current_team].nodes.erase(find(all(G.teams[current_team].nodes), node));
		ld new_score = get_score(G);
		if (new_score < score) {
			node_t[node] = 0;
			return;
		}
		ld p = exp((score - new_score) / T);
		if (rand() % 1000000 < p * 1000000) {
			node_t[node] = 0;
			return;
		}
		G.nodes[node].team = current_team;
		G.teams[team].nodes.pop_back();
		G.teams[current_team].nodes.pb(node);
	}
};

struct simulated_annealing_agent_swaps {
	Graph G;
	ld T;
	void init(Graph &G_in) {
		G = G_in;
		T = 1;
	}
	void step() {
		ll num_swap_nodes = T * 5 + 1;
		set<ll> swap_nodes;
		while (sz(swap_nodes) < num_swap_nodes) {
			swap_nodes.insert(rand() % G.V);
		}
		vector<ll> swap_nodes_vec(all(swap_nodes));
		vector<ll> curr_teams;
		for (ll node : swap_nodes_vec) {
			curr_teams.pb(G.nodes[node].team);
		}
		vector<ll> new_teams(num_swap_nodes);
		FOR (i, num_swap_nodes) {
			new_teams[i] = rand() % (sz(G.teams) - 1) + 1;
		}
		// vector<ll> new_teams = curr_teams;
		// shuffle(all(new_teams), default_random_engine(rand()));
		ld score = get_score(G);
		FOR (i, sz(swap_nodes_vec)) {
			ll node = swap_nodes_vec[i];
			ll team = new_teams[i];
			G.nodes[node].team = team;
			G.teams[team].nodes.pb(node);
			G.teams[curr_teams[i]].nodes.erase(find(all(G.teams[curr_teams[i]].nodes), node));
		}
		ld new_score = get_score(G);
		if (new_score < score) {
			return;
		}
		ld p = exp((score - new_score) / T);
		if (rand() % 1000000 < p * 1000000) {
			return;
		}
		FOR (i, sz(swap_nodes_vec)) {
			ll node = swap_nodes_vec[i];
			ll team = curr_teams[i];
			G.nodes[node].team = team;
			G.teams[team].nodes.pb(node);
			G.teams[new_teams[i]].nodes.erase(find(all(G.teams[new_teams[i]].nodes), node));
		}
	}
};

Graph simulated_annealing(Graph &G_in, ld decay_rate = 0.99) {
	simulated_annealing_agent_team_adjustment agent;
	agent.init(G_in);
	while (agent.T > 0.0001) {
		FOR (i, 1000) {
			agent.step();
		}
		agent.T *= decay_rate;
	}
	return agent.G;
}

struct genetic_algorithm_controller_sim_annealing {
	vector<pair<ld, Graph>> population;
	ld T_start, T_end;
	void init(Graph &G_in, ll team_count, ll population_size) {
		population = vector<pair<ld, Graph>>(population_size);
		FOR (i, population_size) {
			population[i].second = G_in;
			population[i].second = random_assignment(G_in, sz(G_in.teams) - 1 + rand() % 5 - 2);
			// read_teams(population[i].second, "32825_sim_anneal");
			population[i].first = get_score(population[i].second);
		}
		population[0].second = G_in;
		population[0].first = get_score(population[0].second);
		T_start = 10;
		T_end = 9.5;
	}
	void step(bool prune = false) {
		FOR (i, sz(population)) {
			// simulated_annealing_agent_team_adjustment agent;
			simulated_annealing_agent_basic agent;
			agent.init(population[i].second);
			agent.T = T_start;
			while (agent.T > T_end) {
				FOR (i, 100) {
					agent.step();
				}
				agent.T *= 0.99;
			}
			population[i].second = agent.G;
			population[i].first = get_score(population[i].second);
		}
		if (prune) {
			sort(all(population), [](pair<ld, Graph> &a, pair<ld, Graph> &b) {
				return a.first < b.first;
			});
			FOR (i, sz(population) / 2) {
				population[i + sz(population) / 2] = population[i];
			}
		}
		T_start *= 0.99;
		T_end *= 0.99;
	}
};

struct genetic_algorithm_controller_sim_annealing_ants {
	Graph G_0;
	vector<pair<ld, Graph>> population;
	vector<vector<ld>> pheromones;
	ld T_start, T_end;
	void init(Graph &G_in, ll team_count, ll population_size) {
		G_0 = G_in;
		population = vector<pair<ld, Graph>>(population_size);
		FOR (i, population_size) {
			population[i].second = random_assignment(G_in, team_count);
			// population[i].second = G_in;
			// read_teams(population[i].second, "3316_sim_anneal");
		}
		T_start = 10;
		T_end = 9.5;
		pheromones = vector<vector<ld>>(G_in.V, vector<ld>(G_in.V));
	}
	Graph generate_from_pheromones() {
		Graph G = G_0;
		vector<ll> nodes(G.V);
		iota(all(nodes), 0);
		shuffle(all(nodes), default_random_engine(rand()));
		DSU dsu(G.V);
		FOR (i, G.V) {
			ll node = nodes[i];
			vector<ld> weights(G.V);
			FOR (j, G.V) {
				weights[j] = pheromones[node][j];
			}
			ll team = weighted_random(weights);
			dsu.unite(node, team);
		}
		map<ll, ll> team_map;
		FOR (i, G.V) {
			ll team = dsu.get(i);
			if (team_map.find(team) == team_map.end()) {
				team_map[team] = sz(team_map) + 1;
			}
			G.nodes[i].team = team_map[team];
		}
		G.teams = vector<Team>(sz(team_map) + 1);
		FOR (i, G.V) {
			G.teams[G.nodes[i].team].nodes.pb(i);
		}
		return G;
	}
	void step(bool team_adjustment = false) {
		if (team_adjustment) {
			FOR (i, sz(population)) {
				simulated_annealing_agent_team_adjustment agent;
				// simulated_annealing_agent_swaps agent;
				agent.init(population[i].second);
				agent.T = T_start;
				while (agent.T > T_end) {
					FOR (i, 100) {
						agent.step();
					}
					agent.T *= 0.99;
				}
				population[i].second = agent.G;
				population[i].first = get_score(population[i].second);
			}
			sort(all(population), [](pair<ld, Graph> &a, pair<ld, Graph> &b) {
				return a.first < b.first;
			});
			FOR (i, sz(population) / 2) {
				FOR (u, sz(population[i].second.nodes)) {
					FOR (v, sz(population[i].second.nodes)) {
						if (population[i].second.nodes[u].team == population[i].second.nodes[v].team) {
							pheromones[u][v] += 1.0 / population[i].first;
						}
					}
				}
			}
			FOR (i, sz(population) / 2) {
				population[i + sz(population) / 2].second = generate_from_pheromones();
			}
			T_start *= 0.99;
			T_end *= 0.99;
		} else {
			FOR (i, sz(population)) {
				// simulated_annealing_agent_team_adjustment agent;
				simulated_annealing_agent_swaps agent;
				agent.init(population[i].second);
				agent.T = T_start;
				while (agent.T > T_end) {
					FOR (i, 100) {
						agent.step();
					}
					agent.T *= 0.95;
				}
				population[i].second = agent.G;
				population[i].first = get_score(population[i].second);
			}
			sort(all(population), [](pair<ld, Graph> &a, pair<ld, Graph> &b) {
				return a.first < b.first;
			});
			FOR (i, sz(population) / 2) {
				FOR (u, sz(population[i].second.nodes)) {
					FOR (v, sz(population[i].second.nodes)) {
						if (population[i].second.nodes[u].team == population[i].second.nodes[v].team) {
							pheromones[u][v] += 1.0 / population[i].first;
						}
					}
				}
			}
			FOR (i, sz(population) / 2) {
				population[i + sz(population) / 2].second = generate_from_pheromones();
			}
			T_start *= 0.99;
			T_end *= 0.99;
		}
	}
};

Graph genetic_algorithm(Graph &G_in, ll team_count, ll population_size = 10, ll generations = 1000, ld mutation_rate = 0.1) {
	genetic_algorithm_controller_sim_annealing controller;
	controller.init(G_in, team_count, population_size);
	FOR (i, 50) {
		controller.step();
	}
	ld previous_score = 1e18;
	ll stagnation = 0;
	FOR (i, generations) {
		controller.step(true);
		ld score = controller.population[0].first;
		if (abs(score - previous_score) <= 1) {
			stagnation++;
			if (stagnation >= 10) {
				break;
			}
		} else {
			stagnation = 0;
		}
		previous_score = score;
	}
	return controller.population[0].second;
}

// int main() {
// 	FOB (i, 3, 12) {
// 		srand(time(NULL));
// 		set_io("tests/medium/medium105/", "sim_anneal");

// 		Graph G;
// 		read_input(G);
// 		G = genetic_algorithm(G, i, 10, 300);
// 		// read_teams(G, "65232_sim_anneal");
// 		// G = random_assignment(G, 10);
// 		// G = simulated_annealing(G);
// 		write_output(G);
// 	}
// 	return 0;
// }

int main() {
	vector<str> test_sets = {/* "small", "medium", */"large"};
	ll test_count = 259;
	FORE (test_sz, test_sets) {
		ll test_id = test_count;
		while (test_id > 0) {
			set_io("tests/" + test_sz + "/" + test_sz + to_string(test_id) + "/", "sim_anneal");
			cout << "Running " << test_sz << test_id << '\n';
			Graph G;
			read_input(G);
			str best_team_file_name = "9999999999";
			for (const auto & entry : filesystem::directory_iterator("tests/" + test_sz + "/" + test_sz + to_string(test_id) + "/")) {
				filesystem::path team_file;
				team_file = entry.path();
				if (team_file.extension() == ".in") {
					continue;
				}
				str team_file_name = team_file.filename().string();
				team_file_name = team_file_name.substr(0, team_file_name.find("."));
				if (stoll(team_file_name.substr(0, team_file_name.find("_"))) < stoll(best_team_file_name.substr(0, best_team_file_name.find("_")))) {
					best_team_file_name = team_file_name;
				}
			}
			read_teams(G, best_team_file_name);
			G = genetic_algorithm(G, 12, 10, 300);
			write_output(G);
			test_id--;
		}
	}
	return 0;
}