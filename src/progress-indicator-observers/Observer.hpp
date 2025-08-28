#pragma once

class Observable;
struct Message;

/**
* @interface Observer
* @brief Receives notifications from an Observable subject.
*/
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(Observable& sender, const Message& m) = 0;
};