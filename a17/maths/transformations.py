import numpy


def affine_transform_inverse(np_transform):
    """Returns a new matrix that is the inverse of the given np_transform. Rather than simply using
        numpy.linalg.inv(), use this for homegeneous matrices to get the proper inverse.

    Args:
        np_transform(numpy.nparray): A 4x4 matrix representing an affine transform.

    Returns:
        (numpy.nparray): The inverse of the given homogeneous matrix.
    """
    rotation = np_transform[:3, :3]
    translation = np_transform[:3, 3]
    rotation_inv = numpy.linalg.inv(rotation)
    translation_inv = -1 * numpy.dot(rotation_inv, translation)
    result = numpy.identity(4)
    result[:3, :3] = rotation_inv
    result[:3, 3] = translation_inv.flatten()
    return result
