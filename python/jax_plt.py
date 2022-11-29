import jax.numpy as jnp
import matplotlib.pyplot as plt
import pickle as pkl
import sklearn.cluster as skc
from starter import *

with open('./python/history.pkl', 'rb') as f:
    (G, d, history) = pkl.load(f)
    V = G.number_of_nodes()

final = history[-1]

class Node:
    def __init__(self, i, d):
        self.i = i
        self.d = [d[0], d[1], d[2]]
        self.r = float(jnp.linalg.norm(d))
        self.theta = float(jnp.arccos(d[2] / self.r))
        self.phi = float(jnp.arctan2(d[1], d[0]))
        self.color = (float(self.phi / jnp.pi), 0, float(1 - self.phi / jnp.pi))
        self.edges = G[i]
        self.team = 0
    
    def __str__(self):
        return f'Node {self.i}: ({self.x}, {self.y}, {self.z}), r = {self.r}, theta = {self.theta}, phi = {self.phi}, color = {self.color}'
    
    def __repr__(self):
        return self.__str__()

nodes = [Node(i, d[i]) for i in range(V)]
nodes = sorted(nodes, key=lambda node: node.r, reverse=True)
free_nodes, busy_nodes = [], []
for i in range(len(nodes)):
    if nodes[i].r < 10:
        free_nodes.append((i, nodes[i]))
    else:
        busy_nodes.append((i, nodes[i]))

# team_count = 13
# # clustering based on polar coordinates (theta, phi)
# clusters = skc.KMeans(n_clusters=team_count).fit([[node[1].theta, node[1].phi] for node in busy_nodes]).labels_
# # clusters = skc.KMeans(n_clusters=team_count).fit([node[1].d for node in busy_nodes]).labels_
# counts = [0 for i in range(team_count)]
# for i in range(len(clusters)):
#     nodes[busy_nodes[i][0]].team = clusters[i]
#     counts[clusters[i]] += 1

# for i in range(len(free_nodes)):
#     nodes[free_nodes[i][0]].team = counts.index(min(counts))
#     counts[nodes[free_nodes[i][0]].team] += 1

# for i in range(len(nodes)):
#     nodes[i].team += 1
#     G.nodes[i]['team'] = nodes[i].team

# # conflicts = []
# # for u, v, w in G.edges(data=True):
# #     if G.nodes[u]['team'] == G.nodes[v]['team']:
# #         conflicts.append((u, v, w))

# print(f'Number of nodes: {len(nodes)}')
# print(f'Number of free nodes: {len(free_nodes)}')
# print(f'Team assignment: {[node.team for node in nodes]}')
# print(f'Number of nodes per team: {counts}')
# # print(f'Number of conflicts: {len(conflicts)}')
# # print(f'Conflicts: {conflicts}')
# print(f'Score: {score(G)}')
# print(f'Score separated: {score(G, separated=True)}')

plt.hist(jnp.linalg.norm(final, axis=1), bins=100)
plt.show()
plt.hist([node.r for node in nodes], bins=100)
plt.show()

# plt.hist([node.theta for node in nodes], bins=100)
# plt.show()

# plt.hist([node.phi for node in nodes], bins=100)
# plt.show()