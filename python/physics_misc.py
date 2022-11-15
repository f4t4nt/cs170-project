import numpy as np
import vpython as vp

history = []

node_count = 16
edge_weights = [[0 for i in range(node_count)] for j in range(node_count)]
for i in range(node_count):
    edge_weights.append([])
    for j in range(node_count):
        edge_weights[i].append(0)
        if i % 4 != 3 and j == i + 1:
            edge_weights[i][j] = 100
            edge_weights[j][i] = 100
        if i < 12 and j == i + 4:
            edge_weights[i][j] = 100
            edge_weights[j][i] = 100
edge_weights = np.array(edge_weights)

dt = 1e-3
dim = 2
node_pos = np.random.rand(node_count, dim) * 2 - 1
# node_vel = np.zeros((node_count, dim))
node_vel = np.random.rand(node_count, dim) * 2 - 1
vel_decay = 0.99
history.append(node_pos)

def get_acceleration():
    node_accel = np.zeros((node_count, dim))
    for i in range(node_count):
        for j in range(node_count):
            if i != j:
                node_accel[i] += (edge_weights[i][j] - np.linalg.norm(node_pos[i] - node_pos[j])) * (node_pos[i] - node_pos[j]) / np.linalg.norm(node_pos[i] - node_pos[j]) * (100 + edge_weights[i][j])
    for i in range(node_count):
        node_accel[i] += -node_pos[i] / np.linalg.norm(node_pos[i])
    # for i in range(node_count):
    #     closest_node = -1
    #     closest_dist = 1e10
    #     for j in range(node_count):
    #         if i != j and np.linalg.norm(node_pos[i] - node_pos[j]) < closest_dist:
    #             closest_node = j
    #             closest_dist = np.linalg.norm(node_pos[i] - node_pos[j])
    #     node_accel[i] += (node_pos[closest_node] - node_pos[i]) / np.linalg.norm(node_pos[closest_node] - node_pos[i])
    # for i in range(node_count):
    #     for j in range(node_count):
    #         if edge_weights[i][j] == 0 and i != j:
    #             node_accel[i] += (node_pos[j] - node_pos[i]) / np.linalg.norm(node_pos[j] - node_pos[i]) * 2
    #             # node_accel[i] += (node_pos[j] - node_pos[i]) / np.linalg.norm(node_pos[j] - node_pos[i])
    return node_accel

total_steps = 1e4

for step in range(int(total_steps)):
    node_pos = node_pos + node_vel * dt
    node_accel = get_acceleration()
    node_vel = node_vel + node_accel * dt
    node_vel = node_vel * vel_decay
    history.append(node_pos)
    
def vector_to_3d(vector):
    rv = []
    if len(vector) == 1:
        rv = np.append(vector, [0, 0])
    elif len(vector) == 2:
        rv =  np.append(vector, 0)
    else:
        rv = vector[:3]
    return vp.vector(*rv)

def visualize(history):
    vp.scene.width = 1600
    vp.scene.height = 900
    vp.scene.background = vp.color.white
    vp.scene.center = vector_to_3d(np.mean(history[0], axis=0))
    vp.scene.range = 1.5 * np.max(np.linalg.norm(history[0], axis=1))
    vp.scene.autoscale = False
    nodes = []
    for i in range(node_count):
        nodes.append(vp.sphere(pos=vector_to_3d(history[0][i]), radius=0.5, color=vp.color.red))
    edges = dict()
    for i in range(node_count):
        for j in range(node_count):
            if edge_weights[i][j] > 0:
                edges[(i, j)] = vp.cylinder(pos=vector_to_3d(history[0][i]), axis=vector_to_3d(history[0][j] - history[0][i]), radius=0.05, color=vp.color.blue)
    for step in range(len(history)):
        vp.rate(60)
        for i in range(node_count):
            nodes[i].pos = vector_to_3d(history[step][i])
        for i in range(node_count):
            for j in range(node_count):
                if edge_weights[i][j] > 0:
                    edges[(i, j)].pos = vector_to_3d(history[step][i])
                    edges[(i, j)].axis = vector_to_3d(history[step][j] - history[step][i])

visualize(history)