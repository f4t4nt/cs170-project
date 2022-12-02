from starter import *
import random
import sys

sys.setrecursionlimit(1000000)

G = read_input('./tests/medium/medium13/graph.in')
# target_score = 366095.4145999371

# node_weights = [(0, i) for i in range(G.number_of_nodes())]
# weight_matrix = nx.to_numpy_matrix(G)
# for u, v in G.edges:
#     node_weights[u] = (node_weights[u][0] + G.edges[u, v]['weight'], u)
#     node_weights[v] = (node_weights[v][0] + G.edges[u, v]['weight'], v)

# node_weights.sort(reverse=True)

# def get_components(G: nx.Graph):
#     components = []
#     visited = set()
#     for v in G:
#         if v in visited:
#             continue
#         visited.add(v)
#         component = [v]
#         queue = [v]
#         while len(queue) > 0:
#             u = queue.pop()
#             for v in G[u]:
#                 if v not in visited:
#                     visited.add(v)
#                     component.append(v)
#                     queue.append(v)
#         components.append(component)
#     return components

# components = get_components(G)
# print(len(components))

# team_count = 13
# # for i in range(G.number_of_nodes()):
# #     team_count = max(team_count, G.nodes[i]['team'])
# # team_dist = [0] * team_count
# # for i in range(G.number_of_nodes()):
# #     team_dist[G.nodes[i]['team'] - 1] += 1
# # team_dist.sort(reverse=True)
# # print(team_dist)
# # target_score -= K_COEFFICIENT * math.exp(K_EXP * team_count) + math.exp(B_EXP * np.linalg.norm((np.array(team_dist) / G.number_of_nodes()) - 1 / team_count, 2))
# # print(target_score)

# max_t = 1000
# min_t = 0

# # 'largest_first'
# # 'random_sequential'
# # 'smallest_last'
# # 'independent_set'
# # 'connected_sequential_bfs'
# # 'connected_sequential_dfs'
# # 'connected_sequential' (alias for the previous strategy)
# # 'saturation_largest_first'
# # 'DSATUR' (alias for the previous strategy)

# while max_t - min_t > 1:
#     G_copy = G.copy()
#     threshold = (max_t + min_t) // 2
#     for edge in G_copy.edges:
#         if G_copy.edges[edge]['weight'] < threshold:
#             G_copy.remove_edge(edge[0], edge[1])
#     colors = nx.coloring.greedy_color(G_copy, strategy='smallest_last')
#     if len(set(colors.values())) > team_count:
#         min_t = threshold
#     else:
#         max_t = threshold

# deg_list = []
# for v in G:
#     deg_list.append((len(G[v]), v))
# deg_list.sort()

# threshold = max_t
# edges = []
# for u, v, w in G.edges(data=True):
#     edges.append((w['weight'], u, v))
# edges.sort()
# removed_edges = []
# for edge in G.edges:
#     if G.edges[edge]['weight'] < threshold:
#         removed_edges.append(edge)
#         G.remove_edge(edge[0], edge[1])
# colors = nx.coloring.greedy_color(G, strategy='smallest_last')
# team_dist = [0] * team_count
# for v in G:
#     G.nodes[v]['team'] = colors[v] + 1
#     team_dist[colors[v]] += 1
# for _ in range(10):
#     for v in G:
#         possible_teams = []
#         for team in range(team_count):
#             valid = True
#             for neighbor in G[v]:
#                 if G.nodes[neighbor]['team'] == team + 1:
#                     valid = False
#                     break
#             if valid:
#                 possible_teams.append(team)
#         smallest_team = float('inf')
#         for team in possible_teams:
#             if team_dist[team] < smallest_team:
#                 smallest_team = team_dist[team]
#                 G.nodes[v]['team'] = team + 1
# print(team_dist)
# for edge in removed_edges:
#     G.add_edge(edge[0], edge[1])
#     G.edges[edge]['weight'] = weight_matrix[edge[0], edge[1]]
# validate_output(G)
# # visualize(G)
# print(score(G, separated=True))
# print(score(G))
# for team in range(1, team_count + 1):
#     print(team, [v for v in G if G.nodes[v]['team'] == team])

