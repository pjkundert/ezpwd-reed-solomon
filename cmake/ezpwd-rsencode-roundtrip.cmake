# 
# ezpwd-rsencode-roundtrip.cmake -- portable rsencode encode/decode round-trip
# 
# Invoked by CTest:
#     cmake -DEZPWD_BIN=<path to rsencode> -P ezpwd-rsencode-roundtrip.cmake
# 
# Encodes a payload to a file, decodes it back, and verifies the original
# payload is recovered.  Pure CMake -- no shell/perl/sed dependency, so it
# runs identically on Linux, macOS and Windows.  (Single-error correction is
# exercised separately by the rsexercise stress test.)
# 
if(NOT DEFINED EZPWD_BIN)
    message(FATAL_ERROR "EZPWD_BIN not defined")
endif()

# In script mode (-P) CMAKE_CURRENT_BINARY_DIR is not set, so derive a
# writable temp dir from the (build-tree) directory holding the binary.
get_filename_component(_ezpwd_bindir "${EZPWD_BIN}" DIRECTORY)
set(dir "${_ezpwd_bindir}/ezpwd-rsencode-roundtrip")
file(REMOVE_RECURSE "${dir}")
file(MAKE_DIRECTORY "${dir}")

# 1. Write the payload.
set(payload "The quick brown fox jumps over the lazy dog. 0123456789.")
file(WRITE "${dir}/in.bin" "${payload}")

# 2. Encode it (adds Reed-Solomon parity).
execute_process(
    COMMAND "${EZPWD_BIN}" "${dir}/in.bin" "${dir}/enc.bin"
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "rsencode encode failed (rc=${rc})")
endif()

# 3. Decode the encoded data back to a payload.
execute_process(
    COMMAND "${EZPWD_BIN}" --decode "${dir}/enc.bin" "${dir}/out.bin"
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "rsencode decode failed (rc=${rc})")
endif()

# 4. Verify the original payload was recovered.
file(READ "${dir}/in.bin"  orig)
file(READ "${dir}/out.bin" recovered)
if(NOT orig STREQUAL recovered)
    message(FATAL_ERROR
        "round-trip mismatch:\n  original:  ${orig}\n  recovered: ${recovered}")
endif()

message(STATUS "rsencode round-trip OK (payload recovered exactly)")
