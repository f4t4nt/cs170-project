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
					if (G_copy.team_sizes[possible_teams[j]] < G_copy.team_sizes[team]) {
						team = possible_teams[j];
					}
				}
			}
			G_copy.node_teams[i] = team;
			if (team == G_copy.invariant->T) {
				G_copy.invariant = G_copy.invariant->change_T(G_copy.invariant->T + 1);
				G_copy.team_sizes.pb(1);
				G_copy.B_vec.pb(0);
			} else {
				G_copy.team_sizes[team]++;
			}
		}
		optimized_get_score(G_copy);
		if (G_copy.score < G.score) {
			G = G_copy;
		}
	}
	return G;
}

OptimizedGraph optimized_random_assignment(OptimizedGraph &G_in, short team_count) {
	OptimizedGraph G = G_in;
	init_teams(G, team_count);
	FOR (i, G.invariant->V) {
		G.node_teams[i] = rand() % team_count;
		G.team_sizes[G.node_teams[i]]++;
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
			G.team_sizes[old_team]--;
			G.team_sizes[new_team]++;
			G.B_vec[old_team] = B_old;
			G.B_vec[new_team] = B_new;
			G.score = new_score;
			G.C_w = C_w;
			G.B_norm_squared = B_norm_squared;
			return;
		}
		ld p = exp((G.score - new_score) / T);
		if (rand() < p * 32767) {
			G.node_teams[node] = new_team;
			G.team_sizes[old_team]--;
			G.team_sizes[new_team]++;
			G.B_vec[old_team] = B_old;
			G.B_vec[new_team] = B_new;
			G.score = new_score;
			G.C_w = C_w;
			G.B_norm_squared = B_norm_squared;
			return;
		}
	}
};


struct OptimizedAnnealingSwapAgent {
	OptimizedGraph G;
	ld T;
	void init(const OptimizedGraph &G_in, ld T0) {
		G = G_in;
		T = T0;
	}
	void step() {
		short node1 = rand() % G.invariant->V;
		short node2 = rand() % G.invariant->V;
		while (node1 == node2 || G.node_teams[node1] == G.node_teams[node2]) {
			node2 = rand() % G.invariant->V;
		}
		ld C_w = optimized_update_swap_score(G, node1, node2);
		ld new_score = C_w + G.K + exp(B_EXP * sqrt(G.B_norm_squared));
		if (new_score < G.score) {
			swap(G.node_teams[node1], G.node_teams[node2]);
			G.score = new_score;
			G.C_w = C_w;
			return;
		}
		ld p = exp((G.score - new_score) / T);
		if (rand() < p * 32767) {
			swap(G.node_teams[node1], G.node_teams[node2]);
			G.score = new_score;
			G.C_w = C_w;
			return;
		}
	}
};

