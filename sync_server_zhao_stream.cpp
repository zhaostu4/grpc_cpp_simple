//
// Created by zhao on 2021/5/12.
//
#include "experiment.grpc.pb.h"
#include "Timer/timer.h"
#include "ThreadTool/threadtool.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <strstream>

class ServiceImpl: public Experiment::Server::Service{
    ::grpc::Status Test(
            ::grpc::ServerContext *context,
            const ::Experiment::Request *request,
            ::Experiment::Reply *response) override {
        time_t current = timer::getNow();
        response->set_svrname(request->name());
        response->set_takeuptime( difftime(timer::getNow(), current) );
        response->set_tid(getTid());
        return grpc::Status::OK;
    };
    ::grpc::Status TestInStream(
            ::grpc::ServerContext *context,
            ::grpc::ServerReader< ::Experiment::Request> *reader,
            ::Experiment::Reply *response) override {
            ::Experiment::Request request;
            std::string svrName, extend;
            response->set_tid(getTid());
            while(reader->Read(&request)){
                svrName += "," + request.name();
                extend += "," + std::to_string(request.value());
            }
            response->set_svrname(svrName);
            response->set_extend(extend);
            return grpc::Status::OK;
    }
    ::grpc::Status TestOutStream(
            ::grpc::ServerContext *context,
            const ::Experiment::Request *request,
            ::grpc::ServerWriter< ::Experiment::Reply> *writer) override {
        double echo = request->value();
        time_t start = timer::getNow();
        Experiment::Reply response;
        response.set_tid(getTid());
        response.set_svrname(request->name());
        for (int i = 0; i < echo; ++i) {
            response.set_takeuptime(difftime(timer::getNow(), start));
            response.set_extend(std::to_string(i)+"/"+std::to_string(echo));
            writer->Write(response);
        }
        return grpc::Status::OK;
    }
    ::grpc::Status TestBidirectionalStream(
            ::grpc::ServerContext *context,
            ::grpc::ServerReaderWriter< ::Experiment::Reply,
            ::Experiment::Request> *stream) override {
        time_t start = timer::getNow();
        Experiment::Request request;
        Experiment::Reply response;
        while(stream->Read(&request)){
            response.set_tid(getTid());
            response.set_svrname(request.name());
            response.set_takeuptime(difftime(timer::getNow(), start));
            response.set_extend(std::to_string(request.value()));
            stream->Write(response);
        }
        return grpc::Status::OK;
    }
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