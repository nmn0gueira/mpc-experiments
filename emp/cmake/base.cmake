# ##############################################################################
# WARNINGS
# ##############################################################################

# Feel free to add to these. Note that GCC and Clang each support different
# flags, so we'll only add extra flags if GCC (the GNU compiler) is detected.
set(WARNINGS
    -Wall
    #-Werror
    -Wextra
    -Wshadow
    -Wpedantic
    -Wextra-semi
    -Wredundant-decls
    -Wctor-dtor-privacy
    -Wfloat-equal
    -Wformat-security
    -Wnon-virtual-dtor
    -Wunused
    # These are omitted just so that we can continue piecewise developing.
    -Wno-error=unused-parameter    
    -Wno-error=return-type
    -Woverloaded-virtual
    -Wsign-conversion
    -Wconversion
    -Wmisleading-indentation
    -Wnull-dereference
    -Wshift-overflow
    -Wstrict-overflow=2
    -Wdouble-promotion
    -Wformat=2
    -Wcast-qual
    -Wmissing-declarations
    -Wsign-promo)


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  list(
    APPEND
    WARNINGS
    -Wstrict-null-sentinel
    -Wformat-signedness
    -Wformat-overflow
    -Wduplicated-branches
    -Wlogical-op
    -Wuseless-cast)
endif()

# Note that the debug flags also adds debug info for standard library calls:
# this is primarily to make sure that we aren't violating any standard library assumptions.
# This comes at a pretty big performance cost though.

set(G_C_DEBUG_FLAGS "-ggdb -fno-omit-frame-pointer -fstack-protector-all" "${WARNINGS}")
set(G_C_OPT_FLAGS "-Ofast -DNDEBUG" "${WARNINGS}")

# Clang doesn't support -mno-avx512f
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  list(APPEND G_C_DEBUG_FLAGS -mno-avx512f)
  list(APPEND G_C_OPT_FLAGS -mno-avx512f)
endif()

# Same problem as above, but with optimisation flags.
# These are primarily needed for e.g inlining forcibly inlined functions
# such as machine intrinsics. Note that AppleClang needs to be handled separately
# because it doesn't support march=native, so we just assume you're using an M1
# and hope for the best.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  list(APPEND G_C_OPT_FLAGS
    -march=native)
  list(APPEND G_C_DEBUG_FLAGS
    -march=native)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  list(APPEND G_C_OPT_FLAGS
    -mcpu=apple-m1)
  list(APPEND G_C_DEBUG_FLAGS
    -mcpu=apple-m1)
endif()

# This splits up the arguments into a form that CMAKE can deal with when passing
# to executables
separate_arguments(C_DEBUG_FLAGS UNIX_COMMAND "${G_C_DEBUG_FLAGS}")
separate_arguments(C_OPT_FLAGS UNIX_COMMAND "${G_C_OPT_FLAGS}")