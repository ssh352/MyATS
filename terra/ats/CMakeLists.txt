#CMake file for library ats.a
#Author:jianwei cheng
#Create:20160616
#SET(PROJECT_SOURCE_DIR ../../../CppProjectsV2/)
#SET(PROJECT_BINARY_DIR ../../../CppProjectsV2/terra/ats/)
#源文件
FILE(GLOB_RECURSE SOURCE_FILES src/*.cpp)
    
#相关头文件的目录    
 INCLUDE_DIRECTORIES( 
  /usr/include/c++/6.1.0 
  /usr/local/include 
  /usr/include 
  ${PROJECT_SOURCE_DIR}/terra/thrift_include/thrift/src 
  ${PROJECT_SOURCE_DIR}/terra/thrift_include/zlib-1.2.8  
  ${PROJECT_SOURCE_DIR}/terra/zmq404/include 
  ${PROJECT_SOURCE_DIR}/terra/common/include
  ${PROJECT_SOURCE_DIR}/terra/instruments/include
  ${PROJECT_SOURCE_DIR}/terra/feedcommon/include
  ${PROJECT_SOURCE_DIR}/terra/marketaccess/include
  ${PROJECT_SOURCE_DIR}/feed/TwsSocketClient
  ${PROJECT_SOURCE_DIR}/feed/lts/api/include
  ${PROJECT_SOURCE_DIR}/feed/lts/include
  ${PROJECT_SOURCE_DIR}/feed/ltsl2/api/include 
  ${PROJECT_SOURCE_DIR}/feed/ltsl2/include   
  ${PROJECT_SOURCE_DIR}/feed/ib/include
  ${PROJECT_SOURCE_DIR}/feed/cffex/api/include
  ${PROJECT_SOURCE_DIR}/feed/cffex/include
  ${PROJECT_SOURCE_DIR}/feed/ltsudp/include
  ${PROJECT_SOURCE_DIR}/feed/xspeed/api/include
  ${PROJECT_SOURCE_DIR}/feed/xspeed/include
  ${PROJECT_SOURCE_DIR}/feed/xspeed2/api/include
  ${PROJECT_SOURCE_DIR}/feed/xspeed2/include
  ${PROJECT_SOURCE_DIR}/feed/femas/api/include
  ${PROJECT_SOURCE_DIR}/feed/femas/include 
  ${PROJECT_SOURCE_DIR}/feed/ltscffexudp/include
  ${PROJECT_SOURCE_DIR}/feed/fs/include
  ${PROJECT_SOURCE_DIR}/feed/fs/api/include
  ${PROJECT_SOURCE_DIR}/feed/tdf/include
  ${PROJECT_SOURCE_DIR}/feed/tdf/api/include
  ${PROJECT_SOURCE_DIR}/feed/sl/include
  ${PROJECT_SOURCE_DIR}/feed/sl/api/include
  ${PROJECT_SOURCE_DIR}/feed/sl2/include
  ${PROJECT_SOURCE_DIR}/feed/sl2/api/include
  ${PROJECT_SOURCE_DIR}/feed/es/include
  ${PROJECT_SOURCE_DIR}/feed/es/api/include
  ${PROJECT_SOURCE_DIR}/feed/xsl2/include
  ${PROJECT_SOURCE_DIR}/feed/xsl2/api/include
  ${PROJECT_SOURCE_DIR}/feed/zd/include
  ${PROJECT_SOURCE_DIR}/feed/zd/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing 
  ${PROJECT_SOURCE_DIR}/orderpassing/lts/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/lts/include
  ${PROJECT_SOURCE_DIR}/orderpassing/cffex/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/cffex/include
  ${PROJECT_SOURCE_DIR}/orderpassing/xspeed/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/xspeed/include
  ${PROJECT_SOURCE_DIR}/orderpassing/xspeed_of/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/xspeed_of/include
  ${PROJECT_SOURCE_DIR}/orderpassing/x1/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/x1/include
  ${PROJECT_SOURCE_DIR}/orderpassing/fs/include
  ${PROJECT_SOURCE_DIR}/orderpassing/fs/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/sl/include
  ${PROJECT_SOURCE_DIR}/orderpassing/sl/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/es/include
  ${PROJECT_SOURCE_DIR}/orderpassing/es/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/zd/include
  ${PROJECT_SOURCE_DIR}/orderpassing/zd/api/include
  ${PROJECT_SOURCE_DIR}/terra/tbb/include

  ${PROJECT_SOURCE_DIR}/terra/ats/include
)     

#所依赖库的目录路径
 LINK_DIRECTORIES(
 /usr/local/lib
 /usr/lib64
 ${PROJECT_BINARY_DIR}/lib
 ${PROJECT_SOURCE_DIR}/terra/marketaccess/lib
 ${PROJECT_SOURCE_DIR}/terra/common/lib
 ${PROJECT_SOURCE_DIR}/terra/feedcommon/lib
 )
 #依赖的库文件       
 
#生成静态链接库
ADD_LIBRARY(ats ${SOURCE_FILES})


#设置编译的库文件存放的目录                    

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
