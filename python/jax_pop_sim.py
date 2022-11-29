import jax
import jax.numpy as jnp
import numpy as np
import time
from starter import *

preset = False
G = read_input('./tests/small/small156/graph.in')
GV = G.number_of_nodes()
GT = 13
GT_1 = GT - 1
K = K_COEFFICIENT * jnp.exp(K_EXP * GT)
w = np.zeros((GV, GV))
for u, v, w_ in G.edges(data=True):
    w[u, v] = w_['weight']
    w[v, u] = w_['weight']
w = jnp.array(w)

preset = True
read_output(G, './tests/small/small156/345911_sick0.out')
preset_t = np.zeros(GV, dtype=np.int32)
preset_c = np.zeros(GT, dtype=np.int32)
preset_score = score(G)
for i in range(GV):
    preset_t[i] = G.nodes[i]['team'] - 1
    preset_c[preset_t[i]] += 1

T = 10

def score_(t, c):
    return K + jnp.exp(B_EXP * jnp.linalg.norm(c / GV - 1 / GT)) + jnp.sum((t[:, None] == t[None, :]) * w) / 2

jit_score = jax.jit(score_)

def gen(t0, c0, rng):
    # pick node
    p = jnp.exp(B_EXP * jnp.abs(c0[t0] / GV - 1 / GT)) + jnp.sum((t0[:, None] == t0[None, :]) * w, axis=1)
    p_nodes = p / jnp.sum(p)
    u = jax.random.choice(jax.random.PRNGKey(rng), jnp.arange(GV), p=p_nodes)
    # p = jnp.ones(GV)
    # u = jax.random.randint(jax.random.PRNGKey(rng), (), 0, GV)
    ut0 = t0[u]
    
    # pick team
    pt = jnp.exp(B_EXP * jnp.abs((c0 + 1) / GV - 1 / GT)) - jnp.exp(B_EXP * jnp.abs(c0 / GV - 1 / GT)) + jnp.sum((t0[:, None] == jnp.arange(GT)).T * w[u], axis=1)
    pt = jnp.max(pt) - pt
    pt_teams = pt / jnp.sum(pt)
    ut1 = jax.random.choice(jax.random.PRNGKey(rng), jnp.arange(GT), p=pt_teams)
    # ut1 = jax.random.randint(jax.random.PRNGKey(rng), (), 0, GT)
    
    # calculate score
    c1 = c0.at[ut0].set(c0[ut0] - 1).at[ut1].set(c0[ut1] + 1)
    t1 = t0.at[u].set(ut1)
    score1 = jit_score(t1, c1)
    # return
    return score1, t1, c1, u, ut0, ut1

jit_gen = jax.jit(gen)

def jaxless_gen(t0, c0, rng):
    # pick node
    p = jnp.exp(B_EXP * jnp.abs(c0[t0] / GV - 1 / GT)) + jnp.sum((t0[:, None] == t0[None, :]) * w, axis=1)
    p_nodes = p / jnp.sum(p)
    u = jax.random.choice(jax.random.PRNGKey(rng), jnp.arange(GV), p=p_nodes)
    # p = jnp.ones(GV)
    # u = jax.random.randint(jax.random.PRNGKey(rng), (), 0, GV)
    ut0 = t0[u]
    
    # pick team
    pt = jnp.exp(B_EXP * jnp.abs((c0 + 1) / GV - 1 / GT)) - jnp.exp(B_EXP * jnp.abs(c0 / GV - 1 / GT)) + jnp.sum((t0[:, None] == jnp.arange(GT)).T * w[u], axis=1)
    pt = jnp.max(pt) - pt
    pt_teams = pt / jnp.sum(pt)
    ut1 = jax.random.choice(jax.random.PRNGKey(rng), jnp.arange(GT), p=pt_teams)
    # ut1 = jax.random.randint(jax.random.PRNGKey(rng), (), 0, GT)
    
    # calculate score
    c1 = c0.at[ut0].set(c0[ut0] - 1).at[ut1].set(c0[ut1] + 1)
    t1 = t0.at[u].set(ut1)
    score1 = score_(t1, c1)
    # return
    return score1, t1, c1, u, ut0, ut1

