import numpy as np

# for a vector t, return the matrix
# [t == 1]
# [t == 2]
# [t == 3]
# ...

t = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
print(t[:, None] == np.arange(1, 11))