# check-size.cmake — Post-build Flash/RAM size gate
#
# Invoked via:
#   cmake -DELF=<path/to/firmware.elf>
#         -DSIZE_TOOL=<arm-none-eabi-size>
#         -DMAX_FLASH=<bytes>
#         -DMAX_RAM=<bytes>
#         -P cmake/check-size.cmake
#
# Fails the build if Flash (text+data) or RAM (data+bss) exceeds the limit.
# PY32F071: Flash limit 118 KB (120832), RAM limit 16 KB (16384).

cmake_minimum_required(VERSION 3.22)

foreach(VAR ELF SIZE_TOOL MAX_FLASH MAX_RAM)
    if(NOT DEFINED ${VAR})
        message(FATAL_ERROR "check-size.cmake: missing required variable -D${VAR}=...")
    endif()
endforeach()

execute_process(
    COMMAND ${SIZE_TOOL} --format=berkeley ${ELF}
    OUTPUT_VARIABLE SIZE_OUTPUT
    ERROR_VARIABLE  SIZE_ERROR
    RESULT_VARIABLE SIZE_RESULT
)

if(NOT SIZE_RESULT EQUAL 0)
    message(FATAL_ERROR "size tool failed: ${SIZE_ERROR}")
endif()

# Parse sysv/berkeley output — second line: "   text    data     bss     dec     hex filename"
string(REGEX MATCH "[ \t]+([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+)[ \t]+([0-9]+)" _ "${SIZE_OUTPUT}")
set(TEXT_SIZE ${CMAKE_MATCH_1})
set(DATA_SIZE ${CMAKE_MATCH_2})
set(BSS_SIZE  ${CMAKE_MATCH_3})

if(NOT TEXT_SIZE)
    message(FATAL_ERROR "check-size.cmake: could not parse size output:\n${SIZE_OUTPUT}")
endif()

math(EXPR FLASH_USED "${TEXT_SIZE} + ${DATA_SIZE}")
math(EXPR RAM_USED   "${DATA_SIZE} + ${BSS_SIZE}")

math(EXPR FLASH_PCT "${FLASH_USED} * 100 / ${MAX_FLASH}")
math(EXPR RAM_PCT   "${RAM_USED}   * 100 / ${MAX_RAM}")

message(STATUS "Flash: ${FLASH_USED} / ${MAX_FLASH} bytes  (${FLASH_PCT}%)  [text=${TEXT_SIZE}  data=${DATA_SIZE}]")
message(STATUS "RAM:   ${RAM_USED}   / ${MAX_RAM} bytes  (${RAM_PCT}%)  [data=${DATA_SIZE}  bss=${BSS_SIZE}]")

if(FLASH_USED GREATER MAX_FLASH)
    math(EXPR OVER "${FLASH_USED} - ${MAX_FLASH}")
    message(FATAL_ERROR
        "*** FLASH OVERFLOW: ${FLASH_USED} bytes used, limit is ${MAX_FLASH} bytes (over by ${OVER} bytes) ***")
endif()

if(RAM_USED GREATER MAX_RAM)
    math(EXPR OVER "${RAM_USED} - ${MAX_RAM}")
    message(FATAL_ERROR
        "*** RAM OVERFLOW: ${RAM_USED} bytes used, limit is ${MAX_RAM} bytes (over by ${OVER} bytes) ***")
endif()

message(STATUS "Size check passed.")
