# This file is based off of https://github.com/ApolloAuto/apollo/blob/master/tools/cpplint.bzl
# -*- python -*-

# From https://bazel.build/versions/master/docs/be/c-cpp.html#cc_library.srcs
_CPP_SOURCE_EXTENSIONS = [source_ext for source_ext in """
.c
.cc
.cpp
.cxx
.c++.C
.h
.hh
.hpp
.hxx
.inc
""".split("\n") if len(source_ext)]

# The cpplint.py command-line argument so it doesn't skip our files!
_EXTENSIONS_ARGS = ["--extensions=" + ",".join(
    [ext[1:] for ext in _CPP_SOURCE_EXTENSIONS],
)]

def _extract_labels(srcs):
  """Convert a srcs= or hdrs= value to its set of labels."""
  # Tuples are already labels.
  if type(srcs) == type(()):
    return list(srcs)
  return []

def _is_cpp_source_label(label):
  for extension in _CPP_SOURCE_EXTENSIONS:
      if label.endswith(extension):
        return True
  return False

def _is_py_source_label(label):
  if label.endswith(".py"):
      return True
  return False

def _add_ccplint_linter_rules(source_labels, source_filenames, name, data=None):
  # Common attributes for all of our py_test invocations.
  data = (data or [])
  size = "small"
  tags = ["cpplint"]

  # Google cpplint.
  cpplint_cfg = ["//:CPPLINT.cfg"] + native.glob(['CPPLINT.cfg'])
  native.py_test(
    name = name + "_cpplint",
    srcs = ["@cpplint_archive//:cpplint"],
    data = data + cpplint_cfg + source_labels,
    args = _EXTENSIONS_ARGS + source_filenames,
    main = "cpplint.py",
    size = size,
    tags = tags,
  )

def _add_pylint_linter_rules(source_labels, source_filenames, name, extra_args=None):
  # Common attributes for all of our py_test invocations.
  extra_args = (extra_args or [])
  size = "small"
  tags = ["pycodestyle"]

  pylint_cfg = ["//:pycodestyle.cfg"]
  pylint_cfg_args = ["--config $(location //:pycodestyle.cfg) "] + extra_args
  native.py_test(
    name = name + "_pycodestyle",
    srcs = ["@pycodestyle_archive//:pycodestyle"],
    data = pylint_cfg + source_labels,
    args = pylint_cfg_args + source_filenames,
    main = "pycodestyle.py",
    size = size,
    tags = tags,
  )

def cpplint(data=None, extra_srcs=None):
  """For every rule in the BUILD file so far, adds a test rule that runs
  cpplint over the C++ sources listed in that rule.  Thus, BUILD file authors
  should call this function at the *end* of every C++-related BUILD file.
  By default, only the CPPLINT.cfg from the project root and the current
  directory are used.  Additional configs can be passed in as data labels.
  Sources that are not discoverable through the "sources so far" heuristic can
  be passed in as extra_srcs=[].
  """
  # Iterate over all rules.
  for rule in native.existing_rules().values():
    # Extract the list of C++ source code labels and convert to filenames.
    candidate_labels = (
      _extract_labels(rule.get("srcs", ())) +
      _extract_labels(rule.get("hdrs", ()))
    )
    source_labels = [
      label for label in candidate_labels
      if _is_cpp_source_label(label)
    ]
    source_filenames = ["$(location %s)" % x for x in source_labels]

    # Run the cpplint checker as a unit test.
    if len(source_filenames) > 0:
      _add_ccplint_linter_rules(source_labels, source_filenames, rule["name"], data)

  # Lint all of the extra_srcs separately in a single rule.
  if extra_srcs:
    source_labels = extra_srcs
    source_filenames = ["$(location %s)" % x for x in source_labels]
    _add_ccplint_linter_rules(source_labels, source_filenames,
                      "extra_srcs_cpplint", data)

def pylint(extra_args=None, extra_srcs=None):
  """For every rule in the BUILD file so far, adds a test rule that runs
  pycodestyle over the python sources listed in that rule.  Thus, BUILD
  file authors should call this function at the *end* of every
  python-related BUILD file. By default, only the pycodestyle.cfg from
  the project root is used to configure the linter. Additional configs
  can be passed in as extra_args=[]. Sources that are not discoverable
  through the "sources so far" heuristic can be passed in as extra_srcs=[].
  """
  # Iterate over all rules.
  for rule in native.existing_rules().values():
    # Extract the list of C++ source code labels and convert to filenames.
    candidate_labels = (
      _extract_labels(rule.get("srcs", ()))
    )
    source_labels = [
      label for label in candidate_labels
      if _is_py_source_label(label)
    ]
    source_filenames = ["$(location %s)" % x for x in source_labels]

    # Run the pylint checker as a unit test.
    if len(source_filenames) > 0:
      _add_pylint_linter_rules(source_labels, source_filenames, rule["name"], extra_args=extra_args)

  # Lint all of the extra_srcs separately in a single rule.
  if extra_srcs:
    source_labels = extra_srcs
    source_filenames = ["$(location %s)" % x for x in source_labels]
    _add_pylint_linter_rules(source_labels, source_filenames,
                      "extra_srcs_cpplint", extra_args=extra_args)
