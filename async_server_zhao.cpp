//
// Created by zhao on 2021/5/12.
//

#include "simple.grpc.pb.h"
#include "ThreadTool/threadtool.h"
#include "ThreadPool/threadpool.h"
#include <thread>
#include <grpcpp/grpcpp.h>
#include <strstream>
#include <functional>

using grpc::ServerContext;
using grpc::ServerAsyncResponseWriter;

using Simple::TestRequest;
using Simple::TestReply;
using Simple::TestNull;

#define isTest1(RequestType, ResponseType) (std::is_same<RequestType, Simple::TestRequest>::value and std::is_same<ResponseType, Simple::TestNull>::value)
#define isTest2(RequestType, ResponseType) (std::is_same<RequestType, Simple::TestNull>::value and std::is_same<ResponseType, Simple::TestReply>::value)
#define isTest3(RequestType, ResponseType) (std::is_same<RequestType, Simple::TestRequest>::value and std::is_same<ResponseType, Simple::TestReply>::value)


class ServiceImpl:public Simple::Server::AsyncService{
public:
    ::grpc::Status Test1(::grpc::ServerContext *context, const ::Simple::TestRequest *request, ::Simple::TestNull *response) override {

        std::string message;
        message += "Client Name:" + request->name();
        message += "\tClient ID:" + std::to_string(request->id());
        message += "\tClient Value:" + std::to_string(request->value());
        message += "\tServer TID:" + std::to_string(getTid());
        // grpc状态可以设置message,所以也可以用来返回一些信息
        return grpc::Status(grpc::StatusCode::OK, message);
    }
    ::grpc::Status Test2(::grpc::ServerContext *context, const ::Simple::TestNull *request, ::Simple::TestReply *response) override {
        response->set_tid(100);
        response->set_svrname(__func__);
        response->set_takeuptime(0.01);
        return grpc::Status::OK;
    }
    ::grpc::Status Test3(::grpc::ServerContext *context, const ::Simple::TestRequest *request, ::Simple::TestReply *response) override {
        // printf("Input response.svrname:%s\n", response->svrname().c_str());
        std::string message;
        message += "Client Name:" + request->name();
        message += "\tClient ID:" + std::to_string(request->id());
        message += "\tClient Value:" + std::to_string(request->value());
        message += "\tServer TID:" + std::to_string(getTid());

        // 休眠0.5秒，以便观察异步执行的效果
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));

        response->set_tid(getTid());
        response->set_svrname(message);// __FILE__
        response->set_takeuptime(1.234);

        // printf("Output response.svrname:%s\n", response->svrname().c_str());
        return grpc::Status::OK;
    }
public:
    std::mutex mu_;
};


enum FunctionType{
    Test1,
    Test2,
    Test3,
};
enum HandlerStatus{
    CREATE,
    PROCESS,
    FINISH,
};
struct HandlerContextBase{
    // FunctionType Type_;
    HandlerStatus Status_;
    grpc::ServerContext ctx_;

    explicit HandlerContextBase():Status_(HandlerStatus::CREATE){}
    virtual  ~HandlerContextBase(){}
    virtual void Process(ServiceImpl &service, grpc::ServerCompletionQueue* cq) {}
    virtual void RegitsterNew(ServiceImpl &service, grpc::ServerCompletionQueue* cq){}
};

template <typename RequstType, typename ReplyType>
struct HandlerContext:public HandlerContextBase{
    RequstType request_;
    ReplyType response_;
    grpc::ServerAsyncResponseWriter<ReplyType> responser_;

    explicit HandlerContext():responser_(&ctx_){}

    void RegitsterNew(ServiceImpl &service, grpc::ServerCompletionQueue* cq) override {
        auto handler = new (typeof(*this))();
        if(isTest1(RequstType, ReplyType)){
            // if(this->Type_ == FunctionType::Test1)
            service.RequestTest1(
                    &handler->ctx_,
                    (::Simple::TestRequest*)&handler->request_,
                    (::grpc::ServerAsyncResponseWriter< ::Simple::TestNull>*)&handler->responser_,
                    cq, cq, handler);
        }
        if(isTest2(RequstType, ReplyType)){
            // if(this->Type_ == FunctionType::Test2) {
            service.RequestTest2(
                    &handler->ctx_,
                    (::Simple::TestNull*)&handler->request_,
                    (::grpc::ServerAsyncResponseWriter< ::Simple::TestReply>*)&handler->responser_,
                    cq, cq, handler);
        }
        if(isTest3(RequstType, ReplyType)){
            // if(this->Type_ == FunctionType::Test3) {
            service.RequestTest3(
                    &handler->ctx_,
                    (::Simple::TestRequest *) &handler->request_,
                    (::grpc::ServerAsyncResponseWriter<::Simple::TestReply> *) &handler->responser_,
                    cq, cq, handler);
        }
    }

