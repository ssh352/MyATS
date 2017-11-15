#CMake file for library ltsl2.a
#Author:jianwei cheng
#Create:20160616

#源文件
SET(SOURCE_FILES
    src/ltsl2_connection.cpp
    src/ltsl2_decoder.cpp
    src/ltsl2_source.cpp   
    )
    
#相关头文件的目录    
 INCLUDE_DIRECTORIES( 
  /usr/include/c++/6.1.0 
  /usr/local/include 
  /usr/include 
  ${PROJECT_SOURCE_DIR}/terra/thrift_include/thrift/src 
  ${PROJECT_SOURCE_DIR}/terra/thrift_include/zlib-1.2.8
  ${PROJECT_SOURCE_DIR}/terra/nanomsg-1.0.0/src
  ${PROJECT_SOURCE_DIR}/terra/zmq404/include 
  ${PROJECT_SOURCE_DIR}/terra/common/include
  ${PROJECT_SOURCE_DIR}/terra/instruments/include
  ${PROJECT_SOURCE_DIR}/terra/feedcommon/include
  ${PROJECT_SOURCE_DIR}/feed/ltsl2/api/include
  ${PROJECT_SOURCE_DIR}/feed/ltsl2/include
  ${PROJECT_SOURCE_DIR}/terra/tbb/include/
)     

#所依赖库的目录路径
 LINK_DIRECTORIES(
 /usr/local/lib
 /usr/lib64
 )
 
#生成静态链接库
ADD_LIBRARY(ltsl2 ${SOURCE_FILES})

#依赖的库文件
                    
#设置编译的库文件存放的目录                    

TARGET_LINK_LIBRARIES(ltsl2 libstdc++.a)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
