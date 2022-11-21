#include "extras.cpp"

OptimizedGraph optimized_coloring_threshold(OptimizedGraph &G_in) {
	OptimizedGraph G = G_in;
	G.score = INF;
	FOR (threshold, 10) {
		OptimizedGraph G_copy = G_in;
		FOR (i, G_copy.V) {
			G_copy.node_teams[i] = -1;
		}
		vector<ll> shuffled(G_copy.V);
		iota(all(shuffled), 0);
		random_shuffle(shuffled);
		FOR (i, G_copy.V) {
			set<short> neighbors;
			FOR (j, G_copy.V) {
				if (G_copy.weights[i][j] > threshold) {
					neighbors.insert(G_copy.node_teams[j]);
				}
			}
			ll team = 0;
			vector<ll> possible_teams;
			while (team < G_copy.T) {
				if (neighbors.find(team) == neighbors.end()) {
					possible_teams.pb(team);
				}
				team++;
			}
			if (possible_teams.empty()) {
				team = G_copy.T;
			} else {
				team = possible_teams[0];
				FOR (j, sz(possible_teams)) {
					if (G_copy.team_counts[possible_teams[j]] < G_copy.team_counts[team]) {
						team = possible_teams[j];
					}
				}
			}
			G_copy.node_teams[i] = team;
			if (team == G_copy.T) {
				G_copy.T++;
				G_copy.team_counts.pb(1);
				G_copy.B_vec.pb(0);
			} else {
				G_copy.team_counts[team]++;
			}
		}
		optimized_get_score(G_copy);
		if (G_copy.score < G.score) {
			G = G_copy;
		}
	}
	return G;
}

OptimizedGraph optimized_random_assignment(OptimizedGraph &G_in, ll team_count) {
	OptimizedGraph G = G_in;
	G.T = team_count;
	G.team_counts = vector<ll>(team_count, 0);
	G.B_vec = vector<ld>(team_count, 0);
	FOR (i, G.V) {
		G.node_teams[i] = rand() % team_count;
		G.team_counts[G.node_teams[i]]++;
	}
	return G;
}

struct OptimizedAnnealingAgent {
	OptimizedGraph G;
	ld T;
	void init(OptimizedGraph &G_in, ld T0) {
		G = G_in;
		T = T0;
	}
	void step() {
		ll node = rand() % (ll) G.V;
		ll old_team = G.node_teams[node];
		ll new_team = rand() % (ll) G.T;
		while (new_team == old_team) {
			new_team = rand() % (ll) G.T;
		}
		ld C_w, K, B, B_norm_squared, B_old, B_new;
		tie(C_w, K, B, B_norm_squared, B_old, B_new) = optimized_update_score(G, node, old_team, new_team);
		ld new_score = C_w + K + B;
		// G.node_teams[node] = new_team;
		// G.team_counts[old_team]--;
		// G.team_counts[new_team]++;
		// ld test_score = optimized_get_score(G);
		// if (abs(new_score - test_score) >= 1e-6) {
		// 	cout << "ERROR: " << new_score << " " << test_score << endl;
		// 	cout << "C_w: " << C_w << " " << G.C_w << endl;
		// 	cout << "K: " << K << " " << G.K << endl;
		// 	cout << "B: " << B << " " << exp(B_EXP * sqrt(B_norm_squared)) << endl;
		// 	cout << "B_norm_squared: " << B_norm_squared << " " << G.B_norm_squared << endl;
		// 	optimized_get_score(G, node);
		// 	assert(false);
		// }
		// G.node_teams[node] = old_team;
		// G.team_counts[old_team]++;
		// G.team_counts[new_team]--;
		if (new_score < G.score) {
			G.node_teams[node] = new_team;
			G.team_counts[old_team]--;
			G.team_counts[new_team]++;
			G.B_vec[old_team] = B_old;
			G.B_vec[new_team] = B_new;
			G.score = new_score;
			G.C_w = C_w;
			G.K = K;
			G.B_norm_squared = B_norm_squared;
			return;
		}
		ld p = exp((G.score - new_score) / T);
		if (rand() % 1000000 < p * 1000000) {
			G.node_teams[node] = new_team;
			G.team_counts[old_team]--;
			G.team_counts[new_team]++;
			G.B_vec[old_team] = B_old;
			G.B_vec[new_team] = B_new;
			G.score = new_score;
			G.C_w = C_w;
			G.K = K;
			G.B_norm_squared = B_norm_squared;
			return;
		}
	}
};