struct OptimizedBlacksmithController {
	vector<OptimizedAnnealingAgent> population;
	ld T_start, T_end;
	void init(OptimizedGraph &G_in, ll team_count, ll population_size, ld T_start0, ld T_end0, bool randomize = true) {
		population = vector<OptimizedAnnealingAgent>(population_size);
		T_start = T_start0;
		T_end = T_end0;
		if (randomize) {
			FOR (i, population_size) {
				population[i] = {optimized_random_assignment(G_in, team_count), T_start};
				optimized_get_score(population[i].G);
			}
		} else {
			optimized_get_score(G_in);
			FOR (i, population_size) {
				population[i] = {G_in, T_start};
			}
		}
	}
	void step() {
		auto population_size = sz(population);
		#pragma omp parallel for
		for (size_t i = 0; i < population_size; i++) {
			auto &agent = population[i];
			agent.T = T_start;
			while (agent.T > T_end) {
				FOR (j, 500) {
					agent.step();
				}
				agent.T *= 0.999;
			}
		}
		T_start *= 0.996;
		T_end *= 0.996;
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

struct OptimizedBlacksmithSwapController {
	vector<OptimizedAnnealingSwapAgent> population;
	ld T_start, T_end;
	void init(OptimizedGraph &G_in, ll team_count, ll population_size, ld T_start0, ld T_end0, bool randomize = true) {
		population = vector<OptimizedAnnealingSwapAgent>(population_size);
		T_start = T_start0;
		T_end = T_end0;
		if (randomize && find(G_in.team_sizes.begin(), G_in.team_sizes.end(), -1) != G_in.team_sizes.end()) {
			FOR (i, population_size) {
				population[i] = {optimized_random_assignment(G_in, team_count), T_start};
				optimized_get_score(population[i].G);
			}
		} elif (randomize) {
			FOR (i, population_size) {
				population[i] = {G_in, 0};
				FOR (_, 1000) {
					short node1 = rand() % population[i].G.invariant->V;
					short node2 = rand() % population[i].G.invariant->V;
					swap(population[i].G.node_teams[node1], population[i].G.node_teams[node2]);
				}
				optimized_get_score(population[i].G);
			}
		} else {
			optimized_get_score(G_in);
			FOR (i, population_size) {
				population[i] = {G_in, T_start};
			}
		}
	}
	void step() {
		auto population_size = sz(population);
		#pragma omp parallel for
		for (size_t i = 0; i < population_size; i++) {
			auto &agent = population[i];
			agent.T = T_start;
			while (agent.T > T_end) {
				FOR (j, 500) {
					agent.step();
				}
				agent.T *= 0.999;
			}
		}
		T_start *= 0.993;
		T_end *= 0.993;
	}
	void step_and_prune() {
		step();
		sort(all(population), [](OptimizedAnnealingSwapAgent &a, OptimizedAnnealingSwapAgent &b) {
			return a.G.score < b.G.score;
		});
		FOR (i, sz(population) / 2) {
			population[i + sz(population) / 2] = population[i];
		}
	}
};

OptimizedGraph optimized_annealing_algorithm(OptimizedGraph &G, ll team_count, ll population_size, ll generations, ld T_start0, ld T_end0, bool randomize = true, ld target_score = 0, short stagnation_limit = 1, ld ignition_factor = 1.1, bool swaps = false, bool search_deltas = false) {
	G.score = INF;
	bool extended = true;
	ll stagnation = 0;
	ld best_score = 1e18, previous_score = 1e18;
	if (!swaps) {
		cout << "Regular solve confirmed" << endl;
		OptimizedBlacksmithController shepard;
		shepard.init(G, team_count, population_size, T_start0, T_end0, randomize);
		FOR (i, 50) {
			shepard.step();
		}
		FOR (i, generations) {
			shepard.step_and_prune();
			auto population_best = shepard.population[0].G;
			if (population_best.score < best_score) {
				best_score = population_best.score;
				if (population_best.score < G.score) {
					G = population_best;
				}
			}
			if (i % 100 == 0) {
				optimized_write_output(G);
				cout << "Generation " << i << " best score (" << G.score << " | " << best_score << "), temperature " << shepard.T_start << endl;
				if (search_deltas) {
					FOR (j, population_size) {
						auto &agent = shepard.population[j];
						if (round(agent.G.score - target_score) == agent.G.score - target_score) {
							cout << "Found a score with an integer delta with the target score!" << endl;
							optimized_write_output(agent.G, true);
							return agent.G;
						}
					}
				}
				if (previous_score == best_score || shepard.T_start < 1e-2) {
					stagnation++;
					if (stagnation >= stagnation_limit) {
						cout << "Stagnation limit reached, terminating" << endl;
						break;
					}
					if (shepard.T_start < T_start0 / ignition_factor) {
						shepard.T_start *= ignition_factor;
						shepard.T_end *= ignition_factor;
						cout << "Reigniting, temperature set to " << shepard.T_start << endl;
					} else {
						shepard.init(G, team_count, population_size, T_start0, T_end0);
						cout << "Ionizing" << endl;
					}
					best_score = 1e18;
				}
				if (G.score <= target_score && extended) {
					stagnation_limit++;
					extended = false;
				}
				previous_score = best_score;
			}
			FOR (j, 19) {
				shepard.step();
			}
			i += 19;
		}
	} else {
		cout << "Swap solve confirmed" << endl;
		OptimizedBlacksmithSwapController shepard;
		shepard.init(G, team_count, population_size, T_start0, T_end0, randomize);
		FOR (i, 50) {
			shepard.step();
		}
		FOR (i, generations) {
			shepard.step_and_prune();
			auto population_best = shepard.population[0].G;
			if (population_best.score < best_score) {
				best_score = population_best.score;
				if (population_best.score < G.score) {
					G = population_best;
				}
			}
			if (i % 200 == 0) {
				optimized_write_output(G);
				cout << "Generation " << i << " best score (" << G.score << " | " << best_score << "), temperature " << shepard.T_start << endl;
				if (previous_score == best_score || shepard.T_start < 1e-2) {
					stagnation++;
					if (stagnation >= stagnation_limit) {
						cout << "Stagnation limit reached, terminating" << endl;
						break;
					}
					if (shepard.T_start < T_start0 / ignition_factor) {
						shepard.T_start *= ignition_factor;
						shepard.T_end *= ignition_factor;
						cout << "Reigniting, temperature set to " << shepard.T_start << endl;
					} else {
						shepard.init(G, team_count, population_size, T_start0, T_end0);
						cout << "Ionizing" << endl;
					}
					best_score = 1e18;
				}
				if (G.score <= target_score && extended) {
					stagnation_limit++;
					extended = false;
				}
				previous_score = best_score;
			}
			FOR (j, 19) {
				shepard.step();
			}
			i += 19;
		}
	}
	return G;
}

void assume_team_range(Result &result, ld target_score, ch team_max, ch team_min = 2) {
	OptimizedGraph G;
	short population_sz = 5000;
	cout << "Rigorously solving " << result.size << result.id << " with target score " << target_score << " and population size " << population_sz << endl << endl;
	ch team_count = team_max;
	ld previous_score = INF;
	optimized_read_graph(G, result.size, result.id, "rank_1");
	while (team_count >= team_min) {
		cout << "Trying " << (ll) team_count << " teams" << endl;
		init_teams(G, team_count);
		G = optimized_annealing_algorithm(G, team_count, population_sz, 100000, 2000, 1900, true, target_score, 5, 200, false, true);
		optimized_write_output(G);
		if (G.score < target_score + 1e-9) {
			cout << "Target score reached" << endl;
		} elif (G.score < result.local_score) {
			cout << "Local score beat" << endl;
		}
		cout << endl;
		if (G.score > previous_score) {
			break;
		}
		team_count--;
	}
}

void rigorous_solve(Result &result, ld target_score) {
	OptimizedGraph G;
	short population_sz = 4000;
	cout << "Rigorously solving " << result.size << result.id << " with target score " << target_score << " and population size " << population_sz << endl << endl;
	ch team_count = max_teams(result.best_score);
	ld previous_score = INF;
	// optimized_read_graph(G, result.size, result.id, "mindstormX");
	optimized_read_best_graph(G, result.size, result.id, "rank_1");
	team_count = min((ch) (G.invariant->T + 1), team_count);
	short increase_limit = 1;
	while (team_count >= 2) {
		cout << "Trying " << (ll) team_count << " teams" << endl;
		init_teams(G, team_count);
		G = optimized_annealing_algorithm(G, team_count, population_sz, 20000, 2000, 1900, true, target_score, 3, 100);
		optimized_write_output(G);
		if (G.score < target_score + 1e-9) {
			cout << "Target score reached" << endl;
		} elif (G.score < result.local_score) {
			cout << "Local score beat" << endl;
		}
		if (G.score > previous_score) {
			increase_limit--;
			if (increase_limit == 0) {
				cout << "Increase limit reached, terminating" << endl;
				break;
			}
		}
		cout << endl;
		team_count--;
		previous_score = G.score;
	}
}

void swap_solve(Result &result, ld target_score) {
	OptimizedGraph G;
	short population_sz = 1024;
	optimized_read_best_graph(G, result.size, result.id, "rank_1");
	cout << "Rigorously swap solving " << result.size << result.id << " with target score " << target_score << " and population size " << population_sz << endl << endl;
	G = optimized_annealing_algorithm(G, G.invariant->T, population_sz, 10000, 1000, 950, false, target_score, 1, 100, true);
	optimized_write_output(G);
	if (G.score <= result.best_score) {
		cout << "Target score reached" << endl;
	} elif (G.score < result.local_score) {
		cout << "Local score beat" << endl;
	}
}

void find_swap_solve(Result &result, ld target_score) {
	vector<OptimizedGraph> Gs;
	OptimizedGraph G;
	short population_sz = 4000;
	optimized_read_graph(G, result.size, result.id, "rank_1");
	Gs = optimized_read_local_graphs(G, result.size, result.id, "rank_1");
	cout << "Rigorously find-swap solving " << result.size << result.id << " with target score " << target_score << " and population size " << population_sz << endl << endl;
	ll idx = 0;
	while (idx < Gs.size() && Gs[idx].score - target_score != round(Gs[idx].score - target_score)) {
		idx++;
	}
	if (idx == Gs.size()) {
		cout << "No graph with integer delta score found" << endl;
		// find_distribution(target_score, G.invariant->V, 13);
		rigorous_solve(result, target_score);
		return;
	}
	cout << "Found graph with integer delta score" << endl;
	G = Gs[idx];
	G = optimized_annealing_algorithm(G, G.invariant->T, population_sz, 10000, 2000, 1900, false, target_score, 1, 300, true);
	optimized_write_output(G);
	if (G.score <= result.best_score) {
		cout << "Target score reached" << endl;
	} elif (G.score < result.local_score) {
		cout << "Local score beat" << endl;
	}
}

void improve_existing(Result &result) {
	OptimizedGraph G;
	short population_sz = 128;
	optimized_read_best_graph(G, result.size, result.id, "rank_1");
	cout << "Improving " << result.size << result.id << " best score " << result.best_score << ", current score " << optimized_get_score(G) << " with population size " << population_sz << endl << endl;
	ld T_start = max((ld) 200, result.delta_score);
	G = optimized_annealing_algorithm(G, G.invariant->T, population_sz, 100000, 2000, 1900, true, result.best_score, 5, 200, false, true);
	optimized_write_output(G);
	if (G.score <= result.best_score) {
		cout << "Target score reached" << endl;
	} elif (G.score < result.local_score) {
		cout << "Local score beat" << endl;
	}
}

int main() {
	srand(time(NULL));
	vector<Result> results = read_queue();
	auto start = chrono::high_resolution_clock::now();
	while (true) {
		FORE (result, results) {
			improve_existing(result);
			auto end = chrono::high_resolution_clock::now();
			auto duration = chrono::duration_cast<chrono::seconds>(end - start);
			cout << "Time elapsed: " << duration.count() << " seconds" << endl << endl;
		}
		cout << "Restarting" << endl << endl;
	}
	return 0;
}