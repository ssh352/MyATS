#CMake file for app simpleatsserver
#Author:jianwei cheng
#Create:20160616

#源文件
SET(SOURCE_FILES
    src/main.cpp
    src/simpleatshandler.cpp
    src/SimpleAtsMsg_constants.cpp
    src/SimpleAtsMsg_types.cpp
    src/SimpleAtsOperation.cpp
    src/simpleatsserver.cpp
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
  ${PROJECT_SOURCE_DIR}/terra/marketaccess/include
  ${PROJECT_SOURCE_DIR}/feed/TwsSocketClient
  ${PROJECT_SOURCE_DIR}/feed/lts/api/include
  ${PROJECT_SOURCE_DIR}/feed/lts/include
  ${PROJECT_SOURCE_DIR}/feed/ltsl2/api/include 
  ${PROJECT_SOURCE_DIR}/feed/ltsl2/include   
  ${PROJECT_SOURCE_DIR}/feed/ib/include
  ${PROJECT_SOURCE_DIR}/feed/zd/api/include
  ${PROJECT_SOURCE_DIR}/feed/zd/include
  ${PROJECT_SOURCE_DIR}/feed/cffex/api/include
  ${PROJECT_SOURCE_DIR}/feed/cffex/include
  ${PROJECT_SOURCE_DIR}/feed/ltsudp/include
  ${PROJECT_SOURCE_DIR}/feed/xspeed/api/include
  ${PROJECT_SOURCE_DIR}/feed/xspeed/include
  ${PROJECT_SOURCE_DIR}/feed/xspeed2/api/include
  ${PROJECT_SOURCE_DIR}/feed/xspeed2/include
  ${PROJECT_SOURCE_DIR}/feed/femas/api/include
  ${PROJECT_SOURCE_DIR}/feed/femas/include
  ${PROJECT_SOURCE_DIR}/feed/tdf/api/include
  ${PROJECT_SOURCE_DIR}/feed/tdf/include  
  ${PROJECT_SOURCE_DIR}/feed/ltscffexudp/include
  ${PROJECT_SOURCE_DIR}/feed/es/include
  ${PROJECT_SOURCE_DIR}/orderpassing 
  ${PROJECT_SOURCE_DIR}/orderpassing/lts/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/lts/include
  ${PROJECT_SOURCE_DIR}/orderpassing/cffex/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/cffex/include
  ${PROJECT_SOURCE_DIR}/orderpassing/sl/api/lib/include
  ${PROJECT_SOURCE_DIR}/orderpassing/sl/include
  ${PROJECT_SOURCE_DIR}/orderpassing/xspeed/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/xspeed/include
  ${PROJECT_SOURCE_DIR}/orderpassing/xspeed_of/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/xspeed_of/include
  ${PROJECT_SOURCE_DIR}/orderpassing/x1/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/x1/include
  ${PROJECT_SOURCE_DIR}/orderpassing/ib/include
  ${PROJECT_SOURCE_DIR}/orderpassing/zd/api/include
  ${PROJECT_SOURCE_DIR}/orderpassing/zd/include
  ${PROJECT_SOURCE_DIR}/terra/ats/include
  ${PROJECT_SOURCE_DIR}/terra/tbb/include
  ${PROJECT_SOURCE_DIR}/terra/atsserver/include
  ${PROJECT_SOURCE_DIR}/simpleats/include
  ${PROJECT_SOURCE_DIR}/simpleatsserver/include
)     

