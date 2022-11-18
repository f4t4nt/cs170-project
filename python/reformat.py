import os

for size in ['small', 'medium', 'large']:
    for filename in os.listdir('./tests/' + size):
        if filename.endswith('.in'):
            file = filename.split('.')[0]
            os.makedirs('./tests/' + size + '/' + file)
            os.rename('./tests/' + size + '/' + filename, './tests/' + size + '/' + file + '/graph.in')