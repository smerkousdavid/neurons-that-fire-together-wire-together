import numpy as np

def construct_hebb_matrix(xi):
    n = xi.shape[0]
    if len(xi.shape) == 1:
        w = np.outer(xi, xi) / n  # p = 1
    elif len(xi.shape) == 2:
        w = np.einsum("ik,jk", xi, xi) / n  # p > 1
    else:
        raise ValueError("Unexpected shape of input pattern xi: {}".format(xi.shape))
    np.fill_diagonal(w, 0)  # set diagonal elements to zero
    return w


N = 5
W = np.zeros((N, N), dtype=np.float64)
patterns = [
    np.array([-1, 1, 1, 1, -1]),
    np.array([-1, -1, -1, -1, -1])
]

# for p in patterns:
#     W += construct_hebb_matrix(p)
for pattern in patterns:
    for i in range(len(pattern)):
        for j in range(len(pattern)):
            if i==j:
                W[i][j] = 0
            else:
                w_ij = pattern[i]*pattern[j]
                W[i][j] += w_ij
                W[j][i] += w_ij

print(W / len(patterns))