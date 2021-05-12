//
// Created by zhao on 2021/5/12.
//

#include "simple.grpc.pb.h"
#include <grpcpp/grpcpp.h>

#include <memory>
#include <iostream>

int main()
{
    // 创建一个连接服务器的通道
    std::shared_ptr<grpc::Channel> channel =
            grpc::CreateChannel("localhost:33333", grpc::InsecureChannelCredentials());
    // 创建一个stub
    std::unique_ptr<Simple::Server::Stub> stub = Simple::Server::NewStub(channel);

    // 上面部分可以复用，下面部分复用的话要自己考虑多线程安全问题
    {
        // 创建一个请求对象，用于打包要发送的请求数据
        Simple::TestRequest request;
        // 创建一个响应对象，用于解包响要接收的应数据
        Simple::TestReply   reply;

        // 创建一个客户端上下文。它可以用来向服务器传递附加的信息，以及可以调整某些RPC行为
        grpc::ClientContext context;
        // 发送请求，接收响应
        grpc::Status st = stub->Test3(&context,request,&reply);
        if(st.ok()){
            // 输出下返回数据
            std::cout<< "tid = " << reply.tid()
                     << "\nsvrname = " << reply.svrname()
                     << "\ntakeuptime = " << reply.takeuptime() << std::endl;
        }
        else {
            // 返回状态非OK
            std::cout<< "StatusCode = "<< st.error_code()
                     <<"\nMessage: "<< st.error_message() <<std::endl;
        }

    }

}


