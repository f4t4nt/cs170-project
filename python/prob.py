from starter import *
import random
import sys

sys.setrecursionlimit(1000000)

G = read_input('./tests/large/large6/graph.in')
target_score = 494.2023873427806

node_weights = [(0, i) for i in range(G.number_of_nodes())]
weight_matrix = nx.to_numpy_matrix(G)
for u, v in G.edges:
    node_weights[u] = (node_weights[u][0] + G.edges[u, v]['weight'], u)
    node_weights[v] = (node_weights[v][0] + G.edges[u, v]['weight'], v)

node_weights.sort(reverse=True)

def get_components(G: nx.Graph):
    components = []
    visited = set()
    for v in G:
        if v in visited:
            continue
        visited.add(v)
        component = [v]
        queue = [v]
        while len(queue) > 0:
            u = queue.pop()
            for v in G[u]:
                if v not in visited:
                    visited.add(v)
                    component.append(v)
                    queue.append(v)
        components.append(component)
    return components

components = get_components(G)

team_count = 3
# for i in range(G.number_of_nodes()):
#     team_count = max(team_count, G.nodes[i]['team'])
# team_dist = [0] * team_count
# for i in range(G.number_of_nodes()):
#     team_dist[G.nodes[i]['team'] - 1] += 1
# team_dist.sort(reverse=True)
# print(team_dist)
# target_score -= K_COEFFICIENT * math.exp(K_EXP * team_count) + math.exp(B_EXP * np.linalg.norm((np.array(team_dist) / G.number_of_nodes()) - 1 / team_count, 2))
# print(target_score)

# binary search on a threshold that makes so we're only using team_count number of teams
# max_t = 1000
# min_t = 0

# while max_t - min_t > 1:
#     G_copy = G.copy()
#     threshold = (max_t + min_t) // 2
#     for edge in G_copy.edges:
#         if G_copy.edges[edge]['weight'] > threshold:
#             G_copy.remove_edge(edge[0], edge[1])
#     colors = nx.coloring.greedy_color(G_copy, strategy='DSATUR')
#     if len(set(colors.values())) > team_count:
#         min_t = threshold
#     else:
#         max_t = threshold

set0 = set(range(G.number_of_nodes()))
for v in G[0]:
    set0.remove(v)

# edges = []
# for u, v, w in G.edges(data=True):
#     if u != 0 and v != 0:
#         edges.append((w['weight'], u, v))
# edges.sort()

deg_list = []
for v in G:
    deg_list.append((len(G[v]), v))
deg_list.sort()

threshold = 500
for edge in G.edges:
    if G.edges[edge]['weight'] > threshold:
        G.remove_edge(edge[0], edge[1])
colors = nx.coloring.greedy_color(G, strategy='DSATUR')
team_dist = [0] * team_count
for v in G:
    G.nodes[v]['team'] = colors[v] + 1
    team_dist[colors[v]] += 1
# un0_count = [0, 0, 0]
for v in set0:
    # un0_count[G.nodes[v]['team'] - 1] += 1
    if G.nodes[v]['team'] == 2:
        G.nodes[v]['team'] = 3
        team_dist[2] += 1
        team_dist[1] -= 1
print(team_dist)
validate_output(G)
print(score(G, separated=True))
print(score(G))
print('[' + ', '.join([str(G.nodes[i]['team']) for i in range(G.number_of_nodes())]) + ']')