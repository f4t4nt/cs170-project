import os
from starter import *

hidden = ['insurance', 'wrath_of_god', 'sick0', 'sick1', 'sick2']
used = set()

for size in ['small', 'medium', 'large']:
    for test in range(1, 261):
        best_file = 'inf.in'
        best_score = float('inf')
        folder = './tests/' + size + '/' + size + str(test)
        G = read_input(folder + '/graph.in')
        for filename in os.listdir(folder):
            if filename.endswith('.out') and float((filename.split('_')[0]).split('.')[0]) <= \
                    float((best_file.split('_')[0]).split('.')[0]) and \
                    filename.split('_')[1][:-4] not in hidden:
                read_output(G, folder + '/' + filename)
                validate_output(G)
                score_G = score(G)
                if round(score_G) != float((filename.split('_')[0]).split('.')[0]):
                    print('Score mismatch, file: ' + folder + '/' + filename + ', score: ' + str(score_G))
                else:
                    best_file = filename
                    best_score = score_G
        new_used = best_file.split('_')[1:]
        new_used = '_'.join(new_used)
        used.add(new_used)
        file = best_file.split('.')[0]
        file = open('./tests/' + size + '/' + size + str(test) + '/' + file + '.out', 'r')
        result = file.read()
        file.close()
        file = open('./phase_2/' + size + str(test) + '.out', 'w')
        file.write(result)
        file.close()

print(used)
tar('./phase_2/', overwrite=True)