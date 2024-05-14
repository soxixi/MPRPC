#aux_source_directory(. SRC_LIST)
set(SRC_LIST 
    mprpcapplication.cc 
    mprpcconfig.cc 
    rpcheader.pb.cc 
    rpcprovider.cc 
    mprpcchannel.cc
    mprpccontroller.cc
    logger.cc
    zookeeperutil.cc)
add_library(mprpc ${SRC_LIST}) 
#编译成静态库
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)
#muduo_net是网络的核心模块，其他的库也会依赖muduo_net