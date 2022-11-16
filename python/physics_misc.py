import numpy as np
import vpython as vp
import matplotlib.pyplot as plt
from starter import *

def visualize_physics_2d(node_pos, edge_weights, teams):
    node_count = node_pos.shape[0]
    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.set_aspect('equal')
    ax.set_xticks([])
    ax.set_yticks([])
    for v in range(node_count):
        for u in range(v):
            if edge_weights[v, u] > 0:
                ax.plot([node_pos[v, 0], node_pos[u, 0]], [node_pos[v, 1], node_pos[u, 1]], color='black', alpha=0.5)
    team_colors = dict()
    for v in range(node_count):
        if teams[v] not in team_colors:
            team_colors[teams[v]] = np.random.rand(3)
        ax.scatter(node_pos[v, 0], node_pos[v, 1], color=team_colors[teams[v]], zorder=10)
    ax.set_xlim(np.min(node_pos[:, 0]) - 1, np.max(node_pos[:, 0]) + 1)
    ax.set_ylim(np.min(node_pos[:, 1]) - 1, np.max(node_pos[:, 1]) + 1)
    plt.show()

def visualize_physics(history, node_count, edge_weights, teams, show_edges=True):
    def vector_to_3d(vector):
        rv = []
        if len(vector) == 1:
            rv = np.append(vector, [0, 0])
        elif len(vector) == 2:
            rv =  np.append(vector, 0)
        else:
            rv = vector[:3]
        return vp.vector(*rv)
    
    vp.scene.width = 1600
    vp.scene.height = 900
    vp.scene.background = vp.color.white
    vp.scene.center = vector_to_3d(np.mean(history[0], axis=0))
    vp.scene.range = 1.5 * np.max(np.linalg.norm(history[0], axis=1))
    vp.scene.autoscale = True
    
    nodes = []
    team_colors = dict()
    for v in range(node_count):
        if teams[v] not in team_colors:
            team_colors[teams[v]] = vp.color.hsv_to_rgb(vp.vector(np.random.rand(), 1, 1))
        nodes.append(vp.sphere(pos=vector_to_3d(history[0][v]), radius=5, color=team_colors[teams[v]]))
    
    if show_edges:
        edges = dict()
        for i in range(node_count):
            for j in range(node_count):
                if edge_weights[i][j] > 0:
                    if teams[i] == teams[j]:
                        edges[(i, j)] = vp.cylinder(pos=nodes[i].pos, axis=nodes[j].pos - nodes[i].pos, radius=0.1, color=vp.color.orange)
                    else:
                        edges[(i, j)] = vp.cylinder(pos=nodes[i].pos, axis=nodes[j].pos - nodes[i].pos, radius=0.1, color=vp.color.blue)
    for step in range(len(history)):
        vp.rate(120)
        for i in range(node_count):
            nodes[i].pos = vector_to_3d(history[step][i])
        if show_edges:
            for i in range(node_count):
                for j in range(node_count):
                    if edge_weights[i][j] > 0:
                        edges[(i, j)].pos = vector_to_3d(history[step][i])
                        edges[(i, j)].axis = vector_to_3d(history[step][j] - history[step][i])

def physics_solve(G: nx.Graph, dim: int=3, vel_decay: float=0.99, total_steps: int=2000):
    node_count = G.number_of_nodes()
    edge_weights = np.zeros((node_count, node_count))
    for v in range(node_count):
        for u in range(v):
            if G.has_edge(v, u):
                edge_weights[v, u] = edge_weights[u, v] = G[v][u]['weight']
    
    history = []

    dt = 1e-3
    node_pos = (np.random.rand(node_count, dim) * 2 - 1)
    node_vel = np.zeros((node_count, dim))
    # node_vel = np.random.rand(node_count, dim) * 2 - 1
    history.append(node_pos)

    def get_acceleration():
        node_accel = np.zeros((node_count, dim))
        dist = np.linalg.norm(node_pos[:, np.newaxis] - node_pos, axis=2)
        dist[dist == 0] = 1
        node_accel = np.sum((edge_weights[:, :, np.newaxis] - dist[:, :, np.newaxis]) * (node_pos[:, np.newaxis] - node_pos) / dist[:, :, np.newaxis] * 10000, axis=1)
        node_accel += -node_pos / np.linalg.norm(node_pos, axis=1)[:, np.newaxis]
        return node_accel

    for step in range(int(total_steps)):
        node_pos = node_pos + node_vel * dt
        node_accel = get_acceleration()
        node_vel = node_vel + node_accel * dt
        node_vel = node_vel * vel_decay
        history.append(node_pos)

    return history, node_pos, edge_weights

