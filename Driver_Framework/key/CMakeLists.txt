# 得到当前文件名
get_filename_component(DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
# 添加源文件
file(GLOB_RECURSE ${DIR_NAME}_Source 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)


# 生成库文件
add_library(${DIR_NAME} STATIC ${${DIR_NAME}_Source})
# 添加当前头文件路径
target_include_directories(${DIR_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