struct OptimizedAnnealingBatchAgent {
	OptimizedGraph G;
	ld T;
	void init(OptimizedGraph &G_in, ll batch_size0, ld T0) {
		G = G_in;
		T = T0;
	}
	void stepSingle() {
		ll node = rand() % (ll) G.V;
		ll old_team = G.node_teams[node];
		ll new_team = rand() % (ll) G.T;
		while (new_team == old_team) {
			new_team = rand() % (ll) G.T;
		}
		ld C_w, K, B, B_norm_squared, B_old, B_new;
		tie(C_w, K, B, B_norm_squared, B_old, B_new) = optimized_update_score(G, node, old_team, new_team);
		ld new_score = C_w + K + B;
		if (new_score < G.score) {
			G.node_teams[node] = new_team;
			G.team_counts[old_team]--;
			G.team_counts[new_team]++;
			G.B_vec[old_team] = B_old;
			G.B_vec[new_team] = B_new;
			G.score = new_score;
			G.C_w = C_w;
			G.K = K;
			G.B_norm_squared = B_norm_squared;
			return;
		}
		ld p = exp((G.score - new_score) / T);
		if (rand() % 1000000 < p * 1000000) {
			G.node_teams[node] = new_team;
			G.team_counts[old_team]--;
			G.team_counts[new_team]++;
			G.B_vec[old_team] = B_old;
			G.B_vec[new_team] = B_new;
			G.score = new_score;
			G.C_w = C_w;
			G.K = K;
			G.B_norm_squared = B_norm_squared;
			return;
		}
	}
	void stepSwaps(ll batch_size = 2) {
		if (batch_size <= 0) {
			return;
		} elif (batch_size == 1) {
			stepSingle();
			return;
		}
		ll team_count = 0;
		vector<ll> team_pick_node(G.T, -1);
		while (team_count < batch_size) {
			ll team = rand() % (ll) G.T;
			if (team_pick_node[team] == -1) {
				continue;
			}
			team_pick_node[team] = 1;
			team_count++;
		}
		FOR (i, G.T) {
			if (team_pick_node[i]) {
				team_pick_node[i] = rand() % (ll) G.team_counts[i];
			}
		}
		vector<ll> nodes, old_teams;
		FOR (i, G.V) {
			if (team_pick_node[G.node_teams[i]] == 0) {
				nodes.push_back(i);
				old_teams.push_back(G.node_teams[i]);
			}
			team_pick_node[G.node_teams[i]]--;
		}
		assert(nodes.size() == batch_size);
		vector<ll> new_teams = old_teams;
		random_shuffle(new_teams);
		ld C_w = optimized_update_score_batch_swaps(G, nodes, old_teams, new_teams);
		if (C_w < G.C_w) {
			FOR (i, batch_size) {
				G.node_teams[nodes[i]] = new_teams[i];
				G.team_counts[old_teams[i]]--;
				G.team_counts[new_teams[i]]++;
			}
			G.C_w = C_w;
			G.score = C_w + G.K + G.B_norm_squared;
			return;
		}
		ld p = exp((G.C_w - C_w) / T);
		if (rand() % 1000000 < p * 1000000) {
			FOR (i, batch_size) {
				G.node_teams[nodes[i]] = new_teams[i];
				G.team_counts[old_teams[i]]--;
				G.team_counts[new_teams[i]]++;
			}
			G.C_w = C_w;
			G.score = C_w + G.K + G.B_norm_squared;
			return;
		}
	}
};

