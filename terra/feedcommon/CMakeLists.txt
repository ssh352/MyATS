#CMake file for library feedcommon.a
#Author:jianwei cheng
#Create:20160616

#源文件

FILE(GLOB_RECURSE SOURCE_FILES src/*.cpp)
    
#相关头文件的目录    
 INCLUDE_DIRECTORIES( 
  /usr/include/c++/6.1.0 
  /usr/local/include 
  ${PROJECT_SOURCE_DIR}/terra/common/include
  ${PROJECT_SOURCE_DIR}/terra/feedcommon/include
  ${PROJECT_SOURCE_DIR}/terra/tbb/include
  ${PROJECT_SOURCE_DIR}/terra/nanomsg-1.0.0/src
)     

#所依赖库的目录路径
 LINK_DIRECTORIES(
 /usr/local/lib
 /usr/lib64
 )
 
#生成静态链接库
ADD_LIBRARY(feedcommon ${SOURCE_FILES})

#依赖的库文件
                    
#设置编译的库文件存放的目录                    

TARGET_LINK_LIBRARIES(feedcommon libstdc++.a)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