#所依赖库的目录路径
 LINK_DIRECTORIES(
 /usr/local/lib
 /usr/lib64
 /usr/tbb
 /CTA_lib
 ${PROJECT_SOURCE_DIR}/lib
 ${PROJECT_SOURCE_DIR}/terra/thrift_include/zlib-1.2.8
 ${PROJECT_SOURCE_DIR}/terra/ats/lib
 ${PROJECT_SOURCE_DIR}/terra/atsserver/lib
 ${PROJECT_SOURCE_DIR}/terra/common/lib
 ${PROJECT_SOURCE_DIR}/terra/marketaccess/lib
 ${PROJECT_SOURCE_DIR}/terra/feedcommon/lib
 ${PROJECT_SOURCE_DIR}/terra/instruments/lib
 ${PROJECT_SOURCE_DIR}/terra/tbb/lib/linux/intel64/gcc4.7
 ${PROJECT_SOURCE_DIR}/terra/cryptopp/cryptopp
 ${PROJECT_SOURCE_DIR}/feed/TwsSocketClient/lib
 ${PROJECT_SOURCE_DIR}/orderpassing/cffex/lib
 ${PROJECT_SOURCE_DIR}/orderpassing/lts/lib
 ${PROJECT_SOURCE_DIR}/orderpassing/xspeed/lib
 ${PROJECT_SOURCE_DIR}/orderpassing/sl/lib
 ${PROJECT_SOURCE_DIR}/orderpassing/xspeed_of/lib
 ${PROJECT_SOURCE_DIR}/orderpassing/x1/lib
 ${PROJECT_SOURCE_DIR}/orderpassing/fs/lib
 ${PROJECT_SOURCE_DIR}/orderpassing/femas/lib
 ${PROJECT_SOURCE_DIR}/orderpassing/ib/lib
 ${PROJECT_SOURCE_DIR}/orderpassing/zd/lib
 
 ${PROJECT_SOURCE_DIR}/feed/cffex/lib
 ${PROJECT_SOURCE_DIR}/feed/lts/lib
 ${PROJECT_SOURCE_DIR}/feed/ltsudp/lib
 ${PROJECT_SOURCE_DIR}/feed/femas/lib
 ${PROJECT_SOURCE_DIR}/feed/lts/lib
 ${PROJECT_SOURCE_DIR}/feed/ltsl2/lib
 ${PROJECT_SOURCE_DIR}/feed/xspeed/lib
 ${PROJECT_SOURCE_DIR}/feed/xspeed2/lib
 ${PROJECT_SOURCE_DIR}/feed/ltscffexudp/lib
 ${PROJECT_SOURCE_DIR}/feed/fs/lib
 ${PROJECT_SOURCE_DIR}/feed/tdf/lib
 ${PROJECT_SOURCE_DIR}/feed/sl/lib
 ${PROJECT_SOURCE_DIR}/feed/sl2/lib 
 ${PROJECT_SOURCE_DIR}/feed/es/lib
 ${PROJECT_SOURCE_DIR}/feed/xsl2/lib
 ${PROJECT_SOURCE_DIR}/feed/femas/lib
 ${PROJECT_SOURCE_DIR}/feed/ib/lib
 ${PROJECT_SOURCE_DIR}/feed/zd/lib
 
 ${PROJECT_SOURCE_DIR}/orderpassing/cffex/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/orderpassing/lts/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/orderpassing/sl/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/orderpassing/xspeed/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/orderpassing/xspeed_of/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/orderpassing/x1/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/orderpassing/fs/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/orderpassing/es/api/lib/linux64 
 ${PROJECT_SOURCE_DIR}/orderpassing/femas/api/lib/linux64 
 ${PROJECT_SOURCE_DIR}/orderpassing/zd/api/lib/linux64
 
 ${PROJECT_SOURCE_DIR}/feed/cffex/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/lts/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/ltsudp/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/femas/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/lts/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/ltsl2/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/xspeed/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/xspeed2/api/lib/linux64_api_en
 ${PROJECT_SOURCE_DIR}/feed/ltscffexudp/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/fs/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/ib/api/lib
 ${PROJECT_SOURCE_DIR}/feed/tdf/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/sl/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/sl2/api/lib/linux
 ${PROJECT_SOURCE_DIR}/feed/es/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/xsl2/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/femas/api/lib/linux64
 ${PROJECT_SOURCE_DIR}/feed/zd/api/lib/linux64
 )

ADD_EXECUTABLE(simpleatsserver ${SOURCE_FILES})

#依赖的库文件
                    
#设置编译的库文件存放的目录                    
#TARGET_LINK_LIBRARIES(simpleatsserver libstdc++.a)
#TARGET_LINK_LIBRARIES(simpleatsserver libthrift.a)
#TARGET_LINK_LIBRARIES(simpleatsserver libthriftz.a)
#TARGET_LINK_LIBRARIES(simpleatsserver libthriftnb.a)
TARGET_LINK_LIBRARIES(simpleatsserver thrift)
TARGET_LINK_LIBRARIES(simpleatsserver thriftnb)
TARGET_LINK_LIBRARIES(simpleatsserver rt)
TARGET_LINK_LIBRARIES(simpleatsserver event)
TARGET_LINK_LIBRARIES(simpleatsserver pthread)
TARGET_LINK_LIBRARIES(simpleatsserver uuid)

