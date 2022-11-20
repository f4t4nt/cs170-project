#include "extras.cpp"

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

OptimizedGraph optimized_random_assignment(OptimizedGraph &G_in, ll team_count) {
	OptimizedGraph G = G_in;
	FOR (i, G.V) {
		G.node_teams[i] = rand() % team_count;
		G.team_counts[G.node_teams[i]]++;
	}
	return G;
}

struct OptimizedGeneticController {
	vector<OptimizedAnnealingAgent> population;
	ld T_start, T_end;
	void init(OptimizedGraph &G_in, ll team_count, ll population_size, ld T_start0, ld T_end0) {
		population = vector<OptimizedAnnealingAgent>(population_size);
		T_start = T_start0;
		T_end = T_end0;
		FOR (i, population_size) {
			// population[i] = {optimized_random_assignment(G_in, team_count), T_start};
			population[i] = {G_in, T_start};
			optimized_get_score(population[i].G);
		}
	}
	void step() {
		FORE (agent, population) {
			agent.T = T_start;
			while (agent.T > T_end) {
				FOR (i, 100) {
					agent.step();
				}
				agent.T *= 0.99;
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
		if (abs(score - previous_score) / previous_score < 1e-3) {
			stagnation++;
			if (stagnation >= stagnation_limit) {
				break;
			}
		} else {
			stagnation = 0;
		}
		if (score < best_score) {
			best_score = score;
			G = controller.population[0].G;
			if (best_score < target_score + 1e-3) {
				break;
			}
		}
	}
	return G;
}

int main() {
	vector<Result> results = read_queue();
	FORE (result, results) {
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

		// OptimizedGraph G;
		// optimized_read_graph(G, result.size, result.id, "optimized");
		// while (team_count >= 2) {
		// 	G.T = team_count;
		// 	G.team_counts = vector<ll>(team_count, 0);
		// 	G.B_vec = vector<ld>(team_count, 0);
		// 	G = optimized_algorithm(G, team_count, 20, 1000, 10, 9.5);
		// 	optimized_write_output(G);
		// 	if (optimized_get_score(G) < result.best_score + 1e-3) {
		// 		cout << "Beat best score of " << result.best_score << " with " << optimized_get_score(G) <<
		// 			" on " << result.size << result.id << endl;
		// 		break;
		// 	}
		// 	team_count--;
		// }

		OptimizedGraph G;
		optimized_read_best_graph(G, result.size, result.id, "optimized");
		G = optimized_algorithm(G, team_count, 20, 1000, 10, 9.5, 10, result.best_score);
		optimized_write_output(G);
	}
	return 0;
}