def get_teams(G: nx.Graph, team_count: int, node_pos: np.ndarray):
    node_count = G.number_of_nodes()
    class DSU:
        def __init__(self, n):
            self.parent = list(range(n))
            self.size = [1] * n
            self.team_count = n
        
        def find(self, v):
            if v == self.parent[v]:
                return v
            self.parent[v] = self.find(self.parent[v])
            return self.parent[v]
    
        def union(self, v, u):
            v = self.find(v)
            u = self.find(u)
            if v == u:
                return
            if self.size[v] < self.size[u]:
                v, u = u, v
            self.parent[u] = v
            self.size[v] += self.size[u]
            self.team_count -= 1
        
        def get_team_count(self):
            return self.team_count
    
        def get_team_size(self, v):
            return self.size[self.find(v)]
        
        def get_team_leaders(self):
            return [v for v in range(node_count) if v == self.parent[v]]
        
    def custom_physics_dsu_score(dsu):
        k = dsu.get_team_count()
        team_leaders = dsu.get_team_leaders()
        team_sizes = np.array([dsu.get_team_size(v) for v in team_leaders])
        b = np.linalg.norm((team_sizes / G.number_of_nodes()) - 1 / k, 2)
        C_w = 0
        for u, v, d in G.edges(data='weight'):
            if dsu.find(u) == dsu.find(v):
                C_w += d
        return C_w + K_COEFFICIENT * math.exp(K_EXP * k) + math.exp(B_EXP * b)
    
    def get_edges():
        edges = []
        for i in range(node_count):
            for j in range(i):
                edges.append((np.linalg.norm(node_pos[i] - node_pos[j]), i, j))
        edges.sort()
        return edges

    def modified_kruskal_1():
        edges = get_edges()
        dsu = DSU(node_count)
        for edge in edges:
            dsu.union(edge[1], edge[2])
            if dsu.get_count() == team_count:
                return [dsu.find(i) for i in range(node_count)]
            
    def modified_kruskal_2():
        edges = get_edges()
        dsu = DSU(node_count)
        while dsu.get_team_count() > team_count:
            smallest_merge = 1e9
            best_edge = None
            for edge in edges:
                if dsu.find(edge[1]) != dsu.find(edge[2]):
                    if smallest_merge > dsu.get_team_size(edge[1]) + dsu.get_team_size(edge[2]):
                        smallest_merge = dsu.get_team_size(edge[1]) + dsu.get_team_size(edge[2])
                        best_edge = edge
            dsu.union(best_edge[1], best_edge[2])
        return [dsu.find(i) for i in range(node_count)]
    
    def modified_kruskal_3():
        edges = get_edges()
        dsu = DSU(node_count)
        while dsu.get_team_count() > team_count:
            smallest_team = 1e9
            best_edge = None
            for edge in edges:
                if dsu.find(edge[1]) != dsu.find(edge[2]):
                    if smallest_team > min(dsu.get_team_size(edge[1]), dsu.get_team_size(edge[2])):
                        smallest_team = min(dsu.get_team_size(edge[1]), dsu.get_team_size(edge[2]))
                        best_edge = edge
            dsu.union(best_edge[1], best_edge[2])
        return [dsu.find(i) for i in range(node_count)]

    def modified_kruskal_4():
        edges = get_edges()
        dsu = DSU(node_count)
        prev_score = float('inf')
        for edge in edges[:999]:
            if dsu.find(edge[1]) != dsu.find(edge[2]):
                dsu2 = DSU(node_count)
                dsu2.parent = dsu.parent.copy()
                dsu2.size = dsu.size.copy()
                dsu2.team_count = dsu.team_count
                dsu2.union(edge[1], edge[2])
                new_score = custom_physics_dsu_score(dsu2)
                if new_score < prev_score:
                    prev_score = new_score
                    dsu = dsu2
        return [dsu.find(i) for i in range(node_count)]
    
    def modified_kruskal_5():
        edges = get_edges()
        dsu = DSU(node_count)
        while dsu.get_team_count() > team_count:
            smallest_score = float('inf')
            best_edge = None
            for edge in edges[:200]:
                if dsu.find(edge[1]) != dsu.find(edge[2]):
                    dsu2 = DSU(node_count)
                    dsu2.parent = dsu.parent.copy()
                    dsu2.size = dsu.size.copy()
                    dsu2.team_count = dsu.team_count
                    dsu2.union(edge[1], edge[2])
                    new_score = custom_physics_dsu_score(dsu2)
                    if new_score < smallest_score:
                        smallest_score = new_score
                        best_edge = edge
            dsu.union(best_edge[1], best_edge[2])
        return [dsu.find(i) for i in range(node_count)]

    teams = modified_kruskal_4()

    def shrink_teams():
        team_dict = dict()
        for i in range(node_count):
            if teams[i] not in team_dict:
                team_dict[teams[i]] = len(team_dict) + 1
        return [team_dict[teams[i]] for i in range(node_count)]

    teams = shrink_teams()
    print(teams)
    
    def assign_teams():
        for i in range(node_count):
            G.nodes[i]['team'] = teams[i]
    
    assign_teams()

def print_teams(G: nx.Graph):
    for i in range(G.number_of_nodes()):
        print(G.nodes[i]['team'], end=' ')
    print()

def run_physics_file():
    IN_FILE = 'tests/small/random_1/'
    G = read_input(IN_FILE + 'graph.in')
    history, node_pos, edge_weights = physics_solve(G)
    get_teams(G, 10, node_pos)
    visualize_physics(history, G.number_of_nodes(), edge_weights, [G.nodes[i]['team'] for i in range(G.number_of_nodes())], show_edges=False)
    # visualize_physics_2d(history[-1], edge_weights, [G.nodes[i]['team'] for i in range(G.number_of_nodes())])
    print(score(G))
    # print_teams(G)

run_physics_file()