TARGET_LINK_LIBRARIES(simpleatsserver nanomsg)

TARGET_LINK_LIBRARIES(simpleatsserver libz.a)
TARGET_LINK_LIBRARIES(simpleatsserver libzmq404.a)
TARGET_LINK_LIBRARIES(simpleatsserver libsimpleats.a)
TARGET_LINK_LIBRARIES(simpleatsserver libatsserver.a)
TARGET_LINK_LIBRARIES(simpleatsserver libats.a)

if(CON_LTS)
TARGET_LINK_LIBRARIES(simpleatsserver libop_lts.a)
endif()
if(CON_CTP)
TARGET_LINK_LIBRARIES(simpleatsserver libop_cffex.a)
endif()
if(CON_XS)
TARGET_LINK_LIBRARIES(simpleatsserver libop_xspeed.a)
endif()
if(CON_XS2)
TARGET_LINK_LIBRARIES(simpleatsserver libop_xspeed_of.a)
endif()
if(CON_X1)
TARGET_LINK_LIBRARIES(simpleatsserver libop_x1.a)
endif()
if(CON_FS)
TARGET_LINK_LIBRARIES(simpleatsserver libop_fs.a)
endif()
if(CON_SL)
TARGET_LINK_LIBRARIES(simpleatsserver libop_sl.a)
endif()
if(CON_ES)
TARGET_LINK_LIBRARIES(simpleatsserver libop_es.a)
endif()
if(CON_FEMAS)
TARGET_LINK_LIBRARIES(simpleatsserver libop_femas.a)
endif()
if(CON_IB)
TARGET_LINK_LIBRARIES(simpleatsserver libop_ib.a)
endif()
if(CON_ZD)
TARGET_LINK_LIBRARIES(simpleatsserver libop_zd.a)
endif()
TARGET_LINK_LIBRARIES(simpleatsserver libmarketaccess.a)
TARGET_LINK_LIBRARIES(simpleatsserver libinstruments.a)
TARGET_LINK_LIBRARIES(simpleatsserver libfeedcommon.a)

if(FEED_FS)
TARGET_LINK_LIBRARIES(simpleatsserver liblts.a)
endif()
if(FEED_LTS2)
TARGET_LINK_LIBRARIES(simpleatsserver libltsl2.a)
endif()
if(FEED_CTP)
TARGET_LINK_LIBRARIES(simpleatsserver libcffex.a)
endif()
if(FEED_XS)
TARGET_LINK_LIBRARIES(simpleatsserver libxspeed.a)
endif()
if(FEED_XS2)
TARGET_LINK_LIBRARIES(simpleatsserver libxspeed2.a)
endif()
if(FEED_LTSUDP)
TARGET_LINK_LIBRARIES(simpleatsserver libltsudp.a)
endif()
if(FEED_LTSCFFEXUDP)
TARGET_LINK_LIBRARIES(simpleatsserver libltscffexudp.a)
endif()
if(FEED_FS)
TARGET_LINK_LIBRARIES(simpleatsserver libfs.a)
endif()
if(FEED_TDF)
TARGET_LINK_LIBRARIES(simpleatsserver libtdf.a)
endif()
if(FEED_SL)
TARGET_LINK_LIBRARIES(simpleatsserver libsl.a)
endif()
if(FEED_SL2)
TARGET_LINK_LIBRARIES(simpleatsserver libsl2.a)
endif()
if(FEED_ES)
TARGET_LINK_LIBRARIES(simpleatsserver libes.a)
endif()
if(FEED_XSL2)
TARGET_LINK_LIBRARIES(simpleatsserver libxsl2.a)
endif()
if(FEED_FEMAS)
TARGET_LINK_LIBRARIES(simpleatsserver libfemas.a)
endif()
if(FEED_IB)
TARGET_LINK_LIBRARIES(simpleatsserver libib.a)
endif()
if(FEED_ZD)
TARGET_LINK_LIBRARIES(simpleatsserver libzd.a)
endif()
#if(FEED_SL2)
TARGET_LINK_LIBRARIES(simpleatsserver libsl2.a)
#endif()
TARGET_LINK_LIBRARIES(simpleatsserver libcommon.a)
TARGET_LINK_LIBRARIES(simpleatsserver libTwsSocketClient.a)

