#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> ulock(_mutex);
    _condition.wait(ulock, [this] {return !_queue.empty(); }); // pass unique lock to condition variable

    // remove first message from queue (FIFO)
    // T msg = std::move(_queue[0]);
    // _queue.erase(_queue.begin());

    T msg = std::move(_queue.back());
    _queue.clear();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();  // notify the receiver after pushing new message in the queue
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while (_trafficLightPhasesQueue.receive() != TrafficLightPhase::green)
    {
        // keep looping while the TrafficLightPhase is not green
    }
    
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
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

    // init variables
    double cycleDuration = 5; // duration of a single simulation cycle in s
    std::chrono::time_point<std::chrono::steady_clock> lastUpdate;
    // Create a random device and seed the random number generator
    std::random_device rd;
    std::mt19937 gen(rd()); //Mersenne Twister random number generator
    std::uniform_real_distribution<float> dist(4000.0, 6000.0); // Random duration between 4 and 6 seconds
    


    // init stop watch
    lastUpdate = std::chrono::steady_clock::now();
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // Pick a random cycle duration
        cycleDuration = dist(gen);

        // compute the time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - lastUpdate
        ).count();

        if (timeSinceLastUpdate >= cycleDuration)
        {
            // toggle between red and green TrafficLightPhase
            if(_currentPhase == TrafficLightPhase::red)
            {
                _currentPhase = TrafficLightPhase::green;
            }
            else 
            {
                _currentPhase = TrafficLightPhase::red;
            }

            // send update message to queue
            TrafficLightPhase updateMsg = _currentPhase;
            _trafficLightPhasesQueue.send(std::move(updateMsg));

            // reset stop watch for next cycle
            lastUpdate = std::chrono::steady_clock::now();
        }
    }
    
}

