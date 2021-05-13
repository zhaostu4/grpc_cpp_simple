//
// Created by zhao on 2021/5/13.
//

#include "simple.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <functional>

using Simple::TestRequest;
using Simple::TestReply;
using Simple::TestNull;

using grpc::Status;
using grpc::ClientContext;
using grpc::CompletionQueue;

using std::string;
using std::unique_ptr;
using std::shared_ptr;
using std::cout;
using std::endl;
using std::to_string;

#define isTest1(RequestType, ResponseType) (std::is_same<RequestType, Simple::TestRequest>::value and std::is_same<ResponseType, Simple::TestNull>::value)
#define isTest2(RequestType, ResponseType) (std::is_same<RequestType, Simple::TestNull>::value and std::is_same<ResponseType, Simple::TestReply>::value)
#define isTest3(RequestType, ResponseType) (std::is_same<RequestType, Simple::TestRequest>::value and std::is_same<ResponseType, Simple::TestReply>::value)


enum FunctionType{
    Test1,
    Test2,
    Test3,
};

template <typename RequestType, typename ResponseType>
class Task{
public:
    RequestType request_;
    ResponseType response_;

    bool ok_;
    Status status_;
    ClientContext ctx_;

    Task():ok_(false){}

    void Process(Simple::Server::Stub* stub, CompletionQueue* cq){
        // Test2
        if(isTest2(RequestType, ResponseType)){
            // if(std::is_same<RequestType, TestNull>::value and std::is_same<ResponseType, TestReply>::value){
            unique_ptr<grpc::ClientAsyncResponseReader<TestReply>> rpc(stub->PrepareAsyncTest2(&ctx_, (*((TestNull*)(&request_))), cq));
            rpc->StartCall();
            rpc->Finish(&response_, &status_, (void*) 1);
        }

        // Test3
        if(isTest3(RequestType, ResponseType)){
            // if(std::is_same<RequestType, TestRequest>::value and std::is_same<ResponseType, TestReply>::value){
            unique_ptr<grpc::ClientAsyncResponseReader<TestReply>> rpc(stub->PrepareAsyncTest3(&ctx_, (*((TestRequest*)(&request_))), cq));
            rpc->StartCall();
            rpc->Finish(&response_, &status_, (void*) 1);
        }

        void* tag;
        GPR_ASSERT(cq->Next(&tag, &ok_));
        GPR_ASSERT(ok_);
        GPR_ASSERT(tag == (void*) 1);

        if (status_.ok()) {
            cout << "SvrName:" << response_.svrname() << endl;
        } else {
            cout << "ErrorCode:" << to_string(status_.error_code()) << "\tErrorMsg:"<< status_.error_message() << endl;
        }

    }
};

typedef Task<TestRequest, TestReply> Test3Task;
typedef Task<TestNull, TestReply> Test2Task;

class Client{
public:
    explicit Client(const shared_ptr<grpc::Channel> &channel):stub_(Simple::Server::NewStub(channel)){}

    void Run(){
        for(int i=0; i<20; i++){
            SendTest3_v1("zhao", i+10, i);  // The actual RPC call!
            // SendTest2_v1();
        }
    }


    void SendTest3(const string &name, const int ClientId, const int externValue){
        TestRequest request;
        TestReply response;
        grpc::ClientContext ctx;
        grpc::Status status;


        request.set_name(name);
        request.set_id(ClientId);
        request.set_value(externValue);

        unique_ptr<grpc::ClientAsyncResponseReader<TestReply>> rpc(stub_->PrepareAsyncTest3(&ctx, request, &cq_));

        rpc->StartCall();

        rpc->Finish(&response, &status, (void*) 1);

        void* getTag;
        bool ok;
        GPR_ASSERT(cq_.Next(&getTag, &ok));
        GPR_ASSERT(ok);
        GPR_ASSERT(getTag == (void*)1 );

        if (status.ok()) {
            cout << "SvrName:" << response.svrname() << endl;
        } else {
            cout << "ErrorCode:" << to_string(status.error_code()) << "\tErrorMsg:"<< status.error_message()<< endl;
        }

    }
    void SendTest3_v1(const string &name, const int ClientId, const int externValue){

        Test3Task task;

        task.request_.set_name(name);
        task.request_.set_id(ClientId);
        task.request_.set_value(externValue);

        task.Process(stub_.get(), &cq_);
    }
    void SendTest2_v1(){

        Test2Task task;

        task.Process(stub_.get(), &cq_);

    }
private:
    unique_ptr<Simple::Server::Stub> stub_;
    grpc::CompletionQueue cq_;

};

int main(){
    Client client(grpc::CreateChannel("localhost:33333", grpc::InsecureChannelCredentials()));
    client.Run();
    return 0;
}