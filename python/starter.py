import networkx as nx
import numpy as np
import os
import json
import matplotlib.pyplot as plt
from matplotlib import cm
import math
from pathlib import Path
from tqdm.auto import tqdm
import tarfile

# Scoring constants
MAX_WEIGHT = 1000
MAX_EDGES = 10000
N_SMALL = 100
N_MEDIUM = 300
N_LARGE = 1000
K_EXP = 0.5
K_COEFFICIENT = 100
B_EXP = 70

MIN_NET_WEIGHT = MAX_WEIGHT*MAX_EDGES*0.05

INPUT_SIZE_LIMIT = 1000000
OUTPUT_SIZE_LIMIT = 10000


def write_input_helper(G: nx.Graph, path: str):
    if validate_input(G):
        with open(path, 'w') as fp:
            json.dump(nx.node_link_data(G), fp)


def write_input(G: nx.Graph, path: str, overwrite: bool=False, copy: bool=True):
    if not copy:
        if not os.path.exists(path):
            os.makedirs(path)
        path = os.path.join(path, 'graph.in')
        assert overwrite or not os.path.exists(path), \
            'File already exists and overwrite set to False. Move file or set overwrite to True to proceed.'
        if os.path.exists(path):
            print(f'WARNING: Overwriting {path}')
        write_input_helper(G, path)
    else:
        path = path.rstrip('/') + '_1/'
        i = 1
        while os.path.exists(path):
            path = path.rstrip(f'_{i}/') + f'_{i+1}/'
            i += 1
        os.makedirs(path)
        path = os.path.join(path, 'graph.in')
        write_input_helper(G, path)


def read_input(path: str, ignore_net_weight=False):
    assert os.path.getsize(path) < INPUT_SIZE_LIMIT, 'This input file is too large'
    with open(path) as fp:
        G = nx.node_link_graph(json.load(fp))
        if validate_input(G, ignore_net_weight):
            return G


def write_output_helper(G: nx.Graph, path: str):
    if validate_output(G):
        with open(path, 'w') as fp:
            json.dump([G.nodes[v]['team'] for v in range(G.number_of_nodes())], fp)


def write_output(G: nx.Graph, path: str, overwrite=False, copy=True):
    if not copy:
        assert overwrite or not os.path.exists(path), \
            'File already exists and overwrite set to False. Move file or set overwrite to True to proceed.'
        if os.path.exists(path):
            print(f'WARNING: Overwriting {path}')
        write_output_helper(G, path)
    else:
        path = Path(path)
        name = path.stem
        suffix = path.suffix
        i = 1
        while os.path.exists(path):
            path = path.parent / f'{name}_{i}{suffix}'
            i += 1
        write_output_helper(G, path)


def read_output(G: nx.Graph, path: str):
    assert os.path.getsize(path) < OUTPUT_SIZE_LIMIT, 'This output file is too large'
    with open(path) as fp:
        l = json.load(fp)
        assert isinstance(l, list), 'Output partition must be a list'
        assert set(G) == set(range(len(l))), 'Output does not match input graph'
        nx.set_node_attributes(G, {v: l[v] for v in G}, 'team')
        if validate_output(G):
            return G


def validate_graph(G: nx.Graph):
    assert not G.is_directed(), 'G should not be directed'
    assert set(G) == set(range(G.number_of_nodes())), 'Nodes must be numbered from 0 to n-1'
    return True


def validate_input(G: nx.Graph, ignore_net_weight=False):
    for n, d in G.nodes(data=True):
        assert not d, 'Nodes cannot have data'
    for u, v, d in G.edges(data=True):
        assert u != v, 'Edges should be between distinct vertices (a penguin is experiencing inner-conflict)'
        assert set(d) == {'weight'}, 'Edge must only have weight data'
        assert isinstance(d['weight'], int), 'Edge weights must be integers'
        assert d['weight'] > 0, 'Edge weights must be positive'
        assert d['weight'] <= MAX_WEIGHT, f'Edge weights cannot be greater than {MAX_WEIGHT}'
    assert G.number_of_edges() <= MAX_EDGES, 'Graph has too many edges'
    net_weight = sum(d for u, v, d in G.edges(data='weight'))
    # assert ignore_net_weight or net_weight >= MIN_NET_WEIGHT, \
    #     f'There must be at least {MIN_NET_WEIGHT} edge weight in the input, but there is only {net_weight}'
    if not ignore_net_weight and net_weight < MIN_NET_WEIGHT:
        print(f'WARNING: There is only {net_weight} edge weight in the input, but there should be at least {MIN_NET_WEIGHT}')
    return validate_graph(G)


