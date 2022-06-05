import unittest
from grid import RectGrid, stabilize, unit_n_ball_volume
import numpy as np
import scipy.sparse as sparse
import matplotlib.pyplot as plt
import itertools

class MiscTests(unittest.TestCase):
    def test_2d_ball_volume(self):
        self.assertAlmostEqual(unit_n_ball_volume(2), np.pi)
    
    def test_3d_ball_volume(self):
        self.assertAlmostEqual(unit_n_ball_volume(3), 4/3*np.pi)

class RectGridTests(unittest.TestCase):

    def setUp(self):
        self.grid = RectGrid()
    
    def test_second_order_difference(self):
        difference = (self.grid.d2 @ self.grid.x)
        self.assertTrue(np.all(difference[1:-1] == 0))

    def test_dimension_operator_shape(self):
        correct_shape = (self.grid.n ** self.grid.n_dimensions,)*2
        self.assertEqual(self.grid.dimension_operator(
            self.grid.d2,
            0
        ).shape, correct_shape)

    def test_laplacian_multiplication(self):
        self.grid.unflatten(
            self.grid.laplacian @ np.cos(self.grid.r)
        )
    
    def test_permutations(self):
        N = self.grid.n ** self.grid.n_dimensions 
        A = self.grid.unflatten(np.arange(N))
        for perm in itertools.permutations(range(self.grid.n_dimensions)):
            perm_op = self.grid.permutation(perm)
            np.testing.assert_array_equal(
                perm_op @ A.reshape(-1),
                A.transpose(perm).reshape(-1)
            )
    
    def test_linear_operator(self):
        N = self.grid.n ** self.grid.n_dimensions 
        A = self.grid.unflatten(np.arange(N))
        for perm in itertools.permutations(range(self.grid.n_dimensions)):
            perm_op = self.grid.permutation(perm)
            perm_inv = [perm.index(i) for i in range(len(perm))]
            matvec = lambda v: self.grid.unflatten(v).transpose(perm).reshape(-1)
            rmatvec = lambda v: self.grid.unflatten(v).transpose(perm_inv).reshape(-1)
            linear_op = sparse.linalg.LinearOperator(
                shape=perm_op.shape,
                matvec=matvec,
                rmatvec=rmatvec)
            np.testing.assert_array_equal(
                perm_op @ A.reshape(-1),
                A.transpose(perm).reshape(-1)
            )
            np.testing.assert_array_equal(
                perm_op @ A.reshape(-1),
                linear_op @ A.reshape(-1),
            )
            np.testing.assert_array_equal(
                perm_op.transpose() @ A.reshape(-1),
                linear_op.transpose() @ A.reshape(-1),
            )
                
    def test_mirrors(self):
        for i in range(self.grid.n_dimensions):
            mirror_op = self.grid.mirror(axis=i)
            x = self.grid.X[i]
            np.testing.assert_array_equal(
                self.grid.unflatten(mirror_op @ x),
                self.grid.unflatten(-x)
            )

    def test_axis_pairs(self):
        self.assertEqual(self.grid.axis_pairs(), [(0,1)])

    def test_expand_collapse_identity(self):
        expand, collapse, mask = self.grid.expand_collapse_ops()
        np.testing.assert_array_equal(
            (collapse @ expand).toarray(),
            sparse.eye(collapse.shape[0]).toarray()
        )
    
    def test_expand_collapse_r(self):
        expand, collapse, mask = self.grid.expand_collapse_ops()
        r_masked = self.grid.r[mask]
        np.testing.assert_array_equal(
            expand @ r_masked,
            self.grid.r
        )

    def test_laplacian_dtype(self):
        self.assertEqual(self.grid.laplacian.dtype, np.int8)
    
    def test_expand_collaps_dtype(self):
        expand, collapse, mask = self.grid.expand_collapse_ops(apply_mask=False)
        self.assertEqual(expand.dtype, np.int8)
        self.assertEqual(collapse.dtype, np.int8)
        self.assertEqual(mask.dtype, bool)
    
    def test_expand_mask(self):
        expand, collapse, mask = self.grid.expand_collapse_ops(apply_mask=False)
        np.testing.assert_array_equal(
            expand @ mask,
            np.ones(mask.shape)
        )
    
    def test_unfold_mask(self):
        _, _, mask = self.grid.expand_collapse_ops(apply_mask=False)
        unfold = self.grid.unfold()
        mask = unfold @ mask
        self.assertTrue(np.all(mask==1))

    def test_stabilize(self):
        pile0 = (self.grid.r * 0).astype(int)
        min_r = np.min(self.grid.r)
        pile0[self.grid.r == min_r] = 64 // sum(self.grid.r == min_r)
        pile, spills = stabilize(
            pile0,
            self.grid.laplacian,
            self.grid.degree
        )
        # test that number of grains is conserved
        self.assertEqual(np.sum(pile0), np.sum(pile))
        # test that number of spills is positive
        self.assertTrue(np.all(spills >= 0))
        # test that all piles are between zero and topple height
        self.assertTrue(np.all(pile >= 0))
        self.assertTrue(np.all(pile < self.grid.degree))

    def test_stabilize_with_condensed_laplacian(self):
        expand, collapse, mask = self.grid.expand_collapse_ops()
        pile = np.zeros(self.grid.r.shape, dtype=int)
        min_r = np.min(self.grid.r)
        pile[self.grid.r == min_r] = 64
        pile1, spills1 = stabilize(
            pile,
            self.grid.laplacian,
            self.grid.degree
        )
        pile2, spills2 = stabilize(
            collapse @ pile,
            collapse @ self.grid.laplacian @ expand,
            collapse @ self.grid.degree
        )
        np.testing.assert_array_equal(
            pile1,
            expand @ pile2
        )
        np.testing.assert_array_equal(
            spills1,
            expand @ spills2
        )
    
    def test_equivalence_of_condensed_laplacian(self):
        expand, collapse, mask = self.grid.expand_collapse_ops()
        laplacian = self.grid.laplacian
        collapsed_laplacian = collapse @ self.grid.laplacian @ expand
        np.testing.assert_array_almost_equal(
            laplacian @ self.grid.r,
            expand @ collapsed_laplacian @ self.grid.r[mask]
        )

        
class RectGridTests3d(RectGridTests):
    
    def setUp(self):
        self.grid = RectGrid(n_dimensions=3)
    
    def test_axis_pairs(self):
        self.assertEqual(self.grid.axis_pairs(), [(0,1), (0,2), (1,2)])

class RectGridTestsEvenN(RectGridTests):
    
    def setUp(self):
        self.grid = RectGrid(n=10)

if __name__ == '__main__':
    unittest.main()