dataset = [
    [
        [0, 13, 26, 39, 51, 64, 77, 90, 101, 114, 127, 140, 151, 164, 177, 190, 201, 214, 227, 240, 251, 264, 277, 290],
        [1, 14, 27, 40, 50, 63, 76, 89, 102, 115, 128, 141, 152, 165, 178, 191, 202, 215, 228, 241, 252, 265, 278, 291],
        [2, 15, 28, 41, 52, 65, 78, 91, 100, 113, 126, 139, 153, 166, 179, 192, 203, 216, 229, 242, 253, 266, 279, 292],
        [3, 16, 29, 42, 53, 66, 79, 92, 103, 116, 129, 142, 150, 163, 176, 189, 204, 217, 230, 243, 254, 267, 280, 293],
        [4, 17, 30, 43, 54, 67, 80, 93, 104, 117, 130, 143, 154, 167, 180, 193, 200, 213, 226, 239, 255, 268, 281, 294],
        [5, 18, 31, 44, 55, 68, 81, 94, 105, 118, 131, 144, 155, 168, 181, 194, 205, 218, 231, 244, 250, 263, 276, 289],
        [6, 19, 32, 45, 56, 69, 82, 95, 106, 119, 132, 145, 156, 169, 182, 195, 206, 219, 232, 245, 256, 269, 282, 295],
        [7, 20, 33, 46, 57, 70, 83, 96, 107, 120, 133, 146, 157, 170, 183, 196, 207, 220, 233, 246, 257, 270, 283, 296],
        [8, 21, 34, 47, 58, 71, 84, 97, 108, 121, 134, 147, 158, 171, 184, 197, 208, 221, 234, 247, 258, 271, 284, 297],
        [9, 22, 35, 48, 59, 72, 85, 98, 109, 122, 135, 148, 159, 172, 185, 198, 209, 222, 235, 248, 259, 272, 285, 298],
        [10, 23, 36, 49, 60, 73, 86, 99, 110, 123, 136, 149, 160, 173, 186, 199, 210, 223, 236, 249, 260, 273, 286, 299],
        [11, 24, 37, 61, 74, 87, 111, 124, 137, 161, 174, 187, 211, 224, 237, 261, 274, 287],
        [12, 25, 38, 62, 75, 88, 112, 125, 138, 162, 175, 188, 212, 225, 238, 262, 275, 288],
    ],
    [
        [12, 25, 38, 50, 63, 76, 89, 112, 125, 138, 162, 175, 188, 212, 225, 238, 262, 275, 288],
        [0, 13, 26, 39, 62, 75, 88, 111, 124, 137, 161, 174, 187, 211, 224, 237, 261, 274, 287],
        [1, 14, 27, 40, 61, 74, 87, 100, 113, 126, 139, 160, 173, 186, 199, 210, 223, 236, 249, 260, 273, 286, 299],
        [2, 15, 28, 41, 60, 73, 86, 99, 110, 123, 136, 149, 150, 163, 176, 189, 209, 222, 235, 248, 259, 272, 285, 298],
        [3, 16, 29, 42, 59, 72, 85, 98, 109, 122, 135, 148, 159, 172, 185, 198, 200, 213, 226, 239, 258, 271, 284, 297],
        [4, 17, 30, 43, 58, 71, 84, 97, 108, 121, 134, 147, 158, 171, 184, 197, 208, 221, 234, 247, 250, 263, 276, 289],
        [5, 18, 31, 44, 57, 70, 83, 96, 107, 120, 133, 146, 157, 170, 183, 196, 207, 220, 233, 246, 257, 270, 283, 296],
        [6, 19, 32, 45, 56, 69, 82, 95, 106, 119, 132, 145, 156, 169, 182, 195, 206, 219, 232, 245, 256, 269, 282, 295],
        [7, 20, 33, 46, 55, 68, 81, 94, 105, 118, 131, 144, 155, 168, 181, 194, 205, 218, 231, 244, 255, 268, 281, 294],
        [8, 21, 34, 47, 54, 67, 80, 93, 104, 117, 130, 143, 154, 167, 180, 193, 204, 217, 230, 243, 254, 267, 280, 293],
        [9, 22, 35, 48, 53, 66, 79, 92, 103, 116, 129, 142, 153, 166, 179, 192, 203, 216, 229, 242, 253, 266, 279, 292],
        [10, 23, 36, 49, 52, 65, 78, 91, 102, 115, 128, 141, 152, 165, 178, 191, 202, 215, 228, 241, 252, 265, 278, 291],
        [11, 24, 37, 51, 64, 77, 90, 101, 114, 127, 140, 151, 164, 177, 190, 201, 214, 227, 240, 251, 264, 277, 290],
    ],
    [
        [12, 25, 38, 62, 75, 88, 111, 124, 137, 159, 172, 185, 198, 200, 213, 226, 239, 255, 268, 281, 294],
        [0, 13, 26, 39, 61, 74, 87, 110, 123, 136, 149, 158, 171, 184, 197, 212, 225, 238, 254, 267, 280, 293],
        [11, 24, 37, 60, 73, 86, 99, 109, 122, 135, 148, 150, 163, 176, 189, 211, 224, 237, 253, 266, 279, 292],
        [10, 23, 36, 49, 59, 72, 85, 98, 100, 113, 126, 139, 157, 170, 183, 196, 210, 223, 236, 249, 252, 265, 278, 291],
        [9, 22, 35, 48, 50, 63, 76, 89, 112, 125, 138, 156, 169, 182, 195, 209, 222, 235, 248, 251, 264, 277, 290],
        [8, 21, 34, 47, 58, 71, 84, 97, 108, 121, 134, 147, 155, 168, 181, 194, 208, 221, 234, 247, 262, 275, 288],
        [7, 20, 33, 46, 57, 70, 83, 96, 107, 120, 133, 146, 154, 167, 180, 193, 207, 220, 233, 246, 261, 274, 287],
        [5, 18, 31, 44, 56, 69, 82, 95, 106, 119, 132, 145, 153, 166, 179, 192, 206, 219, 232, 245, 260, 273, 286, 299],
        [4, 17, 30, 43, 55, 68, 81, 94, 105, 118, 131, 144, 152, 165, 178, 191, 205, 218, 231, 244, 259, 272, 285, 298],
        [3, 16, 29, 42, 54, 67, 80, 93, 104, 117, 130, 143, 162, 175, 188, 204, 217, 230, 243, 258, 271, 284, 297],
        [2, 15, 28, 41, 53, 66, 79, 92, 103, 116, 129, 142, 161, 174, 187, 203, 216, 229, 242, 257, 270, 283, 296],
        [1, 14, 27, 40, 52, 65, 78, 91, 102, 115, 128, 141, 160, 173, 186, 199, 202, 215, 228, 241, 256, 269, 282, 295],
        [6, 19, 32, 45, 51, 64, 77, 90, 101, 114, 127, 140, 151, 164, 177, 190, 201, 214, 227, 240, 250, 263, 276, 289],
    ]
]

