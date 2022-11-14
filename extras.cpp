  /////////////////////////////////////////
 // scraped strategies not working well //
/////////////////////////////////////////

#include "header.cpp"

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