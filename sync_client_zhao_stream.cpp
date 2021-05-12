//
// Created by zhao on 2021/5/12.
//

#include "experiment.grpc.pb.h"
#include "Random/randomint.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <thread>

/*
 * sync-grpc client
 * 1. create channel: 需要 port, InsecureChannelCredentials
 * 2. create stub: channel
 * 3. request function: &clientContext, requst, &response
 * 4. check response
 * */

std::uniform_int_distribution<int> randomInt::u_{1, 5};

class SyncClient{
public:
    explicit SyncClient(const std::shared_ptr<grpc::Channel>& channel):stub_(Experiment::Server::NewStub(channel)){}
    void Run(int echo){
        for (int i = 0; i < echo; ++i) {
            SendTest();
            SendTestInStream();
            SendTestOutStream();
            SendTestBidrectionalStream();
        }

    }
    void SendTest(){
        grpc::ClientContext ctx;
        Experiment::Request request;
        Experiment::Reply response;

        request.set_name("zhao");
        request.set_id(124);
        request.set_value(randomInt::get());

        // request function
        grpc::Status st = stub_->Test(&ctx, request, &response);

        // check
        if(st.ok()){
            std::cout<< "Function: SendTest"
                     << "\ttid:" << response.tid()
                     << "\tsvrname:" << response.svrname()
                     << "\textend:" << response.extend()
                     << "\ttakeuptime: " << response.takeuptime() << std::endl;
        }else{
            std::cout << "Code: " << st.error_code()
                      << "\t Message:" << st.error_message();
        }

    }
    void SendTestInStream(){
        grpc::ClientContext ctx;
        Experiment::Request request;
        Experiment::Reply response;

        request.set_name("zhao");
        request.set_id(124);
        request.set_value(randomInt::get());

        // request function
        std::unique_ptr<grpc::ClientWriter<Experiment::Request>> writer(stub_->TestInStream(&ctx, &response));
        writer->Write(request);
        writer->Write(request);
        writer->Write(request);
        writer->WritesDone();

        grpc::Status st = writer->Finish();
        // check
        if(st.ok()){
            std::cout<< "Function: SendTestInStream"
                     << "\ttid:" << response.tid()
                     << "\tsvrname:" << response.svrname()
                     << "\textend:" << response.extend()
                     << "\ttakeuptime: " << response.takeuptime() << std::endl;
        }else{
            std::cout << "Code: " << st.error_code()
                      << "\t Message:" << st.error_message();
        }

    }
    void SendTestOutStream(){
        grpc::ClientContext ctx;
        Experiment::Request request;
        Experiment::Reply response;

        request.set_name("zhao");
        request.set_id(124);
        request.set_value(randomInt::get());

        // request function
        std::unique_ptr<grpc::ClientReader<Experiment::Reply>> reader(stub_->TestOutStream(&ctx, request));
        while(reader->Read(&response)){
            std::cout<< "Function: SendTestOutStream"
                     << "\ttid:" << response.tid()
                     << "\tsvrname:" << response.svrname()
                     << "\textend:" << response.extend()
                     << "\ttakeuptime: " << response.takeuptime() << std::endl;
        }
        grpc::Status st = reader->Finish();
        // check
        if(st.ok()){
            std::cout << "SendTestOutStream rpc succeeded." << std::endl;
        }else{
            std::cout << "SendTestOutStream rpc failed. Code: " << st.error_code()
                      << "\t Message:" << st.error_message();
        }
    }
    void SendTestBidrectionalStream(){
        grpc::ClientContext ctx;
        Experiment::Request request;
        Experiment::Reply response;

        request.set_name("zhao");
        request.set_id(124);
        request.set_value(randomInt::get());

        // request function
        std::shared_ptr<grpc::ClientReaderWriter<Experiment::Request, Experiment::Reply>> stream(
                stub_->TestBidirectionalStream(&ctx));
        std::thread writer([stream]() {
            Experiment::Request request;
            request.set_name("zhao");
            request.set_id(1024);
            for (int i = 0; i < 5; ++i) {
                request.set_value(i);
                stream->Write(request);
            }
            stream->WritesDone();
        });

        while(stream->Read(&response)){
            std::cout<< "Function: SendTestBidrectionalStream"
                     << "\ttid:" << response.tid()
                     << "\tsvrname:" << response.svrname()
                    << "\textend:" << response.extend()
                     << "\ttakeuptime: " << response.takeuptime() << std::endl;
        }
        writer.join();
        grpc::Status st = stream->Finish();
        // check
        if(st.ok()){
            std::cout << "SendTestBidrectionalStream rpc succeeded." << std::endl;
        }else{
            std::cout << "SendTestBidrectionalStream rpc failed. Code: " << st.error_code()
                      << "\t Message:" << st.error_message();
        }

    }

private:
    std::shared_ptr<Experiment::Server::Stub> stub_;
};

int main(){
    SyncClient client(grpc::CreateChannel("localhost:33333", grpc::InsecureChannelCredentials()));
    client.Run(10);
    return 0;
}