for data in dataset:
    team_assignments = [0] * 300
    for i, team in enumerate(data):
        for player in team:
            team_assignments[player] = i + 1
            G.nodes[player]['team'] = i + 1
    print(team_assignments)
    print(score(G))

best = [9, 10, 11, 12, 13, 1, 7, 8, 3, 4, 2, 5, 6, 12, 3, 11, 13, 7, 1, 4, 9, 8, 2, 10, 6, 5, 10, 7, 3, 11, 1, 13, 8, 2, 12, 4, 9, 6, 5, 8, 7, 10, 2, 11, 12, 13, 9, 4, 1, 3, 12, 7, 11, 9, 5, 13, 4, 2, 8, 6, 1, 3, 10, 12, 9, 4, 2, 13, 1, 11, 6, 7, 8, 5, 3, 10, 1, 2, 6, 7, 11, 13, 9, 8, 4, 5, 12, 3, 10, 6, 11, 5, 8, 12, 7, 1, 13, 4, 9, 2, 13, 7, 9, 2, 12, 5, 8, 11, 4, 10, 6, 1, 3, 2, 9, 13, 8, 10, 12, 11, 6, 7, 5, 4, 3, 1, 9, 8, 4, 5, 11, 10, 2, 6, 7, 12, 13, 1, 3, 5, 11, 13, 7, 6, 4, 12, 9, 10, 8, 2, 6, 11, 13, 12, 3, 1, 8, 5, 2, 9, 10, 7, 4, 8, 6, 3, 10, 12, 1, 9, 2, 5, 13, 11, 4, 7, 10, 8, 1, 2, 6, 13, 12, 3, 9, 5, 11, 7, 4, 8, 13, 1, 3, 9, 12, 6, 10, 5, 2, 11, 1, 10, 9, 6, 7, 2, 8, 4, 12, 5, 3, 11, 13, 2, 8, 12, 10, 1, 7, 9, 3, 4, 5, 6, 11, 13, 3, 6, 4, 1, 2, 9, 10, 7, 5, 12, 8, 11, 13, 10, 1, 9, 3, 12, 6, 4, 5, 8, 7, 2, 2, 13, 7, 12, 8, 6, 5, 1, 4, 10, 9, 11, 3, 2, 1, 7, 4, 12, 6, 8, 13, 9, 5, 10, 11, 3, 4, 6, 5, 8, 12, 2, 9, 10, 13, 7, 1, 3, 11, 8, 4, 9, 2, 10, 13, 5, 6, 12, 7, 1]

teams = [[] for _ in range(13)]

for i in range(len(best)):
    teams[best[i] - 1].append(i)

dataset.append(teams)

frequency = [[0 for i in range(300)] for j in range(300)]

for data in dataset:
    for team in data:
        for node1 in team:
            for node2 in team:
                frequency[node1][node2] += 1

class DSU:
    def __init__(self, size):
        self.parent = [i for i in range(size)]
        self.rank = [0 for i in range(size)]
        self.size = [1 for i in range(size)]
    
    def find(self, x):
        if self.parent[x] != x:
            self.parent[x] = self.find(self.parent[x])
        return self.parent[x]
    
    def union(self, x, y):
        x = self.find(x)
        y = self.find(y)
        if x == y:
            return
        if self.rank[x] < self.rank[y]:
            self.parent[x] = y
            self.size[y] += self.size[x]
        else:
            self.parent[y] = x
            self.size[x] += self.size[y]
            if self.rank[x] == self.rank[y]:
                self.rank[x] += 1

dsu = DSU(300)

# make components if all nodes always appear together
for i in range(300):
    for j in range(300):
        if frequency[i][j] == 3:
            dsu.union(i, j)

# print all dsu components

components = dict()

for i in range(300):
    root = dsu.find(i)
    if root not in components:
        components[root] = []
    components[root].append(i)

values = list(components.values())

for value in values:
    print(value)