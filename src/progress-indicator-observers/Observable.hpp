#pragma once
#include <vector>
#include <algorithm>

class Message;
class Observer;

/**
 * @class Observable
 * @brief Subject role in the Observer pattern.
 */
class Observable {
public:
    virtual ~Observable() = default;

    virtual void attach(Observer* obs);

    virtual void detach(Observer* obs);

protected:
    void notify(Observable& sender, const Message& m);

private:
    std::vector<Observer*> _observers;
};