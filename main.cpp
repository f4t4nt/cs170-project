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
		ll num_swap_nodes = T * 20 + 1;
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
			population[i].second = random_assignment(G_in, team_count);
			// population[i].second = G_in;
			// read_teams(population[i].second, "40344_sim_anneal");
			population[i].first = get_score(population[i].second);
		}
		T_start = 1;
		T_end = 0.95;
	}
	void step() {
		FOR (i, sz(population)) {
			simulated_annealing_agent_swaps agent;
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
			population[i + sz(population) / 2] = population[i];
		}
		T_start *= 0.99;
		T_end *= 0.99;
	}
};

Graph genetic_algorithm(Graph &G_in, ll team_count, ll population_size = 10, ll generations = 1000, ld mutation_rate = 0.1) {
	genetic_algorithm_controller_sim_annealing controller;
	controller.init(G_in, team_count, population_size);
	FOR (i, generations) {
		controller.step();
	}
	return controller.population[0].second;
}

int main() {
	// srand(time(NULL));
	set_io("tests/large/max_weight/", "sim_anneal");

	Graph G;
	read_input(G);
	G = genetic_algorithm(G, 10, 10, 200);
	// read_teams(G, "65232_sim_anneal");
	// G = random_assignment(G, 10);
	// G = simulated_annealing(G);
	write_output(G);
	return 0;
}