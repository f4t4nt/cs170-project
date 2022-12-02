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
	void stepSingle() {
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
	void stepSwap() {
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
	void init(OptimizedGraph &G_in, ll team_count, ll population_size, ld T_start0, ld T_end0, vector<OptimizedGraph> Gs = {}, bool preserve_dist = false) {
		population = vector<OptimizedAnnealingAgent>(population_size);
		T_start = T_start0;
		T_end = T_end0;
		if (sz(Gs) == 0) {
			if (preserve_dist) {
				FOR (i, population_size) {
					population[i] = {optimized_random_assignment(G_in, team_count), T_start};
					optimized_get_score(population[i].G);
				}
			} else {
				FOR (i, population_size) {
					population[i] = {G_in, 0};
					FOR (_, 10000) {
						short node1 = rand() % population[i].G.invariant->V;
						short node2 = rand() % population[i].G.invariant->V;
						swap(population[i].G.node_teams[node1], population[i].G.node_teams[node2]);
					}
					optimized_get_score(population[i].G);
				}
			}
		} else {
			FOR (i, population_size) {
				population[i] = {Gs[i % sz(Gs)], T_start};
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
				FOR (j, 250) {
					agent.stepSingle();
					agent.stepSwap();
				}
				agent.T *= 0.999;
			}
		}
		T_start *= 0.99;
		T_end *= 0.99;
	}
	void step_and_prune() {
		step();
		sort(all(population), [](OptimizedAnnealingAgent &a, OptimizedAnnealingAgent &b) {
			return a.G.score < b.G.score;
		});
		ll half = sz(population) / 2;
		FOB (i, half, sz(population)) {
			population[i] = population[rand() % half];
			ll other;
			do {
				other = rand() % half;
			} while (population[other].G.invariant->T != population[i].G.invariant->T);
			if (other != i) {
				optimized_cross(population[i].G, population[rand() % half].G);
			}
		}
	}
};

OptimizedGraph optimize(
			OptimizedGraph &G,
			ll team_count,
			ll population_size,
			ll generations,
			ll idle_steps,
			ld T_start0,
			ld T_end0,
			ld target_score = 0,
			short stagnation_limit = 10,
			ld ignition_factor = 100,
			bool search_deltas = false,
			vector<OptimizedGraph> Gs = {}
		) {
	G.score = INF;
	bool extended = true;
	ll stagnation = 0;
	ld best_score = 1e18, previous_score = 1e18;
	OptimizedBlacksmithController smith;
	smith.init(G, team_count, population_size, 50, 40, Gs);
	FOR (i, idle_steps) {
		smith.step();
	}
	smith.step_and_prune();
	smith.T_start = T_start0;
	smith.T_end = T_end0;
	ll batch_steps = (idle_steps + 1) * 5;
	FOR (i, generations) {
		smith.step_and_prune();
		auto population_best = smith.population[0].G;
		if (population_best.score < best_score) {
			best_score = population_best.score;
			if (population_best.score < G.score) {
				G = population_best;
			}
		}
		if (i % batch_steps == 0) {
			optimized_write_output(G);
			cout << "Generation " << i << " best score (" << G.score << " | " << best_score << ") and worst score " << smith.population[population_size / 2 - 1].G.score << ", temperature " << smith.T_start << endl;
			if (search_deltas) {
				FOR (j, population_size) {
					auto &agent = smith.population[j];
					if (round(agent.G.score - target_score) == agent.G.score - target_score) {
						cout << "Found a score with an integer delta score" << endl;
						optimized_write_output(agent.G, true);
						return agent.G;
					}
				}
			}
			if (previous_score == best_score || smith.T_start < 1e-2) {
				stagnation++;
				if (stagnation >= stagnation_limit) {
					cout << "Stagnation limit reached, terminating" << endl;
					break;
				}
				cout << "Stagnation " << stagnation << ", firing up furnace" << endl;
				if (smith.T_start < T_start0 / ignition_factor) {
					smith.T_start *= ignition_factor;
					smith.T_end *= ignition_factor;
					cout << "Temperature set to " << smith.T_start << endl;
				} else {
					smith.init(G, team_count, population_size, T_start0, T_end0);
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
		FOR (j, idle_steps) {
			smith.step();
		}
		i += idle_steps;
	}
	return G;
}

void final_solve(Result &result, ld target_score) {
	vector<OptimizedGraph> Gs, Gs_;
	OptimizedGraph G;
	short population_sz = 4096;
	optimized_read_graph(G, result.size, result.id, "boss_battle");
	Gs = optimized_read_local_graphs(G, result.size, result.id, "boss_battle");
	cout << "Final solving " << result.size << result.id << " with population size " << population_sz << endl << endl;
	map<ll, ll> team_sizes;
	ll elite = min((ll) sz(Gs), 128ll), idx = 0, int_deltas = 0;
	while (idx < elite) {
		team_sizes[Gs[idx].invariant->T]++;
		Gs_.pb(Gs[idx]);
		idx++;
	}
	while (idx < sz(Gs) && sz(team_sizes) < 5) {
		if (team_sizes.find(Gs[idx].invariant->T) == team_sizes.end() ||
			team_sizes[Gs[idx].invariant->T] < 8) {
			team_sizes[Gs[idx].invariant->T]++;
			Gs_.pb(Gs[idx]);
		} elif (Gs[idx].score - target_score == round(Gs[idx].score - target_score)) {
			Gs_.pb(Gs[idx]);
			team_sizes[Gs[idx].invariant->T]++;
			int_deltas++;
		}
		idx++;
	}
	while (idx < sz(Gs)) {
		if (Gs[idx].score - target_score == round(Gs[idx].score - target_score)) {
			Gs_.pb(Gs[idx]);
			team_sizes[Gs[idx].invariant->T]++;
			int_deltas++;
		}
		idx++;
	}
	Gs = Gs_;
	cout << "Using " << sz(Gs) << " graphs" << endl;
	FORE (p, team_sizes) {
		cout << "Team size " << p.first << ": " << p.second << endl;
	}
	cout << endl;
	cout << "Integer deltas: " << int_deltas << endl;
	cout << endl;
	cout << "Target score " << target_score << endl;
	cout << "Best score: " << Gs[0].score << endl;
	cout << "Worst score: " << Gs.back().score << endl;
	cout << endl;
	ld T_start = 1000, T_end = 950, ignition_factor = 100;
	ll idle_steps = 19, generations = 1000000, stagnation_limit = 16;
	bool search_deltas = true;
	G = optimize(G, Gs[0].invariant->T, population_sz, generations, idle_steps, T_start, T_end, target_score, stagnation_limit, ignition_factor, search_deltas, Gs);
	if (G.score <= target_score + 1e-9) {
		cout << "Target score reached" << endl;
	} elif (G.score < result.local_score) {
		cout << "Local score beat" << endl;
	}
	cout << endl;
	optimized_write_output(G);
}

int main() {
	srand(time(NULL));
	vector<Result> results = read_queue();
	auto start = chrono::high_resolution_clock::now();
	while (true) {
		FORE (result, results) {
			final_solve(result, result.best_score);
			auto end = chrono::high_resolution_clock::now();
			auto duration = chrono::duration_cast<chrono::seconds>(end - start);
			cout << "Time elapsed: " << duration.count() << " seconds" << endl << endl;
		}
		cout << "Restarting" << endl << endl;
	}
	return 0;
}