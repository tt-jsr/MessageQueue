#ifndef MESSAGE_QUEUE_H_
#define MESSAGE_QUEUE_H_

#include <Arduino.h>
#include "message_queue_impl.h"

const int IDLE_EVENT = 1;
const int TIMER_EVENT = 2;
const int STATE_EVENT = 3;
const int DIGITAL_READ_EVENT = 4;
const int ANALOG_READ_EVENT = 5;
const int USER_EVENT_BASE = 100;

class MessageQueue
{
public:
    /*
    queue_size 
        The max number of messages the queue can hold. This MUST be a power of two (4, 8, 16, 32, ...).
        8 or 16 are probably good sizes to start with

        Each queue slot takes 8 bytes

        Size of this object is 8+8*queue_size bytes
    */
    MessageQueue(const int queue_size);

    // Post a message to the queue, if the queue is full, the oldest message
    // will be dropped.
    void post_message(int msg, int arg1, unsigned long arg2);

    // Get and remove a message from the queue. 
    // If there are no events available in the queue, an IDLE_EVENT will be returned.
    //
    // This call will always return either a queued message, or IDLE_EVENT
    void get_message(int& msg, int& arg1, unsigned long& arg2);

    // Get a message without removing it from the queue.
    // #define INCLUDE_PEEK_MESSAGE to use this function
#if defined(INCLUDE_PEEK_MESSAGE)
    bool peek_message(int& msg, int& arg, unsigned long& arg2);
#endif

    // Enable printing to the monitor for debugging
    void setDebug(bool debug);

private:
    unsigned int consumer_;
    unsigned int producer_;
    const int MAX_MESSAGES;
    Message *msg_queue_;
    bool debug_;
};

/************************************************************************************/


/* Read a digital pin. An event will only be posted
 * if the state changes.
 * Template parameters
 *   ID: The identifier for this object
 *  PIN: The digital pin to read
 *
 * Constructor:
 *    def: The default state of this pin, either HIGH or LOW
 *   mode: Either INPUT or INPUT_PULLUP
 *
 * get_message:
 *    msg: DIGITAL_READ_EVENT
 *   arg1: ID
 *   arg2: value, HIGH or LOW
 */
template <int ID, int PIN>
struct DigitalRead
{
    enum values {
        id = ID,
        pin = PIN
    };

    // def: The starting state, HIGH or LOW
    // mode: INPUT or INPUT_PULLUP. Set the pinMode
    DigitalRead(int def, int mode = INPUT);

    // Call this in your loop() function
    // Returns true if the state changed
    bool execute(MessageQueue *mq = nullptr);

    // Return the current state, HIGH or LOW
    int getState();

    int state;
};

/* Read a digital pin with debounce. An event will only be posted
 * if the state changes.
 *
 * Template parameters
 *   ID: The identifier for this object
 *  PIN: The digital pin to read
 *   DB: The debounce time in microseconds
 *
 * Constructor:
 *    def: The default state of this pin, either HIGH or LOW
 *   mode: Either INPUT or INPUT_PULLUP
 *
 * get_message:
 *    msg: DIGITAL_READ_EVENT
 *   arg1: ID
 *   arg2: value, HIGH or LOW
 */
template <int ID, int PIN, unsigned long DB>
struct DigitalReadDebounce
{
    enum values {
        id = ID,
        pin = PIN
    };

    // def: The starting state, HIGH or LOW
    // mode: INPUT or INPUT_PULLUP. Set the pinMode
    DigitalReadDebounce(int def, int mode = INPUT);

    // Call this in your loop() function
    // Returns true if the state changed
    bool execute(MessageQueue *mq = nullptr);

    // Get the current state, HIGH or LOW
    int getState();

    int state;
    unsigned long t;
};
    
/* Read a analog pin. Will post an event only if the
 * value changes.
 *
 * Template parameters
 *   ID: The identifier for this object
 *  PIN: The digital pin to read
 *
 * get_message:
 *    msg: ANALOG_READ_EVENT
 *   arg1: ID
 *   arg2: value, 0-1023
 */
template <int ID, int PIN>
struct AnalogRead
{
    enum values {
        id = ID,
        pin = PIN
    };

    AnalogRead();

    // Call this in your loop function
    bool execute(MessageQueue *mq = nullptr);

