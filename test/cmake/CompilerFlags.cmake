# 编译器标志配置
# 用于 GitHub Actions CI 环境

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 通用编译标志
set(COMMON_C_FLAGS "-Wall -Wextra -Wno-unused-parameter")

# Debug 模式标志
set(CMAKE_C_FLAGS_DEBUG "${COMMON_C_FLAGS} -g -O0 -DDEBUG")

# Release 模式标志  
set(CMAKE_C_FLAGS_RELEASE "${COMMON_C_FLAGS} -O2 -DNDEBUG")

# 覆盖率标志（用于代码覆盖率测试）
if(ENABLE_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# 平台特定设置
if(WIN32)
    # Windows 特定设置
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    if(MINGW)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    endif()
elseif(UNIX AND NOT APPLE)
    # Linux 特定设置
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
elseif(APPLE)
    # macOS 特定设置
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
endif()

# CI 环境特殊设置
if(DEFINED ENV{GITHUB_ACTIONS})
    message(STATUS "检测到 GitHub Actions 环境")
    set(CMAKE_VERBOSE_MAKEFILE ON)
    
    # 在 CI 中使用更宽松的警告设置，避免因为测试代码的警告导致构建失败
    # 保留重要的错误检测，但允许测试代码中的一些常见警告
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wno-sign-compare -Wno-overflow")
    
    # 只对严重错误使用 -Werror
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=implicit-function-declaration -Werror=incompatible-pointer-types")
endif()
