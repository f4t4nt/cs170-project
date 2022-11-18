import os
from starter import tar

for size in ['small', 'medium', 'large']:
    for test in range(1, 261):
        for filename in os.listdir('./tests/' + size + '/' + size + str(test)):
            if filename.endswith('.out'):
                file = filename.split('.')[0]
                file = open('./tests/' + size + '/' + size + str(test) + '/' + file + '.out', 'r')
                result = file.read()
                file.close()
                file = open('./phase_2/' + size + str(test) + '.out', 'w')
                file.write(result)
                file.close()
                break

tar('./phase_2/', overwrite=True)