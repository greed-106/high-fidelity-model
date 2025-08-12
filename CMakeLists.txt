cmake_minimum_required(VERSION 3.12)
project("HFM")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ENCODER "EncoderApp")
set(DECODER "DecoderApp")
add_definitions("-DLINUX")
add_definitions("-DX86_64")

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  add_definitions(-DDEBUG)
  set(CMAKE_C_FLAGS "-g -O0 -Wall -std=c99")
  set(CMAKE_CXX_FLAGS "-g -O0 -Wall -std=c++17")
else()
  set(CMAKE_C_FLAGS "-O3 -Wall -std=c99")
  set(CMAKE_CXX_FLAGS "-O3 -Wall -std=c++17")
endif()

# common includes
set(INC_ARGS include/ArgParse)
set(INC_COMMON include/Common)
set(INC_UTILS include/Utils)
set(INC_VIDEO include/Video)
set(INC_JM_COMMON third_party/JM/common/inc)

# encoder includes
set(INC_ENCODER include/Encoder)
set(INC_JM_ENCODER third_party/JM/lencod/inc)
set(INC_LL_ENCODER include/Encoder/LLEncoder)
set(INC_HF_ENCODER include/Encoder/HFEncoder)
set(ENCODER_INCLUDES ${INC_ENCODER} ${INC_UTILS} ${INC_LL_ENCODER} ${INC_HF_ENCODER} ${INC_ARGS} ${INC_UTILS} ${INC_VIDEO} ${INC_COMMON} ${INC_JM_ENCODER} ${INC_JM_COMMON})

# decoder includes
set(INC_DECODER include/Decoder)
set(INC_JM_DECODER third_party/JM/ldecod/inc)
set(INC_LL_DECODER include/Decoder/LLDecoder)
set(INC_HF_DECODER include/Decoder/HFDecoder)
set(DECODER_INCLUDES ${INC_DECODER} ${INC_ARGS} ${INC_UTILS} ${INC_VIDEO} ${INC_COMMON} ${INC_JM_DECODER} ${INC_LL_DECODER} ${INC_HF_DECODER} ${INC_JM_COMMON})

# common src
aux_source_directory(src/Utils SRC_UTILS)
aux_source_directory(src/Common SRC_COMMON)
aux_source_directory(src/Video SRC_VIDEO)
aux_source_directory(third_party/JM/common/src JM_COMMON)

# encoder src
aux_source_directory(src/Encoder SRC_ENCODER)
aux_source_directory(src/Encoder/LLEncoder SRC_LL_ENCODER)
aux_source_directory(src/Encoder/HFEncoder SRC_HF_ENCODER)
aux_source_directory(third_party/JM/lencod/src JM_ENCODER)
set(ENCODER_SOURCES ${SRC_ENCODER} ${SRC_LL_ENCODER} ${SRC_HF_ENCODER} ${SRC_VIDEO} ${SRC_UTILS} ${SRC_COMMON} ${JM_ENCODER} ${JM_COMMON})

# decoder src
aux_source_directory(src/Decoder SRC_DECODER)
aux_source_directory(src/Decoder/LLDecoder SRC_LL_DECODER)
aux_source_directory(src/Decoder/HFDecoder SRC_HF_DECODER)
aux_source_directory(third_party/JM/ldecod/src JM_DECODER)
set(DECODER_SOURCES ${SRC_DECODER} ${SRC_UTILS} ${SRC_COMMON} ${SRC_VIDEO} ${JM_DECODER} ${SRC_LL_DECODER} ${SRC_HF_DECODER} ${JM_COMMON})

add_executable(${ENCODER} ${ENCODER_SOURCES})
target_include_directories(${ENCODER} PUBLIC ${ENCODER_INCLUDES})
target_link_libraries(${ENCODER} -lm)

add_executable(${DECODER} ${DECODER_SOURCES})
target_include_directories(${DECODER} PUBLIC ${DECODER_INCLUDES})
target_link_libraries(${DECODER} -lm)

install(TARGETS ${ENCODER} DESTINATION bin)
install(TARGETS ${DECODER} DESTINATION bin)
