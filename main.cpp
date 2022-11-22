#include "extras.cpp"

OptimizedGraph optimized_coloring_threshold(OptimizedGraph G) {
	G.score = INF;
	FOR (threshold, 10) {
		OptimizedGraph G_copy = G;
		FOR (i, G_copy.invariant->V) {
			G_copy.node_teams[i] = -1;
		}
		vector<ll> shuffled(G_copy.invariant->V);
		iota(all(shuffled), 0);
		sshuffle(shuffled);
		FOR (i, G_copy.invariant->V) {
			set<short> neighbors;
			FOR (j, G_copy.invariant->V) {
				if (G_copy.invariant->weights[i][j] > threshold) {
					neighbors.insert(G_copy.node_teams[j]);
				}
			}
			ll team = 0;
			vector<ll> possible_teams;
			while (team < G_copy.invariant->T) {
				if (neighbors.find(team) == neighbors.end()) {
					possible_teams.pb(team);
				}
				team++;
			}
			if (possible_teams.empty()) {
				team = G_copy.invariant->T;
			} else {
				team = possible_teams[0];
				FOR (j, sz(possible_teams)) {
					if (G_copy.team_counts[possible_teams[j]] < G_copy.team_counts[team]) {
						team = possible_teams[j];
					}
				}
			}
			G_copy.node_teams[i] = team;
			if (team == G_copy.invariant->T) {
				G_copy.invariant = G_copy.invariant->change_T(G_copy.invariant->T + 1);
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

OptimizedGraph optimized_random_assignment(OptimizedGraph G, short team_count) {
	init_teams(G, team_count);
	FOR (i, G.invariant->V) {
		G.node_teams[i] = rand() % team_count;
		G.team_counts[G.node_teams[i]]++;
	}

	return G;
}

struct OptimizedAnnealingAgent {
	OptimizedGraph G;
	ld T;
	void init(const OptimizedGraph &G_in, ld T0) {
		G = G_in;
		T = T0;
	}

	void step() {
		short node = rand() % G.invariant->V;
		ch old_team = G.node_teams[node];
		ch new_team = rand() % G.invariant->T;
		while (new_team == old_team) {
			new_team = rand() % G.invariant->T;
		}
		ld C_w, B, B_norm_squared, B_old, B_new;
		tie(C_w, B, B_norm_squared, B_old, B_new) = optimized_update_score(G, node, old_team, new_team);
		ld new_score = C_w + G.K + B;
		if (new_score < G.score) {
			G.node_teams[node] = new_team;
			G.team_counts[old_team]--;
			G.team_counts[new_team]++;
			G.B_vec[old_team] = B_old;
			G.B_vec[new_team] = B_new;
			G.score = new_score;
			G.C_w = C_w;
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
		short node = rand() % (ll) G.invariant->V;
		ch old_team = G.node_teams[node];
		ch new_team = rand() % (ll) G.invariant->T;
		while (new_team == old_team) {
			new_team = rand() % (ll) G.invariant->T;
		}
		ld C_w, B, B_norm_squared, B_old, B_new;
		tie(C_w, B, B_norm_squared, B_old, B_new) = optimized_update_score(G, node, old_team, new_team);
		ld new_score = C_w + G.K + B;
		if (new_score < G.score) {
			G.node_teams[node] = new_team;
			G.team_counts[old_team]--;
			G.team_counts[new_team]++;
			G.B_vec[old_team] = B_old;
			G.B_vec[new_team] = B_new;
			G.score = new_score;
			G.C_w = C_w;
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
		ch team_count = 0;
		vector<short> team_pick_node(G.invariant->T, -1);
		while (team_count < batch_size) {
			ch team = rand() % (ch) G.invariant->T;
			if (team_pick_node[team] == -1) {
				continue;
			}
			team_pick_node[team] = 1;
			team_count++;
		}
		FOR (i, G.invariant->T) {
			if (team_pick_node[i] == 1) {
				team_pick_node[i] = rand() % (short) G.team_counts[i];
			}
		}
		vector<short> nodes;
		vector<ch> old_teams;
		FOR (i, G.invariant->V) {
			if (team_pick_node[G.node_teams[i]] == 0) {
				nodes.pb(i);
				old_teams.pb(G.node_teams[i]);
			}
			team_pick_node[G.node_teams[i]]--;
		}
		assert(sz(nodes) == batch_size);
		vector<ch> new_teams = old_teams;
		sshuffle(new_teams);
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

struct OptimizedBlacksmithController {
	vector<OptimizedAnnealingAgent> population;
	ld T_start, T_end;
	void init(OptimizedGraph &G_in, ll team_count, ll population_size, ld T_start0, ld T_end0, bool randomize = false) {
		population = vector<OptimizedAnnealingAgent>(population_size);
		T_start = T_start0;
		T_end = T_end0;
		if (randomize) {
			FOR (i, population_size) {
				population[i] = {optimized_random_assignment(G_in, team_count), T_start};
				optimized_get_score(population[i].G);
			}
		} else {
			FOR (i, population_size) {
				population[i] = {G_in, T_start};
				optimized_get_score(population[i].G);
			}
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

OptimizedGraph optimized_annealing_algorithm(OptimizedGraph &G_in, ll team_count, ll population_size, ll generations, ld T_start0, ld T_end0, ll stagnation_limit = 10, ld target_score = 0.0) {
	OptimizedGraph G = G_in;
	OptimizedBlacksmithController controller;
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

struct OptimizedShepardAgent {
	vector<OptimizedGraph> population;

};

OptimizedGraph optimized_genetic_algorithm(OptimizedGraph &G_in, ll team_count, ll population_size, ll generation) {
	OptimizedGraph G = G_in;
	
}

int main() {
	srand(time(0));
	vector<Result> results = read_queue();
	auto start = chrono::high_resolution_clock::now();
	FORE (result, results) {
		cout << "Solving " << result.size << result.id << " with target score " << result.best_score << endl << endl;
		ch team_count = max_teams(result.best_score);
		short population_sz = 1000 / (short) team_count;
		OptimizedGraph G;
		ld previous_score = INF;
		optimized_read_graph(G, result.size, result.id, "hyper_optimized");
		while (team_count >= 2) {
			cout << "Trying " << (ll) team_count << " teams" << endl;
			init_teams(G, team_count);
			G = optimized_annealing_algorithm(G, team_count, population_sz, 10000, 1000, 950);
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
		auto end = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::seconds>(end - start);
		cout << "Time elapsed: " << duration.count() << " seconds" << endl << endl;
	}
	return 0;
}