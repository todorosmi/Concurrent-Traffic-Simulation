#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"

using namespace std::chrono;

/* Implementation of class "MessageQueue" */

/* 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
}
*/

 template <typename T> T MessageQueue<T>::receive() {
    // create lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_queue.empty(); });

    // dequeue
    T msg = std::move(_queue.back());
    // _queue.pop_back(); // can't use pop_back since the outer intersections don't get a lot of traffic and the queue fills up
    _queue.clear();

    // return object
    return msg;
}
/*
template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
}
*/

template <typename T> void MessageQueue<T>::send(T &&msg) {
    std::lock_guard<std::mutex> gLock(_mutex);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */


void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if(_messageQueue.receive() == green){
            return;
        }
    }
    
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(4.0, 6.0);
    float cycle = dist(mt);

    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> timePassed;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        timePassed = std::chrono::high_resolution_clock::now() - start;

        if (timePassed.count() > cycle) {
            _currentPhase = (_currentPhase == TrafficLightPhase::green) ? red : green;
            std::cout << "Current phase: " << _currentPhase << "\n";

            start = std::chrono::high_resolution_clock::now();
            cycle = dist(mt);

            TrafficLightPhase msgPhase = _currentPhase;
            _messageQueue.send(std::move(msgPhase));

        }
    }
}

