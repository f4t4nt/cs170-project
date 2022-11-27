import pickle as pkl
import jax.numpy as jnp
import matplotlib.pyplot as plt
from starter import *

with open('./python/history.pkl', 'rb') as f:
    (G, d, history) = pkl.load(f)
    V = G.number_of_nodes()

final = history[-1]

class Node:
    def __init__(self, i, d):
        self.i = i
        self.x = float(d[0])
        self.y = float(d[1])
        self.z = float(d[2])
        self.r = float(jnp.linalg.norm(d))
        self.theta = float(jnp.arccos(d[2] / self.r))
        self.phi = float(jnp.arctan2(d[1], d[0]))
        self.color = (float(self.phi / jnp.pi), 0, float(1 - self.phi / jnp.pi))
        self.edges = G[i]
    
    def __str__(self):
        return f'Node {self.i}: ({self.x}, {self.y}, {self.z}), r = {self.r}, theta = {self.theta}, phi = {self.phi}, color = {self.color}'
    
    def __repr__(self):
        return self.__str__()

# nodes = [Node(i, d[i]) for i in range(V)]
# nodes = sorted(nodes, key=lambda node: node.r, reverse=True)

plt.hist(jnp.linalg.norm(final, axis=1), bins=100)
plt.show()
# plt.hist([node.r for node in nodes], bins=100)
# plt.show()

# plt.hist([node.theta for node in nodes], bins=100)
# plt.show()

# plt.hist([node.phi for node in nodes], bins=100)
# plt.show()