import numpy as np
import time
import itertools
from functools import reduce
import matplotlib.pyplot as plt

from scipy.special import gamma
from scipy.sparse import diags, eye, kron, coo_array, lil_array, bmat, csr_array
from scipy.sparse.linalg import spsolve
from functools import cached_property
import scipy.optimize

class RectGrid:
    '''
    
    '''
    def __init__(self, n=9, n_dimensions=2):
        self.n = n
        self.n_dimensions = n_dimensions

        self.x = np.arange(n) - (n-1) / 2
        self.X = np.meshgrid(
            *list(self.x for _ in range(n_dimensions)),
            indexing='ij'

        )
        self.X = [x.reshape(-1) for x in self.X]
        self.r = np.sqrt(sum(x**2 for x in self.X))

    @cached_property
    def d2(self):
        return diags((1, -2, 1), (-1, 0, 1), shape=(self.n, self.n), dtype=np.int8)

    @cached_property
    def laplacian(self):
        return sum(
            self.dimension_operator(self.d2, i)
                for i in range(self.n_dimensions)
        ).astype(np.int8)
    
    @cached_property
    def degree(self):
        return - self.laplacian.diagonal()

    def mirror(self, axis=0, format='coo'):
        return self.permutation_matrix(
            lambda A: np.flip(A, axis=axis)
        ).asformat(format)

    def permutation(self, perm, format='coo'):
        return self.permutation_matrix(
            lambda A: np.transpose(A, perm)
        ).asformat(format)

    def permutation_symmetries(self, format='coo'):
        ops = []
        for perm in itertools.permutations(range(self.n_dimensions)):
            ops.append(self.permutation(perm, format=format))
        return ops

    def diagonal_unfold(self):
        perms = itertools.permutations(range(self.n_dimensions))
        return self.permutation_matrix(
            *(lambda A, perm=perm: np.transpose(A, perm) for perm in perms)
        ).asformat('csr').astype(bool).astype(np.int8)
    
    def mirror_unfold(self, axis=0):
        return self.permutation_matrix(
            lambda A: np.flip(A, axis=axis),
            lambda A: A
        ).asformat('csr').astype(bool).astype(np.int8)

    def unfold(self):
        unfold = self.eye()
        for i in range(self.n_dimensions):
            unfold @= self.mirror_unfold(axis=i)
        unfold @= self.diagonal_unfold()
        return unfold

    def mirror_symmetries(self, format='coo'):
        N = self.n ** self.n_dimensions
        ops = []
        for i in range(self.n_dimensions):
            ops.append([
                self.mirror(axis=i),
                eye(N, format=format)
            ])
        return ops
    
    def axis_pairs(self):
        return list(itertools.combinations(range(self.n_dimensions), 2))

    def permutation_matrix(self, *fns):
        '''
        returns sparse permutation matrix with rearrangement
        defined by fns

        each fn needs to accept an nd array of indices and return an
        array of the same shape with elements rearranged
        '''
        Nfn = len(fns)
        N = self.n ** self.n_dimensions
        A = np.arange(N)
        A = self.unflatten(A)
        return coo_array(
            (
                np.tile(np.ones(A.shape).reshape(-1), (Nfn)),
                (
                    np.tile(A.reshape(-1), (Nfn)),
                    np.concatenate([fn(A).reshape(-1) for fn in fns])
                )
            ),
            shape=(N, N)
        )

    def unflatten(self, arr):
        return arr.reshape(*(self.n for _ in range(self.n_dimensions)))

    def dimension_operator(self, op, index):
        out = eye(1)
        for i in range(self.n_dimensions):
            out = kron(
                out, 
                op if i == index else eye(self.n, dtype=np.int8)
            )
        return out

    def symmetries(self, format='coo'):
        return [self.permutation_symmetries(format=format)] + \
            self.mirror_symmetries(format=format)

    def eye(self):
        N = self.n ** self.n_dimensions
        return eye(N, dtype=np.int8)

    def expand_collapse_ops(self, apply_mask=True):
        N = self.n ** self.n_dimensions
        mask = np.full(N, True)

        for i in range(self.n_dimensions):
            mask &= (self.X[i] >= 0)
        for i, j in self.axis_pairs():
            mask &= (self.X[i] >= self.X[j])
       
        expand = self.unfold()
        collapse = eye(N, format='csr', dtype=np.int8)
        if apply_mask:
            expand = expand[:, mask]
            collapse = collapse[mask, :]

        return expand, collapse, mask
    
    def _plot_2d(self, array):
        plt.imshow(
            self.unflatten(array),
            extent=(
                np.min(self.x),
                np.max(self.x),
                np.min(self.x),
                np.max(self.x)
            )
        )

    def _plot_3d(self, array, slice=False):
        if slice:
            array = self.unflatten(array)[:,:,self.n//2]
            plt.imshow(
                array,
                extent=(
                    np.min(self.x),
                    np.max(self.x),
                    np.min(self.x),
                    np.max(self.x)
                )
            )
            return
        voxelarray = self.unflatten(array)
        voxelarray[self.unflatten(self.X[2]) > 0] = 0
        colors = np.empty(voxelarray.shape, dtype=object)
        colors[voxelarray==1] = 'red'
        colors[voxelarray==2] = 'blue'
        colors[voxelarray==3] = 'green'
        colors[voxelarray==4] = 'yellow'
        colors[voxelarray==5] = 'orange'
        ax = plt.figure().add_subplot(projection='3d')
        ax.voxels(
            voxelarray,
            facecolors=colors,
            edgecolor=None,
        )

    def plot(self, array, **kwargs):
        if self.n_dimensions == 2:
            self._plot_2d(array)
        elif self.n_dimensions == 3:
            self._plot_3d(array, **kwargs)
        else:
            raise NotImplementedError

def duplicate_non_ones(mat):
    data = np.ones(np.sum(mat.data), dtype=mat.data.dtype)
    indices = np.repeat(mat.indices, mat.data)
    indptr = np.cumsum(np.sum(np.pad(mat.toarray(), pad_width=(1,0)), axis=0))
    return csr_array((data, indices, indptr), mat.shape)

from itertools import chain
use_cython = True
if use_cython:
    from stabilize import _stabilize
    def stabilize(pile, laplacian, degree=4, initial_spills=None, completion_check=1):
        laplacian = laplacian.tocsc()
        laplacian.setdiag(0, k=0)
        laplacian.eliminate_zeros()
        print(sum(laplacian.data > 1) / len(laplacian.data), 'fraction non one elements in data')
        #laplacian = duplicate_non_ones(laplacian)
        print(sum(laplacian.data > 1) / len(laplacian.data), 'fraction non one elements in data')
        t0 = time.time()
        iterations = _stabilize(pile, laplacian.data, laplacian.indices, laplacian.indptr)
        t1 = time.time()
        print('stabilization alone took', t1-t0)
        print(iterations, 'iterations')
        print((t1-t0)/iterations, 's per iteration')
        print((t1-t0)/iterations/len(pile), 's per inner loop executions')
        return pile, 0*pile
else:
    def stabilize(pile, laplacian, degree=4, initial_spills=None, completion_check=1):
        if initial_spills is None:
            spills = 0 * pile
        else:
            spills = initial_spills.copy()
        i = 0
        while True:
            spillover = laplacian @ spills
            topple = (spillover + pile) // degree
            spills = spills + topple
            i += 1
            if i % completion_check == 0:
                if np.any(topple):
                    i += 1
                else:
                    break
        print('iterations', i)

        return spillover + pile, spills


def unit_n_ball_volume(n_dimensions):
    return np.pi ** (n_dimensions / 2) / gamma(n_dimensions / 2 + 1)


CRITICAL_DENSITIES = {
    2: 2.125,
    3: 3.5,
}

def sandpile(
    N=2**12,
    n_dimensions=2,
    density0=3,
    density1=3,
    use_symmetry=True,
    initial_guess=False,
    completion_check=1,
):
    R = (N / unit_n_ball_volume(n_dimensions)) ** (1 / n_dimensions)
    print('R', R)
    critical_density = CRITICAL_DENSITIES[n_dimensions]
    Rinner = R / (np.ceil(critical_density)) ** (1 / n_dimensions)
    Router = R / (np.floor(critical_density)) ** (1 / n_dimensions)
    Rtarget = R / density0 ** (1 / n_dimensions)
    print('outer R', Router)
    n = 2*int(Router) + 1
    print('n', n)
    grid = RectGrid(n, n_dimensions)

    pile = np.zeros(grid.r.shape, dtype=int)
    pile[grid.r==0] = N
    assert np.sum(pile) == N

    laplacian = grid.laplacian
    degree = grid.degree
    r = grid.r

    t0 = time.time()
    if use_symmetry:
        t_symmetry_ops = time.time()
        expand, collapse, mask = grid.expand_collapse_ops()
        laplacian = (collapse @ laplacian @ expand)
        pile = collapse @ pile
        r = collapse @ grid.r
        degree = collapse @ degree
        print('symmetry ops took', time.time() - t_symmetry_ops)

    spills0 = 0 * pile
    if initial_guess:
        target_mask = (r < Rtarget)
        c = np.ones((1, laplacian.shape[0]))
        spills0 = 0.0*pile
        spills0[target_mask] = spsolve(
            laplacian[target_mask,:][:,target_mask],
            density1-pile[target_mask]
        )
        spills0 = spills0.astype(int)

        i = 0
        while True:
            spillover = laplacian @ spills0
            topple = (spillover + pile) // degree
            if np.any(topple < 0):
                topple[topple>0] = 0
                spills0 = spills0 + topple
                i += 1
            else:
                print(i, 'negative toppling rounds')
                break
        pile += spillover
   
    pile = pile.astype(np.int8)
    print('format', laplacian.format)
    print('pile dtype', pile.dtype)
    print('laplacian dtype', laplacian.dtype)
    print('pile height', np.max(pile))

    
    pile, spills = stabilize(
        pile,
        laplacian,
        initial_spills=spills0,
        degree=degree,
        completion_check=completion_check
    )

    if use_symmetry:
        pile = expand @ pile
        spills = expand @ spills
        spills0 = expand @ spills0
    t1 = time.time()
    print('stabilization took', t1-t0)
    print('chip accounting', np.sum(pile) - N)

    grid.plot(pile, slice=True)
    plt.show()



if __name__ == "__main__":
    sandpile(
        2**18,
        n_dimensions=2,
        use_symmetry=True,
        initial_guess=True,
        density0=3,
        density1=3,
        completion_check=100
    )
    print(19146778996)