class agent:
    def __init__(self, t = None, c = None, score = 0, step_idx = 0):
        self.step_idx = step_idx
        if t is None:
            t = jnp.array(np.random.randint(0, GT, GV))
            self.t = t
            self.c = jnp.bincount(t, minlength=GT)
            self.score = jit_score(self.t, self.c)
        else:
            self.t = t
            if c is None:
                self.c = jnp.bincount(t, minlength=GT)
                self.score = jit_score(self.t, self.c)
            else:
                self.c = c
                if score == 0:
                    self.score = jit_score(self.t, self.c)
                else:
                    self.score = score
    
    def __lt__(self, other):
        return self.score < other.score
    
    def __le__(self, other):
        return self.score <= other.score

    def __eq__(self, other):
        return self.score == other.score
    
    def __ne__(self, other):
        return self.score != other.score
    
    def __gt__(self, other):
        return self.score > other.score

    def __ge__(self, other):
        return self.score >= other.score
    
    def __str__(self):
        return f'{self.score:.2f}'
    
    def __repr__(self):
        return self.__str__()

    def get_p(self):
        return jnp.exp(B_EXP * jnp.abs(self.c[self.t] / GV - 1 / GT)) + jnp.sum((self.t[:, None] == self.t[None, :]) * w, axis=1)
    
    def save_figure(self):
        plt.clf()
        t = self.t.tolist()
        partition = dict()
        for n, t_ in enumerate(t):
            if int(t_) not in partition:
                partition[int(t_)] = []
            partition[int(t_)].append(n)
        pos = dict()
        circle_size = len(partition) * 0.5
        for k, v in partition.items():
            pos.update(nx.shell_layout(G, nlist=[v], center=(circle_size*math.cos(math.tau*k / len(partition)),
                                                            circle_size*math.sin(math.tau*k / len(partition)))))
        p = self.get_p()
        avg_p = jnp.mean(p)
        nx.draw_networkx_nodes(G, pos, node_color=t, cmap=cm.get_cmap('tab20b'), node_size=200*p/avg_p)
        nx.draw_networkx_labels(G, pos, font_size=10, font_color="white")
        inter_edges = [e for e in G.edges(data='weight') if t[e[0]] != t[e[1]]]
        intra_edges = [e for e in G.edges(data='weight') if t[e[0]] == t[e[1]]]
        max_weight = max(nx.get_edge_attributes(G, name='weight').values())
        nx.draw_networkx_edges(G, pos, edgelist=inter_edges, edge_color=[x[2] for x in inter_edges],
                            edge_cmap=cm.get_cmap('Blues'), edge_vmax=max_weight*1.5, edge_vmin=max_weight*-0.2)
        nx.draw_networkx_edges(G, pos, width=2, edgelist=intra_edges, edge_color=[x[2] for x in intra_edges],
                            edge_cmap=cm.get_cmap('Reds'), edge_vmax=max_weight*1.5, edge_vmin=max_weight*-0.2)
        plt.tight_layout()
        plt.axis("off")
        plt.text(0, 0, f'{self.score:.2f}', fontsize=10)
        plt.savefig(f'./python/figures/{self.step_idx:06d}.png', dpi=300)
        plt.close()
        
    def gen(self, print_result=False, save_figure=False):
        self.score1, self.t1, self.c1, self.u, self.ut0, self.ut1 = jit_gen(self.t, self.c, self.step_idx)
        self.step_idx += 1
        if print_result:
            return f'{self.step_idx:04d}: {self.u:02d} {self.ut0} -> {self.ut1} {self.score1:.2f}'
    
    def jaxless_gen(self, print_result=False, save_figure=False):
        self.score1, self.t1, self.c1, self.u, self.ut0, self.ut1 = jaxless_gen(self.t, self.c, self.step_idx)
        self.step_idx += 1
        if print_result:
            return f'{self.step_idx:04d}: {self.u:02d} {self.ut0} -> {self.ut1} {self.score1:.2f}'
    
    def accept(self, print_result=False):
        P = jnp.where(self.score1 < self.score, 1, jnp.exp((self.score - self.score1) / T))
        if np.random.random() < P:
            self.score = self.score1
            self.t = self.t1
            self.c = self.c1
            if print_result:
                return 'accepted'
        elif print_result:
            return 'rejected'
        
    def jaxless_accept(self, print_result=False):
        P = jnp.where(self.score1 < self.score, 1, jnp.exp((self.score - self.score1) / T))
        if np.random.random() < P:
            self.score = self.score1
            self.t = self.t1
            self.c = self.c1
            if print_result:
                return 'accepted'
        elif print_result:
            return 'rejected'
    
    def step(self, print_result=False, save_figure=False):
        gen_res = self.gen(print_result, save_figure)
        accept_res = self.accept(print_result)
        if save_figure:
            self.save_figure()
        if print_result:
            print(f'{gen_res} {accept_res}')
            
    def jaxless_step(self, print_result=False, save_figure=False):
        gen_res = self.jaxless_gen(print_result, save_figure)
        accept_res = self.jaxless_accept(print_result)
        if save_figure:
            self.save_figure()
        if print_result:
            print(f'{gen_res} {accept_res}')

