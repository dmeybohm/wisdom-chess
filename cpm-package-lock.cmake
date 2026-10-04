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
# expected
CPMDeclarePackage(expected
  NAME expected
  GIT_TAG v1.3.1
  GITHUB_REPOSITORY TartanLlama/expected
  DOWNLOAD_ONLY YES
)
# nanobench
CPMDeclarePackage(nanobench
  VERSION 4.3.11
  GITHUB_REPOSITORY martinus/nanobench
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
