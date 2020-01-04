# Description:
#   Eigen is a C++ template library for linear algebra: vectors,
#   matrices, and related algorithms.

# TODO Figure out from a lawyer to make sure we're ok here to include the LGPL content
# the FAQ infers that since Eigen is header only, we may be able to include it without having to
# GPL our own code
# http://eigen.tuxfamily.org/index.php?title=Licensing_FAQ&oldid=1117

licenses([
    # Note: Eigen is an MPL2 library that includes GPL v3 and LGPL v2.1+ code.
    #       We've taken special care to not reference any restricted code.
    "reciprocal",  # MPL2
    "notice",  # Portions BSD
    "restricted",  # LGPL
])

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

# Note: unsupported/Eigen is unsupported and might go away at any time
EIGEN_FILES = [
    "Eigen/**",
    "unsupported/Eigen/**",
]

# Currently including LGPL content here
EIGEN_HEADER_FILES = glob(
    EIGEN_FILES + EIGEN_RESTRICTED_FILES + EIGEN_RESTRICTED_DEPS,
    exclude = [
        "Eigen/**/CMakeLists.txt",
        "unsupported/Eigen/**/CMakeLists.txt",
    ],
)

cc_library(
    name = "eigen_restricted",
    hdrs = EIGEN_HEADER_FILES,
    includes = ["."],
    visibility = ["//visibility:public"],
)