TARGET_LINK_LIBRARIES(simpleatsserver libboost_log_setup.a)
TARGET_LINK_LIBRARIES(simpleatsserver libboost_log.a)
TARGET_LINK_LIBRARIES(simpleatsserver libboost_date_time.a)
TARGET_LINK_LIBRARIES(simpleatsserver libboost_filesystem.a)
TARGET_LINK_LIBRARIES(simpleatsserver libboost_system.a)
TARGET_LINK_LIBRARIES(simpleatsserver libboost_thread.a)

TARGET_LINK_LIBRARIES(simpleatsserver libboost_atomic.a)
TARGET_LINK_LIBRARIES(simpleatsserver libboost_chrono.a)
TARGET_LINK_LIBRARIES(simpleatsserver libboost_timer.a)
TARGET_LINK_LIBRARIES(simpleatsserver libboost_exception.a)
TARGET_LINK_LIBRARIES(simpleatsserver libboost_serialization.a)
TARGET_LINK_LIBRARIES(simpleatsserver libboost_container.a)
TARGET_LINK_LIBRARIES(simpleatsserver libboost_coroutine.a)
TARGET_LINK_LIBRARIES(simpleatsserver libboost_regex.a)
TARGET_LINK_LIBRARIES(simpleatsserver libcryptopp.a)

if(FEED_SL2)
TARGET_LINK_LIBRARIES(simpleatsserver slefh)
TARGET_LINK_LIBRARIES(simpleatsserver libciul1.a)
TARGET_LINK_LIBRARIES(simpleatsserver libonload_ext.a)
endif()

TARGET_LINK_LIBRARIES(simpleatsserver tbb)
TARGET_LINK_LIBRARIES(simpleatsserver dl)

if(CON_X1)
TARGET_LINK_LIBRARIES(simpleatsserver x1_dfitc_api)
endif()
if(CON_XS2)
TARGET_LINK_LIBRARIES(simpleatsserver DFITCTraderApi)
endif()
if(FEED_XS2)
TARGET_LINK_LIBRARIES(simpleatsserver DFITCMdApi)
endif()
if(CON_XS)
TARGET_LINK_LIBRARIES(simpleatsserver DFITCSECTraderApi)
endif()
if(FEED_XS)
TARGET_LINK_LIBRARIES(simpleatsserver DFITCSECMdApi)
endif()
if(CON_LTS)
TARGET_LINK_LIBRARIES(simpleatsserver securityqueryapi)
TARGET_LINK_LIBRARIES(simpleatsserver securitytraderapi)
endif()
if(FEED_LTS)
TARGET_LINK_LIBRARIES(simpleatsserver securitymduserapi)
endif()
if(CON_CTP)
TARGET_LINK_LIBRARIES(simpleatsserver thosttraderapi)
endif()
if(FEED_CTP)
TARGET_LINK_LIBRARIES(simpleatsserver thostmduserapi)
endif()
if(CON_FS)
TARGET_LINK_LIBRARIES(simpleatsserver sgittradeapi)
endif()
if(FEED_FS)
TARGET_LINK_LIBRARIES(simpleatsserver sgitquotapi)
endif()
if(FEED_LTS2)
TARGET_LINK_LIBRARIES(simpleatsserver L2mduserapi)
endif()
if(FEED_SL)
TARGET_LINK_LIBRARIES(simpleatsserver EESQuoteApi)
endif()
if(CON_SL)
TARGET_LINK_LIBRARIES(simpleatsserver EESTraderApi)
endif()
if(FEED_ES)
TARGET_LINK_LIBRARIES(simpleatsserver TapQuoteAPI)
endif()
if(CON_ES)
TARGET_LINK_LIBRARIES(simpleatsserver TapTradeAPI)
endif()
if(FEED_TDF)
TARGET_LINK_LIBRARIES(simpleatsserver TDFAPI_v2.5)
endif()
TARGET_LINK_LIBRARIES(simpleatsserver WHNetWork)
if(FEED_XSL2)
TARGET_LINK_LIBRARIES(simpleatsserver level2Api)
endif()
if(FEED_ES)
TARGET_LINK_LIBRARIES(simpleatsserver USTPmduserapiAF)
endif()
if(CON_ES)
TARGET_LINK_LIBRARIES(simpleatsserver USTPtraderapiAF)
endif()
if(FEED_ZD)
TARGET_LINK_LIBRARIES(simpleatsserver ShZdFutureMarketApi)
endif()
if(CON_ZD)
TARGET_LINK_LIBRARIES(simpleatsserver ShZdFutureTradeApi)
endif()
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
