import vpython as vp
import pickle as pkl
from starter import *

with open('./python/history.pkl', 'rb') as f:
    (V, d, history) = pkl.load(f)

print(f'Number of nodes: {V}')
scene = vp.canvas()
scene.width = 1600
scene.height = 1200
# scene.autoscale = True
nodes = []
print('Creating nodes...')
# links = []
for i in range(V):
    nodes.append(vp.sphere(pos=vp.vector(*d[i]), radius=1, color=vp.color.rgb_to_hsv(vp.vector(i / V, 0, 1 - i / V))))
# for u, v in G.edges:
#     links.append(vp.cylinder(pos=nodes[u].pos, axis=nodes[v].pos - nodes[u].pos, radius=0.01))
print('Done.')
i = 0
while i < len(history):
    vp.rate(30)
    for j in range(V):
        nodes[j].pos = vp.vector(*history[i][j])
    i += 1
    # for j in range(len(links)):
    #     u, v = list(G.edges)[j]
    #     links[j].pos = nodes[u].pos
    #     links[j].axis = nodes[v].pos - nodes[u].pos