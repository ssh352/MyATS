#CMake file for library zmq404.a
#Author:jianwei cheng
#Create:20160616

#源文件
FILE(GLOB_RECURSE SOURCE_FILES ${PROJECT_SOURCE_DIR}/terra/zmq404/src/*.cpp)    
    
#相关头文件的目录    
 INCLUDE_DIRECTORIES( 
  /usr/include/c++/6.1.0 
  /usr/local/include 
  ${PROJECT_SOURCE_DIR}/terra/zmq404/include
)     

#所依赖库的目录路径
 LINK_DIRECTORIES(
 /usr/local/lib
 /usr/lib64
 )
 
#生成静态链接库
ADD_LIBRARY(zmq404 ${SOURCE_FILES})

#依赖的库文件
                    
#设置编译的库文件存放的目录                    

TARGET_LINK_LIBRARIES(zmq404 libstdc++.a)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
