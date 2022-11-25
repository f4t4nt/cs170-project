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

OptimizedGraph optimized_random_assignment(OptimizedGraph &G_in, short team_count) {
	OptimizedGraph G = G_in;
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
		if (team_count >= G.invariant->T) {
			team_pick_node = vector<short>(G.invariant->T, 1);
		} else {
			while (team_count < batch_size) {
				ch team = rand() % (ch) G.invariant->T;
				if (team_pick_node[team] == 1) {
					continue;
				}
				team_pick_node[team] = 1;
				team_count++;
			}
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
				if (old_teams[i] != new_teams[i]) {
					G.node_teams[nodes[i]] = new_teams[i];
					G.team_counts[old_teams[i]]--;
					G.team_counts[new_teams[i]]++;
				}
			}
			G.C_w = C_w;
			G.score = C_w + G.K + exp(B_EXP * sqrt(G.B_norm_squared));
			return;
		}
		ld p = exp((G.C_w - C_w) / T);
		if (rand() % 1000000 < p * 1000000) {
			FOR (i, batch_size) {
				if (old_teams[i] != new_teams[i]) {
					G.node_teams[nodes[i]] = new_teams[i];
					G.team_counts[old_teams[i]]--;
					G.team_counts[new_teams[i]]++;
				}
			}
			G.C_w = C_w;
			G.score = C_w + G.K + exp(B_EXP * sqrt(G.B_norm_squared));
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
			FOR (i, population_size) {
				population[i] = {G_in, T_start};
				optimized_get_score(population[i].G);
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

OptimizedGraph optimized_annealing_algorithm(OptimizedGraph &G, ll team_count, ll population_size, ll generations, ld T_start0, ld T_end0, bool randomize = true, ld target_score = 0, short stagnation_limit = 1, ld ignition_factor = 1.1) {
	G.score = INF;
	OptimizedBlacksmithController shepard;
	shepard.init(G, team_count, population_size, T_start0, T_end0, randomize);
	FOR (i, 50) {
		shepard.step();
	}
	bool extended = true;
	ll stagnation = 0;
	ld best_score = 1e18, previous_score = 1e18;
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
			if (previous_score == best_score || shepard.T_start < 1e-3) {
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
	}
	return G;
}

struct OptimizedShepardAgent {
	short V;
	ch T;
	ld mutation_rate;
	unsigned int population_size, mutations, cross_overs;
	ld variation = 0.0;
	vector<OptimizedGraph> population;
	void init(OptimizedGraph &G_in, ll team_count, ll population_size, ld mutation_rate, bool randomize = true) {
		this->population_size = population_size;
		this->mutation_rate = mutation_rate * 1e6;
		mutations = population_size * 1000;
		cross_overs = population_size * 500;
		population = vector<OptimizedGraph>(population_size + mutations + cross_overs);
		if (randomize) {
			FOR (i, population_size * 20) {
				population[i] = optimized_random_assignment(G_in, team_count);
				optimized_get_score(population[i]);
			}
		} else {
			ld score = optimized_get_score(G_in);
			FOR (i, population_size) {
				population[i] = G_in;
				population[i].score = score;
			}
		}
		sort(population.begin(), population.begin() + population_size * 20, [](OptimizedGraph &a, OptimizedGraph &b) {
			return a.score < b.score;
		});
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
		if (mutation_rate > 4e5) {
			FOR (node, V) {
				if (rand2() % 1000000 < mutation_rate) {
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
		} else {
			FOR (node, V) {
				if (rand2() % 1000000 < mutation_rate) {
					ch old_team = G.node_teams[node];
					ch new_team = rand() % T;
					while (new_team == old_team) {
						new_team = rand() % T;
					}
					tie(G.C_w, G.score, G.B_norm_squared, G.B_vec[old_team], G.B_vec[new_team]) = optimized_update_score(G, node, old_team, new_team);
					G.score += G.C_w + G.K;
					G.node_teams[node] = new_team;
					G.team_counts[old_team]--;
					G.team_counts[new_team]++;
				}
			}
		}
		ll shift = rand() % T;
		FOR (node, V) {
			G.node_teams[node] = (G.node_teams[node] + shift) % T;
		}
		rotate(G.node_teams.begin(), G.node_teams.begin() + shift, G.node_teams.end());
		G.score += shift * 1e-3;
		// vector<ch> mapping(T);
		// iota(mapping.begin(), mapping.end(), 0);
		// sshuffle(mapping);
		// vector<short> team_counts = G.team_counts;
		// FOR (team, T) {
		// 	G.team_counts[team] = team_counts[mapping[team]];
		// }
		// FOR (node, V) {
		// 	G.node_teams[node] = mapping[G.node_teams[node]];
		// }
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
		sort(all(population), [](const OptimizedGraph &a, const OptimizedGraph &b) {
			return a.score < b.score;
		});
		size_t i = 1, j = 0, k = 1;
		for (; i < population_size; i++) {
			while (population[j].score == population[k].score) {
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
		mutation_rate = max((ld) 4e5, 0.99 * mutation_rate);
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
		shepard.step();
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
				OptimizedAnnealingAgent agent;
				FOR (j, population_size) {
					agent.init(shepard.population[j], 1000);
					FOR (k, 1000) {
						agent.step();
						agent.T *= 0.99;
					}
					shepard.population[j] = agent.G;
				}
				cout << "Annealed population" << endl;
			} else {
				stagnation = 0;
			}
			if (G.score <= target_score && extended) {
				stagnation_limit++;
				extended = false;
			}
			previous_score = G.score;
		}
	}
	return G;
}

void rigorous_solve(Result &result, ld target_score) {
	OptimizedGraph G;
	short population_sz = 256;
	cout << "Rigorously solving " << result.size << result.id << " with target score " << target_score << " and population size " << population_sz << endl << endl;
	ch team_count = max_teams(result.best_score);
	ld previous_score = INF;
	optimized_read_graph(G, result.size, result.id, "sick2");
	short increase_limit = 2;
	while (team_count >= 2) {
		cout << "Trying " << (ll) team_count << " teams" << endl;
		init_teams(G, team_count);
		G = optimized_annealing_algorithm(G, team_count, population_sz, 10000, 1000, 950, true, target_score, 3, 100);
		optimized_write_output(G);
		if (G.score < target_score + 1e-9) {
			cout << "Target score reached, terminating" << endl;
		} elif (G.score > previous_score) {
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

void improve_existing(Result &result) {
	OptimizedGraph G;
	short population_sz = 512;
	optimized_read_best_graph(G, result.size, result.id, "sick2");
	cout << "Improving " << result.size << result.id << ", current score " << optimized_get_score(G) << " with population size " << population_sz << endl << endl;
	G = optimized_annealing_algorithm(G, G.invariant->T, population_sz, 4000, 100, 95, false, 0.0, result.rank + 1, 1000);
	optimized_write_output(G);
	if (G.score < result.best_score) {
		cout << "Found better score" << endl;
	}
}

void dolly_solve(Result &result, ld target_score) {
	OptimizedGraph G;
	short population_sz = 1024;
	cout << "Dollying " << result.size << result.id << " with target score " << target_score << " and population size " << population_sz << endl << endl;
	ch team_count = max_teams(result.best_score);
	team_count = 13;
	ld previous_score = INF;
	optimized_read_graph(G, result.size, result.id, "sick2");
	while (team_count >= 2) {
		cout << "Trying " << (ll) team_count << " teams" << endl;
		init_teams(G, team_count);
		G = optimized_genetic_algorithm(G, team_count, population_sz, 10000, true, 1, target_score, 3);
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
	short population_sz = 1024;
	cout << "Shearing " << result.size << result.id << " with population size " << population_sz << endl << endl;
	optimized_read_best_graph(G, result.size, result.id, "sick2");
	G = optimized_genetic_algorithm(G, G.invariant->T, population_sz, 10000, false, 1, 0, 3);
	optimized_write_output(G);
	if (G.score < result.best_score) {
		cout << "Found better score" << endl;
	}
}

int main() {
	// srand(time(NULL));
	vector<Result> results = read_queue();
	auto start = chrono::high_resolution_clock::now();
	while (true) {
		FORE (result, results) {
			if (result.delta_score < 1e-9) {
				continue;
			} elif (result.notes == "missing_local") {
				cout << "Missing local" << endl;
				rigorous_solve(result, result.best_score);
				dolly_solve(result, result.best_score);
				dolly_improve(result);
			} elif (result.rank == 1 || result.notes == "sleeper") {
				improve_existing(result);
				dolly_improve(result);
			} else {
				cout << "Dolly the sheep" << endl;
				dolly_solve(result, result.best_score);
			}
			auto end = chrono::high_resolution_clock::now();
			auto duration = chrono::duration_cast<chrono::seconds>(end - start);
			cout << "Time elapsed: " << duration.count() << " seconds" << endl << endl;
		}
		cout << "Restarting" << endl << endl;
	}
	return 0;
}