struct OptimizedGeneticController {
	vector<OptimizedAnnealingAgent> population;
	ld T_start, T_end;
	void init(OptimizedGraph &G_in, ll team_count, ll population_size, ld T_start0, ld T_end0) {
		population = vector<OptimizedAnnealingAgent>(population_size);
		T_start = T_start0;
		T_end = T_end0;
		FOR (i, population_size) {
			population[i] = {optimized_random_assignment(G_in, team_count), T_start};
			// population[i] = {G_in, T_start};
			optimized_get_score(population[i].G);
		}
	}
	void step() {
		auto iter_count = sz(population);
		#pragma omp parallel for
		for (size_t i = 0; i < iter_count; i++) {
			auto &agent = population[i];
			agent.T = T_start;
			while (agent.T > T_end) {
				FOR (j, 100) {
					agent.step();
				}
				agent.T *= 0.999;
			}
		}

		T_start *= 0.991;
		T_end *= 0.991;
	}
	void step_and_prune() {
		step();
		sort(all(population), [](OptimizedAnnealingAgent &a, OptimizedAnnealingAgent &b) {
			return a.G.score < b.G.score;
		});
		FOR (i, sz(population) / 2) {
			population[i + sz(population) / 2] = population[i];
		}
	}
};

OptimizedGraph optimized_algorithm(OptimizedGraph &G_in, ll team_count, ll population_size, ll generations, ld T_start0, ld T_end0, ll stagnation_limit = 10, ld target_score = 0.0) {
	OptimizedGraph G = G_in;
	OptimizedGeneticController controller;
	controller.init(G, team_count, population_size, T_start0, T_end0);
	FOR (i, 50) {
		controller.step();
	}
	ll stagnation = 0;
	ld best_score = 1e18, previous_score = 1e18;
	FOR (i, generations) {
		controller.step_and_prune();
		ld score = controller.population[0].G.score;
		if (score < best_score) {
			best_score = score;
			G = controller.population[0].G;
			if (best_score < target_score + 1e-3) {
				break;
			}
		}
		if (i % 200 == 0) {
			cout << "Generation " << i << " best score " << best_score << endl;
			if (abs(previous_score - best_score) < 0.1) {
				break;
			}
			previous_score = best_score;
		}
	}
	return G;
}

// OptimizedGraph optimized_randomized_genetic_algorithm(OptimizedGraph &G_in, ll team_count, ll population_size, ll generation)

int main() {
	srand(time(0));
	vector<Result> results = read_queue();
	FORE (result, results) {
		cout << "Solving " << result.size << result.id << " with target score " << result.best_score << endl << endl;
		ll team_count = max_teams(result.best_score);
		// while (team_count >= 2) {
		// 	Graph G;
		// 	read_graph(G, result.size, result.id, "focus");
		// 	G = genetic_algorithm(G, team_count, 20, 300, 0.1, result.best_score);
		// 	write_output(G);
		// 	if (get_score(G) < result.best_score) {
		// 		break;
		// 	}
		// 	team_count--;
		// }

		OptimizedGraph G;
		ld previous_score = INF;
		optimized_read_graph(G, result.size, result.id, "optimized");
		while (team_count >= 2) {
			cout << "Trying " << team_count << " teams" << endl;
			G.T = team_count;
			G.team_counts = vector<ll>(team_count, 0);
			G.B_vec = vector<ld>(team_count, 0);
			G = optimized_algorithm(G, team_count, 120, 10000, 1000, 950);
			optimized_write_output(G);
			if (G.score < result.best_score + 1e-3) {
				cout << "Found better score" << endl;
			} elif (G.score > previous_score) {
				cout << "Score increased, stopping" << endl << endl;
				break;
			}
			cout << endl;
			team_count--;
			previous_score = G.score;
		}

		// OptimizedGraph G;
		// optimized_read_best_graph(G, result.size, result.id, "worst_fixed");
		// G = optimized_algorithm(G, team_count, 100, 500, 10, 9.5, 10, result.best_score);
		// optimized_write_output(G);
		// if (G.score < result.best_score + 1e-3) {
		// 	cout << "Found better score " << G.score << ", beating " << result.best_score << endl;
		// }

		// OptimizedGraph G;
		// optimized_read_graph(G, result.size, result.id, "optimized_coloring");
		// G.T = 2;
		// G.team_counts = vector<ll>(G.T);
		// G.B_vec = vector<ld>(G.T);
		// G = optimized_coloring_threshold(G);
		// optimized_write_output(G);

		// break;
	}
	return 0;
}