#include "Observable.hpp"
#include "Observer.hpp"
#include "Message.hpp"

void Observable::notify(Observable& sender, const Message& m) {
    auto copy = _observers;
    for (auto* obs : copy) {
        if (obs) obs->update(sender, m);
    }
}

void Observable::detach(Observer* obs) {
        if (!obs) return;
        auto it = std::remove(_observers.begin(), _observers.end(), obs);
        _observers.erase(it, _observers.end());
}

void Observable::attach(Observer* obs) {
        if (!obs) return;
        auto it = std::find(_observers.begin(), _observers.end(), obs);
        if (it == _observers.end()) _observers.push_back(obs);
}