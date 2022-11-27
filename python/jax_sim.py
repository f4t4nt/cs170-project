import jax
import jax.numpy as jnp
import pickle as pkl
import time
from starter import *

TF_CPP_MIN_LOG_LEVEL=0
from jax.lib import xla_bridge
print(xla_bridge.get_backend().platform)

dt = 5e-4

G = read_input('./tests/small/small156/graph.in')
V = G.number_of_nodes()
l0 = np.zeros((V, V))
for u, v in G.edges:
    l0[u, v] = G[u][v]['weight']
    l0[v, u] = G[u][v]['weight']
l0 = l0 / np.mean(l0[l0 > 0]) * 10
l0 = jnp.array(l0)
# m = jnp.sum(l0, axis=1)
m = jnp.ones(V)
b = 1 - 1e-3 
d = jnp.array(np.random.rand(V, 3) * 10)
v = jnp.zeros((V, 3))

# V = 4
# l0 = jnp.array([[0, 1, 0, 0], [1, 0, 1, 0], [0, 1, 0, 1], [0, 0, 1, 0]])
# m = jnp.ones(V)
# b = 1# - 1e-2
# d = jnp.array([[-1, 0, 0], [0, -1, 0], [1, 0, 0], [0, 1, 0]]) * 30
# v = jnp.zeros((V, 3))

def spring_force(d, l0, k, V = V):
    u = d[:, None, :] - d[None, :, :]
    l = jnp.linalg.norm(u, axis=2)
    F = 2 * k * (l - l0)
    u = u / (l[:, :, None] + 1e-10)
    F = F[:, :, None] * u
    return F

def grav_force(d, g, m, V = V, p = 2):
    u = d[:, None, :] - d[None, :, :]
    l = jnp.linalg.norm(u, axis=2)
    F = -g * m[:, None] * m[None, :] / jnp.squeeze((l[:, :, None] + 1e-10) ** p)
    u = u / (l[:, :, None] + 1e-10)
    F = F[:, :, None] * u
    return F

def origin_force(d, k):
    F = -k * d
    return F

def step(d, v, l0, m, dt, b):
    F = grav_force(d, 1e1, m) + \
        grav_force(d, -1e-1, m, p=3) + \
        grav_force(d, -l0, m) + \
        origin_force(d, 7e-2)
    F = jnp.sum(F, axis=1)
    a = F / m[:, None]
    v = b * v + a * dt
    d = d + v * dt
    return d, v

jit_step = jax.jit(step)

def batch_step(d, v, l0, m, dt, b):
    for i in range(5000):
        d, v = jit_step(d, v, l0, m, dt, b)
    return d, v

history = []
start = time.time()

for i in range(1000):
    d, v = batch_step(d, v, l0, m, dt, b)
    history.append(d.copy())
    
end = time.time()
print(end - start, 's')

with open('./python/history.pkl', 'wb') as f:
    pkl.dump((G, d, history), f)