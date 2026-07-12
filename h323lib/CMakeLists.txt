# libs/h323lib 目录 CMakeLists.txt

# 收集所有源文件
file(GLOB SOURCES "*.cpp" "*.cxx")
file(GLOB HEADERS "*.h")

# 收集asn目录下的源文件
file(GLOB ASN_SOURCES "asn/*.cpp" "asn/*.cxx" "asn/*.c")
list(FILTER ASN_SOURCES EXCLUDE REGEX ".*h235_t\\.cxx$")
file(GLOB ASN_HEADERS "asn/*.h")

# 创建h323lib库
add_library(h323lib STATIC ${SOURCES} ${HEADERS} ${ASN_SOURCES} ${ASN_HEADERS})

# 设置h323lib的文件夹
set_target_properties(h323lib PROPERTIES FOLDER "libs")

# 包含必要的头文件
target_include_directories(h323lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/asn
    ${CMAKE_CURRENT_SOURCE_DIR}/../ptlib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../ptlib/Lib/x64/include
)

if(WIN32)
    target_link_directories(h323lib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../ptlib/Lib2022/x64/$(CMAKE_BUILD_TYPE)
    )
    # Windows平台
    target_link_libraries(h323lib PRIVATE
        $<IF:$<CONFIG:DEBUG>, ptlib64sd, ptlib64s>
    )
else()
    # Linux平台
    target_link_libraries(h323lib PRIVATE
        ptlib
    )
endif()