vmap_gen = jax.vmap(jit_gen, in_axes=(0, 0, 0), out_axes=(0, 0, 0, 0, 0, 0))

def batch_step(agents):
    scores, ts, cs, us, ut0s, ut1s = vmap_gen(jnp.array([a.t for a in agents]), jnp.array([a.c for a in agents]), jnp.array([a.step_idx for a in agents]))
    for i, a in enumerate(agents):
        a.score1 = scores[i]
        a.t1 = ts[i]
        a.c1 = cs[i]
        a.u = us[i]
        a.ut0 = ut0s[i]
        a.ut1 = ut1s[i]
        a.step_idx += 1
    P = jnp.where(scores < jnp.array([a.score for a in agents]), 1, jnp.exp((jnp.array([a.score for a in agents]) - scores) / T))
    Q = jnp.array(np.random.rand(len(agents)))
    for i, a in enumerate(agents):
        if Q[i] < P[i]:
            a.score = a.score1
            a.t = a.t1
            a.c = a.c1
    return agents

def prune(agents):
    agents.sort()
    half_len = len(agents) // 2
    for i in range(half_len):
        agents[i+half_len] = agent(agents[i].t, agents[i].c, agents[i].score, agents[i].step_idx)
    return agents

def jaxless_batch_step(agents):
    for a in agents:
        a.jaxless_step()
    agents.sort()
    half_len = len(agents) // 2
    for i in range(half_len):
        agents[i+half_len] = agent(agents[i].t, agents[i].c, agents[i].score, agents[i].step_idx)
    return agents
        
POPULATION_SIZE = 100
if not preset:
    agents = [agent() for _ in range(POPULATION_SIZE)]
else:
    agents = [agent(preset_t, preset_c, preset_score) for _ in range(POPULATION_SIZE)]
best = (float('inf'), None)
start = time.time()
for _ in range(1000000):
    agents = batch_step(agents)
    agents = prune(agents)
    if agents[0].score < best[0]:
        best = (agents[0].score, agents[0].t)
        print(f'new best: {agents[0].score:.2f}')
        print(best[1])
        agents[0].save_figure()
    T *= 0.999
    print(f'{_ + 1} steps done in {time.time() - start:.2f} seconds, T = {T:.2f}, span = ({agents[0].score:.2f}, {agents[-1].score:.2f})')

print('done')
print(best[0])
print(best[1])