    // Get the current value, 0-1023
    int getValue();

    int value;
};

/* Read a analog pin at an interval. 
 * Will post an event only if the value has changed at the 
 * given interval. The interval time will be adjusted to maintain
 * on average, the specified interval.
 *
 * Template parameters
 *        ID: The identifier for this object
 *       PIN: The digital pin to read
 *  INTERVAL: The read interval, in microseconds
 *
 * get_message:
 *    msg: ANALOG_READ_EVENT
 *   arg1: ID
 *   arg2: value
 */
template <int ID, int PIN, unsigned long INTERVAL>
struct AnalogReadInterval
{
    enum values {
        id = ID,
        pin = PIN
    };

    AnalogReadInterval();

    // Call in your loop() function
    bool execute(MessageQueue *mq = nullptr);

    // Get the current value, 0-1023
    int getValue();

    int value;
    unsigned long readTime;
};

/* Set a oneshot or repeating timer
 *
 * Template parameters
 *        ID: The identifier for this object
 *       PIN: The digital pin to read
 *  INTERVAL: The timer interval, in milliseconds
 *
 * Constructor:
 *    start: True to start the timer right away, false to not start it.
 *           Use reset() to start the timer
 *
 * get_message:
 *    msg: TIMER_EVENT
 *   arg1: ID
 *   arg2: 0
 */
template <int ID, int INTERVAL, bool REPEAT>
struct Timer
{
    enum values {
        id = ID,
    };

    // start: true to start the timer immediatly
    Timer(bool start = true);

    // Call in your loop() function
    bool execute(MessageQueue *mq = nullptr);

    // If paused, will restart the timer at the given interval
    // if not paused, will reset the timer to trigger at the interval
    // from now
    void reset();

    // Pause the timer
    void pause();

    int trigger;
};

/* Pulse an output for a given duration
 *
 * Template parameters
 *       ID: The identifier for this object
 *      PIN: The digital pin to output
 * DURATION: The pulse duration, in microseconds
 *    STATE: Either HIGH or LOW for the first pulse
 *
 * get_message:
 *    No events for this function
 */
template <int ID, int PIN, unsigned long DURATION, int STATE>
struct Pulse
{
    enum values {
        id = ID,
        pin = PIN
    };

    Pulse();

    // Call in your loop() function
    int execute();

    // Call to trigger the pulse
    void trigger();

    unsigned long trigger_;
};

/* Create a repeating pulse with the given duty cycle.
 * This class will adjust the pulse widths to try to keep the
 * average state transistion time as close to the given
 * durations as possible. This is not high precision so you should 
 * expect some drift.
 *
 * Template parameters
 *            ID: The identifier for this object
 *           PIN: The digital pin to output.
 * DURATION_HIGH: Duration of the HIGH state, in microseconds
 *  DURATION_LOW: Duration of the LOW state, in microseconds
 *
 * get_message:
 *    No events for this function
 */
template <int ID, int PIN, unsigned long DURATION_HIGH, unsigned long DURATION_LOW>
struct PulseRepeat
{
    enum values {
        id = ID,
        pin = PIN
    };

    PulseRepeat();

    // Call in your loop() function
    bool execute();

    // Run the repeating pulses starting with the given state, HIGH or LOW
    void run(int startState);

    // Stop the pulses, will immediatly write the given state.
    void stop(int endState);

    unsigned long trigger_;
    int state;
};

/* Create a state variable
 *
 * Template parameters
 *   ID: The identifier for this object
 *    T: One of the integer types: int, long, uint_8, ...
 *
 * Constructor:
 *    v: The vaue to store
 *
 * get_message:
 *    msg: STATE_EVENT
 *   arg1: ID
 *   arg2: value, the value when set() was called, if it changed
 */
template <int ID, typename T>
struct State
{
    enum values {
        id = ID,
    };
    typedef T value_type;

    State(const T& v);

    // Call to set a new value. if the value changes, a STATE_EVENT 
    // will be posted
    bool set(const T& v, MessageQueue *mq = nullptr);

    // Toggle the state between 0 and 1
    bool toggle(MessageQueue *mq = nullptr);

    // Get the current state
    T getState();

    T state;
};

#endif

#include "message_queue_inl.h"

