//
// Created by zhao on 2021/5/12.
//

#include "simple.grpc.pb.h"
#include "Random/randomint.h"
#include <grpcpp/grpcpp.h>
#include <memory>

/*
 * sync-grpc client
 * 1. create channel: 需要 port, InsecureChannelCredentials
 * 2. create stub: channel
 * 3. request function: &clientContext, requst, &response
 * 4. check response
 * */

std::uniform_int_distribution<int> randomInt::u_{0, 1000};

class SyncClient{
public:
    explicit SyncClient(const std::shared_ptr<grpc::Channel>& channel):stub_(Simple::Server::NewStub(channel)){}
    void Run(int echo){
        for (int i = 0; i < echo; ++i) {
            Send();
        }

    }
    void Send(){
        grpc::ClientContext ctx;
        Simple::TestRequest request;
        Simple::TestReply response;

        request.set_name("zhao");
        request.set_id(124);
        request.set_value(randomInt::get());

        // request function
        grpc::Status st = stub_->Test3(&ctx, request, &response);

        // check
        if(st.ok()){
            std::cout<< "tid:" << response.tid()
                     << "\tsvrname:" << response.svrname()
                     << "\ttakeuptime: " << response.takeuptime() << std::endl;
        }else{
            std::cout << "Code: " << st.error_code()
                      << "\t Message:" << st.error_message();
        }

    }
private:
    std::shared_ptr<Simple::Server::Stub> stub_;
};

int main(){
    SyncClient client(grpc::CreateChannel("localhost:33333", grpc::InsecureChannelCredentials()));
    client.Run(10);
    return 0;
}
