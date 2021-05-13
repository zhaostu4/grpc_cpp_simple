//
// Created by zhao on 2021/5/12.
//

#include "simple.grpc.pb.h"
#include "ThreadTool/threadtool.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <iostream>
#include <strstream>

// 继承自生成的Service类，实现三个虚函数
class ServiceImpl: public Simple::Server::Service {
public:

    // Test1 实现都是差不都的，这里只是为了测试，就随便返回点数据了
    grpc::Status Test1(grpc::ServerContext*       context,
                       const Simple::TestRequest* request,
                       Simple::TestNull*          response)
    override
    {
        auto tid = getTid();
        std::ostrstream os;
        os.clear();
        os << "Client Name = " << request->name() << '\t';
        os << "Clinet ID   = " << request->id()   << '\t';
        os << "Clinet Value= " << request->value()<< '\t';
        os << "Server TID  = " << tid<<'\t';
        std::string message = os.str();
        // grpc状态可以设置message,所以也可以用来返回一些信息
        return grpc::Status::OK;
    }
    // Test2
    grpc::Status Test2(grpc::ServerContext*       context,
                       const Simple::TestNull*    request,
                       Simple::TestReply*         response)
    override
    {
        response->set_tid(100);
        response->set_svrname("Simple Server");
        response->set_takeuptime(0.01);
        return grpc::Status::OK;
    }
    // Test3
    grpc::Status Test3(grpc::ServerContext*       context,
                       const Simple::TestRequest* request,
                       Simple::TestReply*         response)
    override
    {

        // printf("Input response.svrname:%s\n", response->svrname().c_str());
        std::string message;
        message += "Client Name:" + request->name();
        message += "\tClient ID:" + std::to_string(request->id());
        message += "\tClient Value:" + std::to_string(request->value());
        message += "\tServer TID:" + std::to_string(getTid());
        // grpc状态可以设置message,所以也可以用来返回一些信息

        response->set_tid(__LINE__);
        response->set_svrname(message);
        response->set_takeuptime(1.234);
        // printf("Output response.svrname:%s\n", response->svrname().c_str());
        // grpc状态可以设置message
        return grpc::Status::OK;
    }
};
/*
 * 1. Server Builder
 * 2. Listening Port
 * 3. Register Server
 * 4. Create Server
 * 5. Wait()
 * */
int main()
{
    // 服务构建器，用于构建同步或者异步服务
    grpc::ServerBuilder builder;
    // 添加监听的地址和端口，后一个参数用于设置认证方式，这里选择不认证
    builder.AddListeningPort("0.0.0.0:33333",grpc::InsecureServerCredentials());
    // 创建服务对象
    ServiceImpl service;
    // 注册服务
    builder.RegisterService(&service);
    // 构建服务器
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout<<"Server Runing"<<std::endl;
    // 进入服务处理循环（必须在某处调用server->Shutdown()才会返回）
    server->Wait();
    return 0;
}