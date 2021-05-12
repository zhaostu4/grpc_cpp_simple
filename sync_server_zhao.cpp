//
// Created by zhao on 2021/5/12.
//
#include "simple.grpc.pb.h"
#include "Timer/timer.h"
#include "ThreadTool/threadtool.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <strstream>

class ServiceImpl: public Simple::Server::Service{
    ::grpc::Status Test1(
            ::grpc::ServerContext *context,
            const ::Simple::TestRequest *request,
            ::Simple::TestNull *response) override {
        std::ostrstream os;
        time_t current = timer::getNow();
        os  << "ClientUser:"<< request->name() << "\tClientId:" << request->id()
            << "\tExterned:" << request->value() << "\tTime:" << std::ctime(&current);
        std::string message = os.str();
        return grpc::Status(grpc::StatusCode::OK, message);
    };
    ::grpc::Status Test2(
            ::grpc::ServerContext *context,
            const ::Simple::TestNull *request,
            ::Simple::TestReply *response) override {
        std::ostrstream os;
        time_t current = timer::getNow();
        os  << "Time:" << std::ctime(&current);
        std::string message = os.str();
        return grpc::Status(grpc::StatusCode::OK, message);
    };
    ::grpc::Status Test3(
            ::grpc::ServerContext *context,
            const ::Simple::TestRequest *request,
            ::Simple::TestReply *response) override {
        time_t current = timer::getNow();
        response->set_svrname(request->name());
        response->set_takeuptime( difftime(timer::getNow(), current) );
        response->set_tid(getTid());
        return grpc::Status::OK;
    };
};

/*
 * 1. server builder
 * 2. lisenning port
 * 3. register servers
 * 4. create server
 * 5. wait
 *
 * */
class Server{
public:
    void Run(){
        grpc::ServerBuilder builder;
        builder.AddListeningPort("localhost:33333", grpc::InsecureServerCredentials());
        ServiceImpl service;
        builder.RegisterService(&service);
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        server_ = std::move(server);
        std::cout << "Server Runing \tTid:" << getTid() << std::endl;
        server_->Wait();
    }

private:
    std::unique_ptr<grpc::Server> server_;
};

int main(){
    Server server;
    server.Run();
    return 0;
}