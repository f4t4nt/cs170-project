import os
from starter import tar

hidden = ['insurance', 'wrath_of_god', 'sick0', 'sick1']

for size in ['small', 'medium', 'large']:
    for test in range(1, 261):
        best_file = 'inf.in'
        for filename in os.listdir('./tests/' + size + '/' + size + str(test)):
            if filename.endswith('.out') and float((filename.split('_')[0]).split('.')[0]) < \
                    float((best_file.split('_')[0]).split('.')[0]) and filename.split('_')[1][:-4] not in hidden:
                best_file = filename
        file = best_file.split('.')[0]
        file = open('./tests/' + size + '/' + size + str(test) + '/' + file + '.out', 'r')
        result = file.read()
        file.close()
        file = open('./phase_2/' + size + str(test) + '.out', 'w')
        file.write(result)
        file.close()

tar('./phase_2/', overwrite=True)