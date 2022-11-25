#include "header.cpp"

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

Graph greedy(Graph &G_in) { // inserts all nodes into one team
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

struct simulated_annealing_agent_team_adjustment { // generally better to focus basic on optimal team count
	Graph G;
	ld T;
	void init(Graph &G_in) {
		G = G_in;
		T = 1;
	}
	void step() {
		ll move = rand() % (G.V + 2);
		if (move == G.V + 1) {
			ld score = get_score(G);
			Graph G_copy = G;
			G_copy.teams.pb(Team());
			while (true) {
				ll largest_team = -1;
				ll largest_team_size = -1;
				FOB (team, 1, sz(G_copy.teams)) {
					if (sz(G_copy.teams[team].nodes) > largest_team_size) {
						largest_team = team;
						largest_team_size = sz(G_copy.teams[team].nodes);
					}
				}
				if (largest_team_size == sz(G_copy.teams[sz(G_copy.teams) - 1].nodes)) {
					break;
				}
				ll node = G_copy.teams[largest_team].nodes[rand() % sz(G_copy.teams[largest_team].nodes)];
				G_copy.teams[largest_team].nodes.erase(find(all(G_copy.teams[largest_team].nodes), node));
				G_copy.teams[sz(G_copy.teams) - 1].nodes.pb(node);
				G_copy.nodes[node].team = sz(G_copy.teams) - 1;
			}
			ld new_score = get_score(G_copy);
			if (new_score < score) {
				G = G_copy;
			}
			ld p = exp((score - new_score) / T);
			if (rand() % 1000000 < p * 1000000) {
				G = G_copy;
			}
		} elif (move == G.V) {
			if (sz(G.teams) == 2) {
				return;
			}
			ld score = get_score(G);
			Graph G_copy = G;
			ll team = rand() % (sz(G_copy.teams) - 1) + 1;
			FORE (node, G_copy.teams[team].nodes) {
				ll new_team = rand() % (sz(G_copy.teams) - 1) + 1;
				while (new_team == team) {
					new_team = rand() % (sz(G_copy.teams) - 1) + 1;
				}
				G_copy.teams[new_team].nodes.pb(node);
				G_copy.nodes[node].team = new_team;
			}
			G_copy.teams[team].nodes.clear();
			ld new_score = get_score(G_copy);
			if (new_score < score) {
				G = G_copy;
			}
			ld p = exp((score - new_score) / T);
			if (rand() % 1000000 < p * 1000000) {
				G = G_copy;
			}
		} else {
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

struct physics_controller { // need more experimenting on physics_misc.py, but it might not be worth it
	Graph G;
	ld dt;
	ll dim;
	ld v_decay;
	vector<ld> origin;
	vector<vector<ld>> node_pos;
	vector<vector<ld>> node_vel;
	vector<ld> antigravity(vector<ld> &pos1, vector<ld> &pos2, ld coeff) {
		ld dist = 0;
		FOR (i, dim) {
			dist += (pos1[i] - pos2[i]) * (pos1[i] - pos2[i]);
		}
		dist = sqrt(dist);
		vector<ld> accel(dim);
		FOR (i, dim) {
			accel[i] = coeff * (pos1[i] - pos2[i]) / (dist * dist * dist);
		}
		return accel;
	}
	vector<ld> spring(vector<ld> &pos1, vector<ld> &pos2, ld equil, ld k = 1) {
		ld dist = 0;
		FOR (i, dim) {
			dist += (pos1[i] - pos2[i]) * (pos1[i] - pos2[i]);
		}
		dist = sqrt(dist);
		vector<ld> accel(dim);
		FOR (i, dim) {
			accel[i] = k * (dist - equil) * (pos1[i] - pos2[i]) / dist;
		}
		return accel;
	}
	void init(Graph &G_in, ld dt = 0.01, ll dim = 2, ld k = 1, ld v_decay = 0.99) {
		G = G_in;
		this->dt = dt;
		this->dim = dim;
		this->v_decay = v_decay;
		origin = vector<ld>(dim);
		node_pos = vector<vector<ld>>(G.V, vector<ld>(dim));
		node_vel = vector<vector<ld>>(G.V, vector<ld>(dim));
		FOR (i, G.V) {
			FOR (j, dim) {
				node_pos[i][j] = rand() % 1000;
				node_vel[i][j] = 0;
			}
		}
	}
	void step() {
		FOR (i, G.V) {
			FOR (j, dim) {
				node_vel[i][j] = 0;
			}
		}
		FOR (i, G.V) {
			FOR (j, i) {
				vector<ld> accel = spring(node_pos[i], node_pos[j], G.weights[i][j]);
				FOR (d, dim) {
					node_vel[i][d] += accel[d] * dt;
					node_vel[j][d] -= accel[d] * dt;
				}
			}
		}
		FOR (i, G.V) {
			vector<ld> accel = spring(node_pos[i], origin, 0, 1000);
			FOR (d, dim) {
				node_vel[i][d] += accel[d] * dt;
			}
		}
		FOR (i, G.V) {
			FOR (j, dim) {
				node_pos[i][j] += node_vel[i][j] * dt;
			}
		}
		FOR (i, G.V) {
			FOR (j, dim) {
				node_vel[i][j] *= v_decay;
			}
		}
	}
};

Graph physics(Graph &G_in, ll total_steps, ld dt = 0.001, ll dim = 2, ld k = 1) {
	physics_controller controller;
	controller.init(G_in, dt, dim, k);
	FOR (i, total_steps) {
		controller.step();
	}
	return controller.G;
}

struct genetic_algorithm_controller_basic { // unmotivated genetic algorithm doesn't work well
	vector<Graph> population;
	ld mutation_rate;
	void init(Graph &G_in, ll team_count, ll population_size, ld mutation_rate = 0.1) {
		this->mutation_rate = mutation_rate;
		population = vector<Graph>(population_size);
		FOR (i, population_size) {
			population[i] = random_assignment(G_in, team_count);
		}
	}
	void step() {
		vector<Graph> new_population;
		FOR (i, sz(population)) {
			ll a = rand() % sz(population);
			ll b = rand() % sz(population);
			Graph G_a = population[a];
			Graph G_b = population[b];
			Graph G_c = crossover(G_a, G_b);
			G_c = mutate(G_c, mutation_rate);
			new_population.pb(G_c);
		}
		sort(all(new_population), [](Graph &a, Graph &b) {
			return get_score(a) > get_score(b);
		});
		population = vector<Graph>(new_population.begin(), new_population.begin() + sz(population));
	}
	Graph crossover(Graph &G_a, Graph &G_b) {
		Graph G_c = G_a;
		FOR (i, G_c.V) {
			if (rand() % 2) {
				G_c.nodes[i].team = G_b.nodes[i].team;
			}
		}
		return G_c;
	}
	Graph mutate(Graph &G, ld rate) {
		Graph G_m = G;
		FOR (i, G_m.V) {
			if (rand() % 1000000 < rate * 1000000) {
				G_m.nodes[i].team = rand() % (sz(G_m.teams) - 1) + 1;
			}
		}
		return G_m;
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

Graph coloring(Graph &G_in) { // silver bullet for pesky tests
	Graph G = G_in;
	ld best_score = INF;
	FOR (threshold, 10) {
		Graph G_copy = G;
		FOR (i, G_copy.V) {
			G_copy.nodes[i].team = -1;
		}
		G_copy.teams = vector<Team>(1);
		FOR (i, G_copy.V) {
			set<ll> neighbors;
			FOR (j, G_copy.V) {
				if (G_copy.weights[i][j] > threshold) {
					neighbors.insert(G_copy.nodes[j].team);
				}
			}
			ll team = 1;
			vector<ll> possible_teams;
			while (team < G_copy.teams.size()) {
				if (neighbors.find(team) == neighbors.end()) {
					possible_teams.push_back(team);
				}
				team++;
			}
			if (possible_teams.empty()) {
				team = G_copy.teams.size();
			} else {
				team = possible_teams[0];
				FOR (j, sz(possible_teams)) {
					if (sz(G_copy.teams[possible_teams[j]].nodes) < sz(G_copy.teams[team].nodes)) {
						team = possible_teams[j];
					}
				}
			}
			G_copy.nodes[i].team = team;
			if (team >= sz(G_copy.teams)) {
				G_copy.teams.pb({});
				G_copy.teams.back().nodes.pb(i);
			} else {
				G_copy.teams[team].nodes.pb(i);
			}
		}
		ld score = get_score(G_copy);
		if (score < best_score) {
			best_score = score;
			G = G_copy;
		}
	}
	return G;
}

// ------------------------------

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

struct genetic_algorithm_controller_sim_annealing {
	vector<pair<ld, Graph>> population;
	ld T_start, T_end;
	void init(Graph &G_in, ll team_count, ll population_size) {
		population = vector<pair<ld, Graph>>(population_size);
		FOR (i, population_size) {
			// population[i].second = G_in;
			population[i].second = random_assignment(G_in, team_count);
			// read_teams(population[i].second, "32825_sim_anneal");
			population[i].first = get_score(population[i].second);
		}
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

Graph genetic_algorithm(Graph &G_in, ll team_count, ll population_size = 10, ll generations = 1000, ld mutation_rate = 0.1, ld best_score = 0.0) {
	genetic_algorithm_controller_sim_annealing controller;
	controller.init(G_in, team_count, population_size);
	FOR (i, 50) {
		controller.step();
	}
	Graph all_time_best = controller.population[0].second;
	ld all_time_best_score = controller.population[0].first;
	ld previous_score = 1e18;
	ll stagnation = 0;
	FOR (i, generations) {
		controller.step(true);
		ld score = controller.population[0].first;
		if (abs(score - previous_score) / previous_score < 1e-3) {
			stagnation++;
			if (stagnation >= 10) {
				break;
			}
		} else {
			stagnation = 0;
		}
		if (score < all_time_best_score) {
			all_time_best = controller.population[0].second;
			all_time_best_score = score;
			if (all_time_best_score < best_score + 1e-3) {
				break;
			}
		}
		previous_score = score;
	}
	return all_time_best;
}

// ------------------------------

struct OptimizedShepardAgent {
	short V;
	ch T;
	ld mutation_rate;
	unsigned int population_size, mutations, cross_overs;
	ld variation = 0.0;
	vector<OptimizedGraph> population;
	void init(OptimizedGraph &G_in, ll team_count, ll population_size, ld mutation_rate, bool randomize = true) {
		this->population_size = population_size;
		this->mutation_rate = mutation_rate;
		mutations = population_size * 1000;
		cross_overs = population_size * 500;
		population = vector<OptimizedGraph>(population_size + mutations + cross_overs);
		// if (randomize) {
		// 	FOR (i, population_size * 20) {
		// 		population[i] = optimized_random_assignment(G_in, team_count);
		// 		optimized_get_score(population[i]);
		// 	}
        //     sort(population.begin(), population.begin() + population_size * 20, [](OptimizedGraph &a, OptimizedGraph &b) {
        //         return a.score < b.score;
        //     });
		// } else {
			ld score = optimized_get_score(G_in);
			FOR (i, population_size) {
				population[i] = G_in;
				population[i].score = score;
			}
		// }
		V = population[0].invariant->V;
		T = population[0].invariant->T;
	}
	short rand_parent() {
		// size_t sm = population_size * 201 + population_size * population_size;
		// size_t y = rand2() % sm;
		// auto result = population_size - 1 - (size_t)((sqrt(201*201 + 4 * y) - 201) / 2);
		// return max(0, -1 + (short)result);
		vector<ld> weights(population_size);
		FOR (i, population_size) {
			weights[i] = 1e6 / population[i].score / population[i].score;
		}
		return weighted_random(weights);
	}
	void mutate(size_t i) {
		auto &G = population[i];
		G = population[rand_parent()];
		short start = rand() % V;
		short end = (start + rand() % ((ll) (mutation_rate * V) + 1)) % V;
		if (start <= end) {
			FOB (node, start, end) {
				ch old_team = G.node_teams[node];
				ch new_team = rand() % T;
				while (new_team == old_team) {
					new_team = rand() % T;
				}
				G.node_teams[node] = new_team;
				G.team_counts[old_team]--;
				G.team_counts[new_team]++;
			}
		} else {
			FOB (node, start, V) {
				ch old_team = G.node_teams[node];
				ch new_team = rand() % T;
				while (new_team == old_team) {
					new_team = rand() % T;
				}
				G.node_teams[node] = new_team;
				G.team_counts[old_team]--;
				G.team_counts[new_team]++;
			}
			FOR (node, end) {
				ch old_team = G.node_teams[node];
				ch new_team = rand() % T;
				while (new_team == old_team) {
					new_team = rand() % T;
				}
				G.node_teams[node] = new_team;
				G.team_counts[old_team]--;
				G.team_counts[new_team]++;
			}
		}
		optimized_get_score(G);
		ll shift = rand() % 3 + 1;
		FOR (node, V) {
			G.node_teams[node] = (G.node_teams[node] + shift) % T;
		}
		rotate(G.node_teams.begin(), G.node_teams.begin() + shift, G.node_teams.end());
	}
	void cross(size_t i) {
		short a = rand_parent();
		short b = rand_parent();
		while(a == b) {
			b = rand_parent();
		}
		auto& g = population[i];
		g = population[a];
		optimized_cross(g, population[b]);
	}
	void prune() {
        FOR (i, population_size) {
            population[i].score = 1e18;
        }
		sort(all(population), [](const OptimizedGraph &a, const OptimizedGraph &b) {
			return a.score < b.score;
		});
		size_t i = 1, j = 0, k = 1;
		for (; i < population_size; i++) {
			while (population[j].score == population[k].score &&
                population[j].node_teams[0] == population[k].node_teams[0]) {
				k++;
			}
			population[i] = population[k];
			j = k;
			k++;
		}
		variation = population_size * 1.0 / k;
	}
	void step() {
		#pragma omp parallel for
		for (size_t i = population_size; i < sz(population); i++) {
			if (i < population_size + cross_overs) {
				cross(i);
			} else {
				mutate(i);
			}
		}
		prune();
		mutation_rate = max((ld) 0.05, 0.99 * mutation_rate);
	}
};

OptimizedGraph optimized_genetic_algorithm(OptimizedGraph &G, ll team_count, ll population_size, ll generations, bool randomize = false, ld mutation_rate = 1, ld target_score = 0, short stagnation_limit = 1) {
	G.score = INF;
	OptimizedShepardAgent shepard;
	shepard.init(G, team_count, population_size, mutation_rate, randomize);
	bool extended = true;
	ll stagnation = 0;
	ld previous_score = 1e18;
	auto tmp = vector<short>(500);
	FOR (i, generations) {
		auto population_best = shepard.population[0];
		if (population_best.score < G.score) {
			G = population_best;
		}
		if (i % 10 == 0) {
			optimized_write_output(G);
			cout << "Generation " << i << " best score " << G.score
				<< ", worst score (" << shepard.population[shepard.population_size - 1].score
				<< " | " << shepard.population[sz(shepard.population) - 1].score
				<< "), variation "  << shepard.variation << ", mutation rate " << shepard.mutation_rate << endl;
			if (previous_score == G.score) {
				stagnation++;
				if (stagnation >= stagnation_limit) {
					cout << "Stagnation limit reached, terminating" << endl;
					break;
				}
			}
			if (G.score <= target_score && extended) {
				stagnation_limit++;
				extended = false;
			}
			previous_score = G.score;
		}
		shepard.step();
	}
	return G;
}

void dolly_solve(Result &result, ld target_score) {
	OptimizedGraph G;
	short population_sz = 900;
	cout << "Dollying " << result.size << result.id << " with target score " << target_score << " and population size " << population_sz << endl << endl;
	ch team_count = max_teams(result.best_score);
	ld previous_score = INF;
	// optimized_read_graph(G, result.size, result.id, "sick2");
	optimized_read_best_graph(G, result.size, result.id, "sick2");
    team_count = G.invariant->T + 1;
	while (team_count >= 2) {
		cout << "Trying " << (ll) team_count << " teams" << endl;
		init_teams(G, team_count);
		G = optimized_genetic_algorithm(G, team_count, population_sz, 2000, true, 1, target_score, 2);
		optimized_write_output(G);
		if (G.score < target_score + 1e-9) {
			cout << "Target score reached, terminating" << endl;
		} elif (G.score > previous_score) {
			cout << "Increase limit reached, terminating" << endl;
			break;
		}
		cout << endl;
		team_count--;
		previous_score = G.score;
	}
}

void dolly_improve(Result &result) {
	OptimizedGraph G;
	short population_sz = 100;
	optimized_read_best_graph(G, result.size, result.id, "sick2");
	cout << "Shearing " << result.size << result.id << ", current score " << optimized_get_score(G) << " with population size " << population_sz << endl << endl;
	G = optimized_genetic_algorithm(G, G.invariant->T, population_sz, 1000, false, 0.05, 0, 100);
	optimized_write_output(G);
	if (G.score < result.best_score) {
		cout << "Found better score" << endl;
	}
}