    void Process(ServiceImpl &service, grpc::ServerCompletionQueue* cq) override {
        // Status_ = HandlerStatus::PROCESS;
        grpc::Status status;
        if(isTest1(RequstType, ReplyType)) {
            status = service.Test1(&ctx_, (::Simple::TestRequest*)&request_, (::Simple::TestNull*)&response_); // call
        }
        if(isTest2(RequstType, ReplyType)) {
            status = service.Test2(&ctx_, (::Simple::TestNull*)&request_, (::Simple::TestReply*)&response_); // call
        }
        if(isTest3(RequstType, ReplyType)) {
            status = service.Test3(&ctx_, (::Simple::TestRequest*)&request_, (::Simple::TestReply*)&response_); // call
        }
        Status_ = HandlerStatus::FINISH;
        responser_.Finish(response_, status, this); // 异步发送结果
    }
};

typedef HandlerContext<Simple::TestRequest,Simple::TestNull>  HandlerTest1Context;
typedef HandlerContext<Simple::TestNull,Simple::TestReply>    HandlerTest2Context;
typedef HandlerContext<Simple::TestRequest,Simple::TestReply> HandlerTest3Context;


class Server{
public:
    void Init(){
        // Create service
        grpc::ServerBuilder builder;
        builder.AddListeningPort("localhost:33333", grpc::InsecureServerCredentials());

        builder.RegisterService(&service_);

        cqPtr_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        std::cout<<"Server Runing"<<std::endl;
    }
    void Register(){
        // Register Request
        auto handlerTest1 = new HandlerTest1Context();
        auto handlerTest2 = new HandlerTest2Context();
        auto handlerTest3 = new HandlerTest3Context();

        service_.RequestTest1(&handlerTest1->ctx_, &handlerTest1->request_, &handlerTest1->responser_, cqPtr_.get(), cqPtr_.get(), handlerTest1);
        service_.RequestTest2(&handlerTest2->ctx_, &handlerTest2->request_, &handlerTest2->responser_, cqPtr_.get(), cqPtr_.get(), handlerTest2);
        service_.RequestTest3(&handlerTest2->ctx_, &handlerTest3->request_, &handlerTest3->responser_, cqPtr_.get(), cqPtr_.get(), handlerTest3);
    }
    void Wait(int threadNum=4){
        ThreadPool pool(threadNum);
        while(1){
            HandlerContextBase* htc = NULL;
            bool ok = false;
            GPR_ASSERT(cqPtr_->Next((void**)&htc, &ok));
            GPR_ASSERT(ok);
            printf("htc:%u\tStatus:%d\n", htc, htc->Status_);
            // Finish
            if(htc->Status_ == HandlerStatus::FINISH){
                delete htc;
                continue;
            }
            // Process
            if(htc->Status_ == HandlerStatus::PROCESS) continue;
            // Create
            if(htc->Status_ == HandlerStatus::CREATE){
                htc->Status_ = HandlerStatus::PROCESS;
                htc->RegitsterNew(service_, cqPtr_.get());
                pool.enqueue([htc, this]{
                    htc->Process(service_, cqPtr_.get());
                });

                /*
                                if(htc->Type_ == FunctionType::Test1) {
                    // 新建一个HandlerContext
                    auto handler = new HandlerTest1Context(FunctionType::Test1);
                    service.RequestTest1(&handler->ctx_, &handler->request_, &handler->responser_, cqPtr.get(), cqPtr.get(), handler);
                    // process
                    pool.enqueue([htc, &service]{
                        auto h = (HandlerTest1Context*) htc;
                        auto status = service.Test1(&h->ctx_, &h->request_, &h->response_); // call

                        h->Status_ = HandlerStatus::FINISH;
                        h->responser_.Finish(h->response_, status, h); // 异步发送结果
                    });
                }
                if(htc->Type_ == FunctionType::Test2) {
                    // 新建一个HandlerContext
                    auto handler = new HandlerTest2Context(FunctionType::Test2);
                    service.RequestTest2(&handler->ctx_, &handler->request_, &handler->responser_, cqPtr.get(), cqPtr.get(), handler);
                    // process
                    pool.enqueue([htc, &service]{
                        auto h = (HandlerTest2Context*) htc;
                        auto status = service.Test2(&h->ctx_, &h->request_, &h->response_); // call

                        h->Status_ = HandlerStatus::FINISH;
                        h->responser_.Finish(h->response_, status, h); // 异步发送结果
                    });
                }
                if(htc->Type_ == FunctionType::Test3) {
                    // 新建一个HandlerContext
                    auto handler = new HandlerTest3Context(FunctionType::Test3);
                    service.RequestTest3(&handler->ctx_, &handler->request_, &handler->responser_, cqPtr.get(), cqPtr.get(), handler);
                    // process
                    pool.enqueue([htc, &service]{
                        auto h = (HandlerTest3Context*) htc;
                        auto status = service.Test3(&h->ctx_, &h->request_, &h->response_); // call

                        h->Status_ = HandlerStatus::FINISH;
                        h->responser_.Finish(h->response_, status, h); // 异步发送结果
                    });
                }
                 */
            }
        }
    }
private:
    ServiceImpl service_;
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<grpc::ServerCompletionQueue> cqPtr_;
};

int main(){
    Server server;
    server.Init();
    server.Register();
    server.Wait();
    return 0;
}