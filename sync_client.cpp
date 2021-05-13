//
// Created by zhao on 2021/5/12.
//

#include "simple.grpc.pb.h"
#include <grpcpp/grpcpp.h>

#include <memory>
#include <iostream>
/*
 * sync-grpc client
 * 1. create channel
 * 2. create stub
 * 3. request function
 * 4. check response
 * */
int main()
{
    // 创建一个连接服务器的通道
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("localhost:33333", grpc::InsecureChannelCredentials());
    // 创建一个stub
    std::unique_ptr<Simple::Server::Stub> stub = Simple::Server::NewStub(channel);

    // 上面部分可以复用，下面部分复用的话要自己考虑多线程安全问题
    for (int i = 0; i < 40; ++i){
        // 创建一个请求对象，用于打包要发送的请求数据
        Simple::TestRequest request;
        request.set_name("zhao");
        request.set_id(1);
        request.set_value(52);
        // 创建一个响应对象，用于解包响要接收的应数据
        Simple::TestReply   reply;

        // 创建一个客户端上下文。它可以用来向服务器传递附加的信息，以及可以调整某些RPC行为
        grpc::ClientContext context;
        // 发送请求，接收响应
        grpc::Status st = stub->Test3(&context,request,&reply);
        if(st.ok()){
                        // 输出下返回数据
            //            std::cout<< "tid = " << reply.tid()
            //                     << "\nsvrname = " << reply.svrname()
            //                     << "\ntakeuptime = " << reply.takeuptime() << std::endl;
            std::cout << reply.svrname() << std::endl;
            // printf("%s\n", reply.svrname().c_str());
        }
        else {
            // 返回状态非OK
            std::cout<< "StatusCode = "<< st.error_code()
                     <<"\tMessage: "<< st.error_message() <<std::endl;
        }
        // sleep(2);

    }

}


