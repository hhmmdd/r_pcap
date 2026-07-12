# plugins/r_pcap 目录 CMakeLists.txt

if (WIN32)
    find_package(npcap REQUIRED)
endif()

# 收集所有源文件
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)
# 收集所有C文件, 确保C文件使用C编译器而不是C++编译器编译
file(GLOB_RECURSE C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
set_source_files_properties(${C_SOURCES} PROPERTIES LANGUAGE C)

list(FILTER SOURCES EXCLUDE REGEX ".*h235_t\\.cxx$")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

# 创建r_pcap库
add_library(r_pcap SHARED ${SOURCES})

# 设置r_pcap的文件夹
set_target_properties(r_pcap PROPERTIES FOLDER "plugins")

if(UNIX)
    # 因为plugins都是动态加载的库需要放在bin目录下才能找到
    set_target_properties(r_pcap PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
    )
endif()

# windows下PKG_MALLOC不支持64位， RockyLinux9下也会在free处崩溃
target_compile_definitions(r_pcap PRIVATE
    -DPASN_NOPRINTON
#    -DPKG_MALLOC
)

target_include_directories(r_pcap PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/r_node
)

# 链接必要的库
if(WIN32)
    target_include_directories(r_pcap PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/ptlib/include
        ${CMAKE_SOURCE_DIR}/libs/ptlib/Lib/x64/include
    )
    target_link_directories(r_pcap PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/ptlib/Lib2022/x64/$(CMAKE_BUILD_TYPE)
    )
    target_link_libraries(r_pcap PRIVATE
        npcap::npcap
        $<IF:$<CONFIG:DEBUG>, ptlib64sd, ptlib64s>
    )
else()
    # 解决error: multiple types in one declaration
    # /usr/local/include/ptlib/unix/ptlib/pmachdep.h
    # 544|typedef int SOCKET;
    # 需手动vi编辑该文件，注释掉该行
    target_include_directories(r_pcap PRIVATE
        /usr/include
        /usr/local/include
    )
    target_link_directories(r_pcap PRIVATE
        /usr/lib64
        /usr/local/lib
    )
    target_link_libraries(r_pcap PRIVATE
        pcap
        $<IF:$<CONFIG:DEBUG>, pt_d_s, pt_s>
    )
endif()