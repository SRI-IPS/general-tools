# Description:
#   Eigen is a C++ template library for linear algebra: vectors,
#   matrices, and related algorithms.

licenses([
    # Note: Eigen is an MPL2 library that includes GPL v3 and LGPL v2.1+ code.
    #       We've taken special care to not reference any restricted code.
    "reciprocal",  # MPL2
    "notice",  # Portions BSD
])

exports_files(["COPYING.MPL2"])

# License-restricted (i.e. not reciprocal or notice) files...
EIGEN_RESTRICTED_FILES = [
    "Eigen/src/OrderingMethods/Amd.h",
    "Eigen/src/SparseCholesky/**",
    "unsupported/Eigen/src/Eigenvalues/ArpackSelfAdjointEigenSolver.h",
    "unsupported/Eigen/src/IterativeSolvers/ConstrainedConjGrad.h",
    "unsupported/Eigen/src/IterativeSolvers/IterationController.h",
]

# Notable transitive dependencies of restricted files...
EIGEN_RESTRICTED_DEPS = [
    "Eigen/Eigen",
    "Eigen/IterativeLinearSolvers",
    "Eigen/MetisSupport",
    "Eigen/Sparse",
    "Eigen/SparseCholesky",
    "Eigen/SparseLU",
    "unsupported/Eigen/ArpackSupport",
    "unsupported/Eigen/IterativeSolvers",
]

# Note: unsupported/Eigen is unsupported and might go away at any time.
EIGEN_FILES = [
    "Eigen/**",
    "unsupported/Eigen/**",
]

# Files known to be under MPL2 license.
EIGEN_MPL2_HEADER_FILES = glob(
    EIGEN_FILES,
    exclude = EIGEN_RESTRICTED_FILES +
              EIGEN_RESTRICTED_DEPS + [
        # Guarantees any file missed by excludes above will not compile.
        "Eigen/src/Core/util/NonMPL2.h",
        "Eigen/**/CMakeLists.txt",
        "unsupported/Eigen/**/CMakeLists.txt",
    ],
)

cc_library(
    name = "eigen",
    hdrs = EIGEN_MPL2_HEADER_FILES,
    defines = [
        # This define (mostly) guarantees we don't link any problematic
        # code. We use it, but we do not rely on it, as evidenced above.
        "EIGEN_MPL2_ONLY",
    ],
    includes = ["."],
    visibility = ["//visibility:public"],
)
