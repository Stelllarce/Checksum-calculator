#include "progress-indicator-observers/ProgressReporter.hpp"
#include "progress-indicator-observers/Message.hpp"
#include "progress-indicator-observers/Observable.hpp"
#include <catch2/catch_all.hpp>
#include <sstream>

class MockObservable : public Observable {
public:
    void sendMessage(const Message& m) {
        notify(*this, m);
    }
};

TEST_CASE("ProgressReporter Basic Functionality", "[ProgressReporter]") {
    std::ostringstream output_stream;
    ProgressReporter reporter(1000, output_stream);
    MockObservable observable;
    
    observable.attach(&reporter);
    reporter.start();

    SECTION("NewFile message updates display") {
        NewFileMessage new_file_msg("/test/file.txt");
        observable.sendMessage(new_file_msg);
        
        std::string output = output_stream.str();
        REQUIRE(output.find("/test/file.txt") != std::string::npos);
        REQUIRE(output.find("0 byte(s) read") != std::string::npos);
    }

    SECTION("BytesRead message updates progress") {
        NewFileMessage new_file_msg("/test/file.txt");
        observable.sendMessage(new_file_msg);
        
        output_stream.str("");
        output_stream.clear();
        
        BytesReadMessage bytes_msg(250);
        observable.sendMessage(bytes_msg);
        
        std::string output = output_stream.str();
        REQUIRE(output.find("250 byte(s) read") != std::string::npos);
        REQUIRE(output.find("25.0%") != std::string::npos);
    }

    SECTION("Multiple BytesRead messages accumulate correctly") {
        NewFileMessage new_file_msg("/test/file.txt");
        observable.sendMessage(new_file_msg);
        
        BytesReadMessage bytes_msg1(100);
        observable.sendMessage(bytes_msg1);
        
        output_stream.str("");
        output_stream.clear();
        
        BytesReadMessage bytes_msg2(250);
        observable.sendMessage(bytes_msg2);
        
        std::string output = output_stream.str();
        REQUIRE(output.find("250 byte(s) read") != std::string::npos);
        REQUIRE(output.find("25.0%") != std::string::npos);
    }

    SECTION("Multiple files update total progress") {
        NewFileMessage file1_msg("/test/file1.txt");
        observable.sendMessage(file1_msg);
        BytesReadMessage bytes1_msg(300);
        observable.sendMessage(bytes1_msg);
        
        NewFileMessage file2_msg("/test/file2.txt");
        observable.sendMessage(file2_msg);
        
        output_stream.str("");
        output_stream.clear();
        
        BytesReadMessage bytes2_msg(200);
        observable.sendMessage(bytes2_msg);
        
        std::string output = output_stream.str();
        REQUIRE(output.find("/test/file2.txt") != std::string::npos);
        REQUIRE(output.find("200 byte(s) read") != std::string::npos);
        REQUIRE(output.find("50.0%") != std::string::npos);
    }
}

TEST_CASE("ProgressReporter Edge Cases", "[ProgressReporter]") {
    SECTION("Zero total expected bytes") {
        std::ostringstream output_stream;
        ProgressReporter reporter(0, output_stream);
        MockObservable observable;
        
        observable.attach(&reporter);
        reporter.start();
        
        NewFileMessage new_file_msg("/test/file.txt");
        observable.sendMessage(new_file_msg);
        BytesReadMessage bytes_msg(100);
        observable.sendMessage(bytes_msg);
        
        std::string output = output_stream.str();
        REQUIRE(output.find("0.0%") != std::string::npos);
    }

    SECTION("Bytes read exceeding total expected") {
        std::ostringstream output_stream;
        ProgressReporter reporter(100, output_stream);
        MockObservable observable;
        
        observable.attach(&reporter);
        reporter.start();
        
        NewFileMessage new_file_msg("/test/file.txt");
        observable.sendMessage(new_file_msg);
        BytesReadMessage bytes_msg(150);
        observable.sendMessage(bytes_msg);
        
        std::string output = output_stream.str();
        REQUIRE(output.find("150.0%") != std::string::npos);
    }
}