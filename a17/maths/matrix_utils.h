#pragma once

#include "Eigen/Core"

namespace a17 {
namespace maths {

/** @brief Create the cross product matrix V of the input vec, such that V * other = vec x other
 *  where "other" is any 3D vector.
 */
template <typename Derived>
Eigen::Matrix<typename Derived::Scalar, 3, 3> SkewSymmetricMatrix(
    const Eigen::MatrixBase<Derived> &vec) {
  EIGEN_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(Derived, 3);

  Eigen::Matrix<typename Derived::Scalar, 3, 3> mat;
  mat << 0, -vec(2), vec(1), vec(2), 0, -vec(0), -vec(1), vec(0), 0;
  return mat;
}

/** @brief Compute the adjugate of a square matrix, see
 * https://en.wikipedia.org/wiki/Adjugate_matrix.
 */
template <typename Derived>
typename Derived::PlainObject Adjugate(const Eigen::MatrixBase<Derived> &mat) {
  // Static check for squareness. Not perfect, if one dimension is Dynamic and the other is static,
  // this will always fail.
  EIGEN_STATIC_ASSERT(Eigen::MatrixBase<Derived>::RowsAtCompileTime ==
                          Eigen::MatrixBase<Derived>::ColsAtCompileTime,
                      THIS_METHOD_IS_ONLY_FOR_MATRICES_OF_A_SPECIFIC_SIZE);

  // Dynamic check for squareness.
  if (mat.rows() != mat.cols()) {
    throw std::invalid_argument("Input must be a square matrix");
  }

  auto det = mat.determinant();
  if (det != 0) {
    return det * mat.inverse();
  }

  // Matrix is not invertible, do the slow manual computation of the adjugate.
  using DynamicMatrix = Eigen::Matrix<typename Derived::Scalar, Eigen::Dynamic, Eigen::Dynamic>;
  typename Derived::PlainObject cofactors(mat.rows(), mat.cols());

  for (int r = 0; r < mat.rows(); ++r) {
    const bool even_row = r % 2 == 0;
    for (int c = 0; c < mat.cols(); ++c) {
      DynamicMatrix temp(mat.rows() - 1, mat.cols() - 1);
      int br_rows = mat.rows() - (r + 1);
      int br_cols = mat.cols() - (c + 1);

      // Top left
      temp.block(0, 0, r, c) = mat.block(0, 0, r, c);

      // Top right
      temp.block(0, c, r, br_cols) = mat.block(0, c + 1, r, br_cols);

      // Bottom left
      temp.block(r, 0, br_rows, c) = mat.block(r + 1, 0, br_rows, c);

      // Bottom right
      temp.block(r, c, br_rows, br_cols) = mat.block(r + 1, c + 1, br_rows, br_cols);

      const bool even_col = c % 2 == 0;
      const int sign = (even_row == even_col) ? 1 : -1;

      cofactors(r, c) = sign * temp.determinant();
    }
  }

  return cofactors.transpose();
}

}  // namespace maths
}  // namespace a17
