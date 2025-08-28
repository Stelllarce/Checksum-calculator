#include "progress-indicator-observers/Observable.hpp"
#include "progress-indicator-observers/Observer.hpp"
#include "progress-indicator-observers/Message.hpp"
#include <catch2/catch_all.hpp>

class MockObserver : public Observer {
public:
    int update_count = 0;
    Observable* last_sender = nullptr;
    Message::Type last_message_type = Message::Type::NewFile;

    void update(Observable& sender, const Message& m) override {
        update_count++;
        last_sender = &sender;
        last_message_type = m.type;
    }
};

class TestObservable : public Observable {
public:
    void triggerNotify(const Message& m) {
        notify(*this, m);
    }
};

TEST_CASE("Observable Basic Functionality", "[Observable]") {
    TestObservable observable;
    MockObserver observer1;
    MockObserver observer2;

    SECTION("Attach observer and notify") {
        observable.attach(&observer1);
        NewFileMessage msg("/test/file.txt");
        
        observable.triggerNotify(msg);
        
        REQUIRE(observer1.update_count == 1);
        REQUIRE(observer1.last_sender == &observable);
        REQUIRE(observer1.last_message_type == Message::Type::NewFile);
    }

    SECTION("Multiple observers receive notifications") {
        observable.attach(&observer1);
        observable.attach(&observer2);
        BytesReadMessage msg(100);
        
        observable.triggerNotify(msg);
        
        REQUIRE(observer1.update_count == 1);
        REQUIRE(observer2.update_count == 1);
        REQUIRE(observer1.last_message_type == Message::Type::BytesRead);
        REQUIRE(observer2.last_message_type == Message::Type::BytesRead);
    }

    SECTION("Detach observer stops notifications") {
        observable.attach(&observer1);
        observable.attach(&observer2);
        
        NewFileMessage msg1("/test/file1.txt");
        observable.triggerNotify(msg1);
        
        REQUIRE(observer1.update_count == 1);
        REQUIRE(observer2.update_count == 1);
        
        observable.detach(&observer1);
        
        NewFileMessage msg2("/test/file2.txt");
        observable.triggerNotify(msg2);
        
        REQUIRE(observer1.update_count == 1);
        REQUIRE(observer2.update_count == 2);
    }

    SECTION("Attach same observer multiple times only adds once") {
        observable.attach(&observer1);
        observable.attach(&observer1);
        observable.attach(&observer1);
        
        NewFileMessage msg("/test/file.txt");
        observable.triggerNotify(msg);
        
        REQUIRE(observer1.update_count == 1);
    }
}

TEST_CASE("Observable Edge Cases", "[Observable]") {
    TestObservable observable;
    MockObserver observer1;

    SECTION("Attach null observer is handled safely") {
        observable.attach(nullptr);
        
        NewFileMessage msg("/test/file.txt");
        observable.triggerNotify(msg);
        
        REQUIRE(observer1.update_count == 0);
    }

    SECTION("Detach null observer is handled safely") {
        observable.attach(&observer1);
        observable.detach(nullptr);
        
        NewFileMessage msg("/test/file.txt");
        observable.triggerNotify(msg);
        
        REQUIRE(observer1.update_count == 1);
    }

    SECTION("Detach non-attached observer is handled safely") {
        MockObserver observer2;
        observable.attach(&observer1);
        observable.detach(&observer2);
        
        NewFileMessage msg("/test/file.txt");
        observable.triggerNotify(msg);
        
        REQUIRE(observer1.update_count == 1);
    }

    SECTION("Detach all observers") {
        MockObserver observer2;
        observable.attach(&observer1);
        observable.attach(&observer2);
        
        observable.detach(&observer1);
        observable.detach(&observer2);
        
        NewFileMessage msg("/test/file.txt");
        observable.triggerNotify(msg);
        
        REQUIRE(observer1.update_count == 0);
        REQUIRE(observer2.update_count == 0);
    }

    SECTION("Multiple notifications to same observers") {
        observable.attach(&observer1);
        
        NewFileMessage msg1("/test/file1.txt");
        BytesReadMessage msg2(50);
        BytesReadMessage msg3(100);
        
        observable.triggerNotify(msg1);
        observable.triggerNotify(msg2);
        observable.triggerNotify(msg3);
        
        REQUIRE(observer1.update_count == 3);
        REQUIRE(observer1.last_message_type == Message::Type::BytesRead);
    }
}