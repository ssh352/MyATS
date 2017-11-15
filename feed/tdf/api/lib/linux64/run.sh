#!/bin/sh -x
g++ -pthread -o Demo.out Demo.cpp -L../lib -lrt -lTDFAPI_v2.5 -lWHNetWork -I../include
export LD_LIBRARY_PATH=../lib/
./Demo.out 10.100.7.18 10000 dev_test dev_test
