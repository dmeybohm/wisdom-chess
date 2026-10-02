# CPM Package Lock
# This file should be committed to version control

# doctest
CPMDeclarePackage(doctest
  NAME doctest
  GIT_TAG v2.4.12
  GITHUB_REPOSITORY doctest/doctest
  OPTIONS
    "DOCTEST_NO_INSTALL On"
)
# nanobench
CPMDeclarePackage(nanobench
  VERSION 4.3.11
  GITHUB_REPOSITORY martinus/nanobench
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
