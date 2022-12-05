#include "extras.cpp"
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

size_t preserve_population_frac = 4;

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
	G.lock_distribution= false;
	return G;
}

struct OptimizedAnnealingAgent {
	OptimizedGraph G;
	ld T;
	void init(const OptimizedGraph &G_in, ld T0) {
		G = G_in;
		T = T0;
	}

	bool stepSingle(int retries = 10, ld start_score = -1) {
		int i = retries;
		while(i--) {
			short node = rand() % G.invariant->V;
			ch old_team = G.node_teams[node];
			ch new_team = rand() % G.invariant->T;
			while (new_team == old_team) {
				new_team = rand() % G.invariant->T;
			}
			ld C_w, B, B_norm_squared, B_old, B_new;
			tie(C_w, B, B_norm_squared, B_old, B_new) = optimized_update_score(G, node, old_team, new_team);
			ld new_score = C_w + G.K + B;
			start_score = start_score < 0 ? G.score : start_score;
			if (new_score < start_score) {
				G.node_teams[node] = new_team;
				G.team_sizes[old_team]--;
				G.team_sizes[new_team]++;
				G.B_vec[old_team] = B_old;
				G.B_vec[new_team] = B_new;
				G.score = new_score;
				G.C_w = C_w;
				G.B_norm_squared = B_norm_squared;
				return true;
			}
			ld p = exp((start_score - new_score) / T);
			if (rand() < p * 32767) {
				G.node_teams[node] = new_team;
				G.team_sizes[old_team]--;
				G.team_sizes[new_team]++;
				G.B_vec[old_team] = B_old;
				G.B_vec[new_team] = B_new;
				G.score = new_score;
				G.C_w = C_w;
				G.B_norm_squared = B_norm_squared;
				return true;
			}
		}

		return false;
	}

	short stepSwap(size_t retries, short node1 = -1, ld start_score = -1) {
		size_t i = retries;
		while(i--) {
			if (node1 == -1) {
				node1 = rand() % G.invariant->V;
			}

			short node2 = rand() % G.invariant->V;
			while (node1 == node2 || G.node_teams[node1] == G.node_teams[node2]) {
				node2 = rand() % G.invariant->V;
			}

			ld C_w = optimized_update_swap_score(G, node1, node2);
			ld new_score = C_w + G.K + exp(B_EXP * sqrt(G.B_norm_squared));
			start_score = start_score < 0 ? G.score : start_score;
			if (new_score < start_score) {
				swap(G.node_teams[node1], G.node_teams[node2]);
				G.score = new_score;
				G.C_w = C_w;
				return node2;
			}

			ld p = exp((start_score - new_score) / T);
			if (rand() < p * 32767) {
				swap(G.node_teams[node1], G.node_teams[node2]);
				G.score = new_score;
				G.C_w = C_w;
				return node2;
			}
		}

		return node1;
	}

	bool stepSwapChain(ll n, size_t swap_retries = 5, ld start_score = -1) {
		bool changed = false;
		short node1 = rand() % G.invariant->V;
		FOR (i, n) {
			auto tmp = stepSwap(swap_retries, node1, start_score);
			if (node1 != tmp) {
				return changed;
			} else {
				changed = true;
			}
		}

		return changed;
	}
};

