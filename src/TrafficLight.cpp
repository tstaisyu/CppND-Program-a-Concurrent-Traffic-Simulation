#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> lck(_mutex);
    // Source: https://knowledge.udacity.com/questions/444528
    _condition.wait(lck, [this] {return !_queue.empty(); });
    T newMessage = std::move(_queue.back());
    _queue.pop_back();
    return newMessage;

}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mutex);
    _queue.emplace_back(msg);
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight()
{

}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (TrafficLight::getCurrentPhase() != TrafficLightPhase::green) {
        TrafficLight::_messageQueue.receive();
    }
}

TrafficLight::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    // Source: https://qiita.com/Nyarll/items/b390b5c41e8a90f5172a
    // A argument "this" is necessary with using methods in the same class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    
    // Source: https://www.techiedelight.com/ja/generate-random-numbers-in-the-specified-range-in-cpp/
    // Source: https://knowledge.udacity.com/questions/586372
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<int> distr(4000, 6000);
    int cycleDuration = distr(eng);
    auto now = std::chrono::system_clock::now();
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto end = std::chrono::system_clock::now();
        auto lambda = std::chrono::duration_cast<std::chrono::milliseconds>(end - now).count();
        if (lambda > cycleDuration) {
            if (_currentPhase == TrafficLightPhase::red) {
                _currentPhase = TrafficLightPhase::green;
            } else {
                _currentPhase = TrafficLightPhase::red;
            }
            _messageQueue.send(std::move(_currentPhase));
            now = std::chrono::system_clock::now();
            cycleDuration = distr(eng);
        }
    }
}

