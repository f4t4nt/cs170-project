import os
from starter import tar

for size in ['small', 'medium', 'large']:
    for test in range(1, 261):
        best_file = '999999999999999_.in'
        for filename in os.listdir('./tests/' + size + '/' + size + str(test)):
            if filename.endswith('.out') and int(filename.split('_')[0]) < int(best_file.split('_')[0]):
                best_file = filename
        file = best_file.split('.')[0]
        file = open('./tests/' + size + '/' + size + str(test) + '/' + file + '.out', 'r')
        result = file.read()
        file.close()
        file = open('./phase_2/' + size + str(test) + '.out', 'w')
        file.write(result)
        file.close()

tar('./phase_2/', overwrite=True)