# Test harness skeleton. No test cases here yet: the harness is a separate item.

enable_testing()

# Must be set BEFORE include(CTest). CTest sets this as a cache entry at include time
# with a default of 1500 seconds, and the cache value set here is kept because the
# CTest set does not use FORCE. Fixing it after the include has no effect.
set(DART_TESTING_TIMEOUT 300 CACHE STRING
  "Maximum time allowed before CTest will kill the test.")

include(CTest)
