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