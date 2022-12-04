from starter import *

target_score = 22946.6169230646

FILE = '23132_rocky'
TEST_SIZE = 'medium'
TEST_ID = 197
TEST_FOLDER = './tests/' + TEST_SIZE + '/' + TEST_SIZE + str(TEST_ID) + '/'
IN_FILE = TEST_FOLDER + 'graph.in'
OUT_FILE = TEST_FOLDER + FILE + '_int.out'
G = read_input(IN_FILE)
read_output(G, OUT_FILE)
W, K, B = score(G, separated=True)

output = [G.nodes[v]['team'] for v in range(G.number_of_nodes())]
teams, counts = np.unique(output, return_counts=True)
team_count = np.max(teams)

print(f'K: {K}, B: {B}, W: {W}, score: {W + K + B}')

true_target_score = target_score - K - B
assert abs(true_target_score - round(true_target_score)) < 1e-6
true_target_score = round(true_target_score)

print(f'Target score: {true_target_score}')
edge_weights = [w['weight'] for u, v, w in G.edges(data=True)]
edge_weights.sort(reverse=True)
edge_counts = dict()
for w in edge_weights:
    edge_counts[w] = edge_counts.get(w, 0) + 1

team_assignments = [-1] * G.number_of_nodes()
team_counts = [0] * team_count