struct OptimizedBlacksmithController {
	vector<OptimizedAnnealingAgent> population;
	ld T_start, T_end;
	size_t start_mixup = 0;
	void init(
		OptimizedGraph &G_in,
		ll team_count,
		ll population_size,
		ld T_start0,
		ld T_end0,
		vector<OptimizedGraph> Gs = {},
		bool preserve_dist = false
	) {
		population = vector<OptimizedAnnealingAgent>(population_size);
		T_start = T_start0;
		T_end = T_end0;

		if (sz(Gs) == 0) {
			if (!preserve_dist) {
				FOR (i, population_size) {
					if (population[i].G.lock_distribution) {
						FOR (_, 10000) {
							short node1 = rand() % population[i].G.invariant->V;
							short node2 = rand() % population[i].G.invariant->V;
							swap(population[i].G.node_teams[node1], population[i].G.node_teams[node2]);
						}
						optimized_get_score(population[i].G);
					} else {
						population[i] = {optimized_random_assignment(G_in, team_count), T_start};
						optimized_get_score(population[i].G);
					}
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
		for (size_t i = start_mixup; i < population_size; i++) {
			auto &agent = population[i];
			auto start_score = agent.G.score;

			const size_t ld_limit = 100, non_ld_limit = 500,
				ld_fast_limit = 10, non_ld_fast_limit = 100,
				swap_limit = 10, move_limit = 10, fast_swap_limit = 3, fast_move_limit = 3;
			int retries = agent.G.lock_distribution ? 1 : 1;
			while(retries--) {
				agent.T = T_start;
				while (agent.T > T_end) {
					size_t sl = start_score == agent.G.score ? swap_limit : fast_swap_limit;
					if (agent.G.lock_distribution) {
						size_t parts = start_score == agent.G.score ? ld_limit : ld_fast_limit;
						FOR (j, parts) {
							agent.stepSwapChain(sl, swap_limit, start_score);
						}
					} else {
						size_t parts = start_score == agent.G.score ? non_ld_limit : non_ld_fast_limit;
						size_t ml = start_score == agent.G.score ? move_limit : fast_move_limit;
						size_t singles = rand() % parts;
						FOR (j, singles) {
							agent.stepSingle(ml);
						}

						FOB (j, singles, parts) {
							agent.stepSwap(sl, -1, start_score);
						}
					}

					agent.T *= 0.99;
				}

				ld p = exp((start_score - agent.G.score) / (T_start / 2));
				if ((agent.G.score < start_score || rand() < p * 32767) && start_score != agent.G.score) {
					break;
				}
			}

			agent.G.unchanged = agent.G.score == start_score;
		}

		T_start *= 0.995;
		T_end *= 0.995;
	}

	int unchanged = 0;
	void prune(ld target_score = 0) {
		unchanged = 0;
		FOR(i, sz(population)) {
			auto &agent = population[i];
			if (agent.G.unchanged) {
				unchanged++;
			}

			agent.G.lock_distribution = target_score > 0
				&& round(agent.G.score - target_score) == agent.G.score - target_score;
		}

		sort(all(population), [](OptimizedAnnealingAgent &a, OptimizedAnnealingAgent &b) {
			// if (a.G.lock_distribution == b.G.lock_distribution) {
				return a.G.score < b.G.score;
			// } else if (abs(a.G.score - b.G.score) < a.G.score * 0.2) {
			// 	return a.G.lock_distribution > b.G.lock_distribution;
			// } else {
			// 	return a.G.score < b.G.score;
			// }
		});

		ll half = sz(population) / preserve_population_frac;

		for(size_t i = 0, j = 1; i < half - 1 && j < sz(population); ++j) {
			if (population[i].G.score != population[j].G.score) {
				population[++i] = population[j];
			}
		}

		sort(population.begin(), population.begin() + half, [](OptimizedAnnealingAgent &a, OptimizedAnnealingAgent &b) {
			return a.G.score < b.G.score;
		});

		for (size_t i = half; i < sz(population); i++) {
			ll rand_high = rand() << 15 + rand() + 100;
			population[i] = population[half - ((size_t)floor(sqrt(rand_high)) - 10) % half - 1];

			if (T_start < 10 && !population[i].G.lock_distribution && rand() % 100 < 25) {
				ll other;
				do {
					ll rand_high = rand() << 15 + rand() + 100;
					other = half - ((size_t)floor(sqrt(rand_high)) - 10) % half - 1;
				} while (population[other].G.invariant->T != population[i].G.invariant->T);

				if (other != i) {
					optimized_cross(population[i].G, population[rand() % half].G);
				}
			}
		}

		FOR(i, sz(population)) {
			auto &agent = population[i];
			agent.G.lock_distribution = target_score > 0
				&& round(agent.G.score - target_score) == agent.G.score - target_score;
		}
	}

	void step_and_prune(ld target_score = 0) {
		step();
		prune(target_score);
	}
};

pair<OptimizedGraph, bool> optimize(
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
	bool extended = true, delta = false;
	ll stagnation = 0;
	ld best_score = 1e18, previous_score = 1e18;
	OptimizedBlacksmithController smith;
	smith.init(G, team_count, population_size, T_start0, T_end0, Gs);
	smith.prune();
	ll batch_steps = (idle_steps + 1) * 5;
	bool reset_temp = false;

	auto comparision_index = sz(smith.population) / preserve_population_frac;
	ld previous_comparision_score = 1e+21;

	smith.start_mixup = comparision_index / 4;
	FOR (i, generations) {
		smith.step_and_prune(search_deltas ? target_score : 0);
		auto population_best = smith.population[0].G;
		if (population_best.score < best_score) {
			best_score = population_best.score;
			if (population_best.score < G.score) {
				G = population_best;
			}
		}

		if (target_score >= best_score && search_deltas) {
			target_score = best_score;
			search_deltas = false;
			delta = false;

			FOR (j, population_size) {
				smith.population[j].G.lock_distribution = false;
			}
		} else {
			FOR (j, population_size) {
				auto &agent = smith.population[j];
				if (target_score > 0 && round(agent.G.score - target_score) == agent.G.score - target_score) {
					agent.G.lock_distribution = true;
				} else {
					agent.G.lock_distribution = false;
				}
			}
		}

		if (i % batch_steps == 0) {
			optimized_write_output(G);
			auto comparision_score = smith.population[comparision_index - 1].G.score;
			cout << "Generation " << i << " best score (" << G.score << " | " << best_score << " | " << comparision_score << "), temperature " << smith.T_start << ", unchanged " << smith.unchanged << endl;

			if (search_deltas) {
				bool found_delta = false;
				FOR (j, population_size) {
					auto &agent = smith.population[j];
					if (agent.G.lock_distribution) {
						if (!found_delta) {
							optimized_write_output(agent.G, true);
							cout << "Found delta " << agent.G.score - target_score << endl;
							found_delta = true;
							delta = true;
						}
					}
				}
			}

			if (std::round(previous_comparision_score * 10) <= std::round(comparision_score * 10)
				|| (smith.unchanged > 150 && i > 100 && smith.T_start < 20)
				|| smith.T_start < 1e-2) {
				stagnation++;
				if (stagnation >= stagnation_limit) {
					cout << "Stagnation limit reached, terminating" << endl;
					break;
				}

				cout << "Stagnation " << stagnation << ", firing up forge" << endl;
				if (smith.T_start < T_start0 / ignition_factor || (delta && !reset_temp)) {
					smith.T_start *= ignition_factor;
					smith.T_end *= ignition_factor;
					if (delta && !reset_temp) {
						reset_temp = true;
						stagnation_limit += 1;
					}

					smith.start_mixup = comparision_index;
					cout << "Temperature set to " << smith.T_start << endl;
				} else {
					// smith.init(G, team_count, population_size, T_start0, T_end0);
					cout << "Ionizing" << endl;
					smith.start_mixup = comparision_index;
					smith.T_start *= ignition_factor;
					smith.T_end *= ignition_factor;
				}

				smith.T_start = min(smith.T_start, T_start0 * 2);
				smith.T_end = min(smith.T_end, T_end0 * 2);

				if (!delta && (best_score - target_score) > target_score * 0.2) {
					break;
				}

				best_score = 1e18;
				comparision_score = 1e21;
			} else {
			}

			if (G.score <= target_score && extended) {
				stagnation_limit++;
				extended = false;
			}

			previous_comparision_score = comparision_score;
			previous_score = best_score;
		}

		FOR (j, idle_steps) {
			smith.step();
		}
		i += idle_steps;
	}
	return {G, delta};
}

void basic_solve(Result &result, ld target_score, ch team_count = -1, ch min_team_count = 2) {
	OptimizedGraph G;
	char *compute_name = getenv("HOSTNAME");
	if (!compute_name) {
		compute_name = getenv("COMPUTERNAME");
	}
	G.score = 1e18;

	string comp_name = string(compute_name);
	optimized_read_graph(G, result.size, result.id, comp_name);

	short population_sz = 1024;
	ld T_start = 1000, T_end = 900, ignition_factor = 200;
	ll idle_steps = 19, generations = 1000000, stagnation_limit = 2;
	bool search_deltas = true;

	cout << "Basic solving " << result.size << result.id << " with population size " << population_sz << endl << endl;
	cout << "Target score " << target_score << endl << endl;

	if (team_count == -1) {
		team_count = max_teams(target_score);
	} elif (team_count < 2) {
		optimized_read_best_graph(G, result.size, result.id, comp_name);
		team_count += G.invariant->T;
	}
	ld prev_score = INF;

	while (team_count >= min_team_count && G.score < prev_score) {
		cout << "Team size " << team_count - 0 << endl;
		init_teams(G, team_count);
		prev_score = G.score;
		tie(G, ignore) = optimize(G, team_count, population_sz, generations, idle_steps, T_start, T_end, target_score, stagnation_limit, ignition_factor, search_deltas);
		optimized_write_output(G);
		if (G.score <= target_score + 1e-9) {
			cout << "Target score reached" << endl;
		} elif (G.score < result.local_score) {
			cout << "Local score beat" << endl;
		}
		cout << endl;
		team_count--;
	}
}

void final_solve(Result &result, ld target_score) {
	vector<OptimizedGraph> Gs, Gs_;
	OptimizedGraph G;
	char *compute_name = getenv("HOSTNAME");
	if (!compute_name) {
		compute_name = getenv("COMPUTERNAME");
	}

	string comp_name = string(compute_name);
	optimized_read_graph(G, result.size, result.id, comp_name);
	Gs = optimized_read_local_graphs(G, result.size, result.id, comp_name);
	map<ll, ll> team_sizes;
	ll elite = min((ll) sz(Gs), 128ll), idx = 0, int_deltas = 0;
	set<ll> int_delta_team_sizes;	
	while (idx < elite && sz(team_sizes) < 5) {
		team_sizes[Gs[idx].invariant->T]++;
		if (Gs[idx].score - target_score == round(Gs[idx].score - target_score)) {
			Gs[idx].lock_distribution = true;
			int_deltas++;
			int_delta_team_sizes.insert(Gs[idx].invariant->T);
		}
		Gs_.pb(Gs[idx]);
		idx++;
	}

	while (idx < sz(Gs) && sz(team_sizes) < 3) {
		if (team_sizes.find(Gs[idx].invariant->T) == team_sizes.end() ||
			team_sizes[Gs[idx].invariant->T] < 8) {
			team_sizes[Gs[idx].invariant->T]++;
			Gs_.pb(Gs[idx]);
		} elif (Gs[idx].score - target_score == round(Gs[idx].score - target_score)) {
			Gs[idx].lock_distribution = true;
			Gs_.pb(Gs[idx]);
			team_sizes[Gs[idx].invariant->T]++;
			int_deltas++;
			int_delta_team_sizes.insert(Gs[idx].invariant->T);
		}
		idx++;
	}

	while (idx < sz(Gs)) {
		if (Gs[idx].score - target_score == round(Gs[idx].score - target_score)) {
			Gs[idx].lock_distribution = true;
			Gs_.pb(Gs[idx]);
			team_sizes[Gs[idx].invariant->T]++;
			int_deltas++;
			int_delta_team_sizes.insert(Gs[idx].invariant->T);
		}
		idx++;
	}

	if (Gs[0].score < target_score) {
		return;
	}

	Gs = Gs_;
	short population_sz = 1024;
	ld T_start = 100, T_end = 95, ignition_factor = 200;
	ll idle_steps = 19, generations = 1000000, stagnation_limit = 2;
	bool search_deltas = true;

	cout << "Final solving " << result.size << result.id << " with population size " << population_sz << endl << endl;
	cout << "Target score " << target_score << endl;
	cout << "Best score: " << Gs[0].score << endl;
	cout << "Worst score: " << Gs.back().score << endl;
	cout << endl;

	target_score = min(target_score, Gs[0].score);
	if (sz(team_sizes) < 3 && sz(int_delta_team_sizes) == 0) {
		cout << "Not enough team sizes, adding another team size" << endl << endl;
		ll min_team_size = (team_sizes.begin())->first - 1;
		OptimizedGraph G_tmp = optimized_random_assignment(G, min_team_size);
		optimized_get_score(G_tmp);
		Gs.pb(G_tmp);
		team_sizes[min_team_size]++;
	}
	FORE (p, team_sizes) {
		if (int_delta_team_sizes.find(p.first) == int_delta_team_sizes.end()) {
			cout << "Team size " << p.first << ": " << p.second << " sols" << endl;
		} else {
			cout << "Team size " << p.first << ": " << p.second << " sols*" << endl;
		}
	}
	cout << endl;
	if (sz(int_delta_team_sizes) == 0) {
		cout << "No integer deltas found" << endl << endl;
		ld previous_score = 1e18;
		FORE (team_size, team_sizes) {
			vector<OptimizedGraph> Gs_filtered;
			FOR (i, sz(Gs)) {
				if (Gs[i].invariant->T == team_size.first) {
					Gs_filtered.pb(Gs[i]);
				}
			}
			cout << "Trying team size " << team_size.first << ": " << sz(Gs_filtered) << " sols" << endl;
			bool delta = false;
			tie(G, delta) = optimize(G, team_size.first, population_sz, generations, idle_steps, T_start, T_end, target_score, stagnation_limit, ignition_factor, search_deltas, Gs_filtered);
			optimized_write_output(G);
			if (G.score <= target_score + 1e-9) {
				cout << "Target score reached" << endl;
			} elif (G.score < result.local_score) {
				cout << "Local score beat" << endl;
			}
			if (delta) {
				cout << "Delta found, terminating" << endl << endl;
				break;
			} elif (previous_score < G.score) {
				cout << "Score increased, terminating" << endl << endl;
				break;
			}
			cout << endl;
			previous_score = G.score;
		}
	} else {
		cout << "Integer deltas found" << endl << endl;
		FORE (team_size, int_delta_team_sizes) {
			vector<OptimizedGraph> Gs_filtered;
			FOR (i, sz(Gs)) {
				if (Gs[i].invariant->T == team_size) {
					Gs_filtered.pb(Gs[i]);
				}
			}
			cout << "Trying team size " << team_size << ": " << sz(Gs_filtered) << " sols" << endl;
			tie(G, ignore) = optimize(G, team_size, population_sz, generations, idle_steps, T_start, T_end, target_score, stagnation_limit, ignition_factor, search_deltas, Gs_filtered);
			optimized_write_output(G);
			if (G.score <= target_score + 1e-9) {
				cout << "Target score reached" << endl;
			} elif (G.score < result.local_score) {
				cout << "Local score beat" << endl;
			}
			cout << endl;
		}
	}
}

int main() {
	cout.precision(15);
	auto start = chrono::high_resolution_clock::now();
	while (true) {
		srand(time(NULL));
		vector<Result> results = read_queue();
		FORE (result, results) {
			basic_solve(result, result.best_score);
			// final_solve(result, result.best_score);
			auto end = chrono::high_resolution_clock::now();
			auto duration = chrono::duration_cast<chrono::seconds>(end - start);
			cout << "Time elapsed: " << duration.count() << " seconds" << endl << endl;
		}
		cout << "------RESTARTING------" << endl << endl;
	}
	return 0;
}