def validate_output(G: nx.Graph):
    for n, d in G.nodes(data=True):
        assert set(d) == {'team'}, 'Nodes must have team data'
        assert isinstance(d['team'], int), 'Team identifier must be an integer'
        assert d['team'] > 0, 'Team identifier must be greater than 0'
        assert d['team'] <= G.number_of_nodes(), 'Team identifier unreasonably large'
    return validate_graph(G)


def score(G: nx.Graph, separated=False):
    output = [G.nodes[v]['team'] for v in range(G.number_of_nodes())]
    teams, counts = np.unique(output, return_counts=True)

    k = np.max(teams)
    b = np.linalg.norm((counts / G.number_of_nodes()) - 1 / k, 2)
    C_w = sum(d for u, v, d in G.edges(data='weight') if output[u] == output[v])

    if separated:
        return C_w, K_COEFFICIENT * math.exp(K_EXP * k), math.exp(B_EXP * b)
    return C_w + K_COEFFICIENT * math.exp(K_EXP * k) + math.exp(B_EXP * b)


def visualize(G: nx.Graph, path: str, show=False):
    output = G.nodes(data='team', default=0)
    partition = dict()
    for n, t in output:
        if t not in partition:
            partition[t] = []
        partition[t].append(n)

    pos = dict()
    circle_size = len(partition) * 0.5
    for k, v in partition.items():
        pos.update(nx.shell_layout(G, nlist=[v], center=(circle_size*math.cos(math.tau*k / len(partition)),
                                                         circle_size*math.sin(math.tau*k / len(partition)))))

    crossing_edges = [e for e in G.edges(data='weight') if output[e[0]] != output[e[1]]]
    within_edges = [e for e in G.edges(data='weight') if output[e[0]] == output[e[1]]]
    max_weight = max(nx.get_edge_attributes(G, name='weight').values())

    nx.draw_networkx_nodes(G, pos, node_color=[output[n] for n in G],
                           cmap=cm.get_cmap('tab20b'))
    nx.draw_networkx_labels(G, pos, font_size=10, font_color="white")

    nx.draw_networkx_edges(G, pos, edgelist=crossing_edges, edge_color=[x[2] for x in crossing_edges],
                           edge_cmap=cm.get_cmap('Blues'), edge_vmax=max_weight*1.5, edge_vmin=max_weight*-0.2)
    nx.draw_networkx_edges(G, pos, width=2, edgelist=within_edges, edge_color=[x[2] for x in within_edges],
                           edge_cmap=cm.get_cmap('Reds'), edge_vmax=max_weight*1.5, edge_vmin=max_weight*-0.2)

    plt.tight_layout()
    plt.axis("off")
    if show:
        plt.savefig(path)
    plt.show()


def run(solver, in_file: str, out_file: str, overwrite: bool=False, copy: bool=False):
    instance = read_input(in_file)
    output = solver(instance)
    if output:
        instance = output
    write_output(instance, out_file, overwrite, copy)
    print(f"{str(in_file)}: cost", score(instance))


def run_all(solver, in_dir, out_dir, overwrite: bool=False):
    for root, dirs, files in os.walk(in_dir):
        for file in files:
            if file == 'graph.in':
                in_file = os.path.join(root, file)
                out_file = os.path.join(out_dir, os.path.basename(os.path.dirname(in_file)) + '.out')
                run(solver, in_file, out_file, overwrite)


def tar(out_dir, overwrite=False):
    path = f'../submission.tar'
    assert overwrite or not os.path.exists(path), \
        'File already exists and overwrite set to False. Move file or set overwrite to True to proceed.'
    if os.path.exists(path):
        print(f'Overwriting {path}')
    with tarfile.open(path, 'w') as fp:
        fp.add(out_dir)