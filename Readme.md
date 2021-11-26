# MessageQueue 
The MessageQueue class provides an event driven framework for processing messages.
In addtion to the event queue, there are a number of classes that work with the queue to post events when certain conditions are met.

## Timers
    You can create one shot timers or repeating timers.
    You can cancel and pause timers and well as reset a timer.

    When a timer fires, a TIMER_EVENT is posted to the event queue

## Digital read
    The digitalRead() sets up the reading of a digital pin. You can also provide 
    a timeout for debouncing the input.

    Only when the state of the pin changes from HIGH to LOW or LOW to HIGH will a DIGITAL_READ_EVENT be posted to the queue.

## Analog read
    The analogRead() call sets up the reading of an analog pin. A ANALOG_READ_EVENT will be posted either on every call to get_message() or when a specified number of microseconds has elapsed.

    Each read generates an ANALOG_READ_EVENT.

## Pulse output
    When toggleing a output pin for a specified duration, the canonical method to to write HIGH, delay, then write low. The delay will prevent any other work from being performed. Using the MessageQueue allows events to still be processed even during lengthy pulse durations. There are no events for this class.

## State values
    State values are a mechanism to define a variable that will generate an event only when the value actually changes.


================================================================================
## Examples:

MessageQueue mq(8)

// Create a repeating timer with id=1, and a timeout
// of 1000ms or 1sec
Timer<1, 1000, true> timer;

// Read a digital pin, id=2, pin 4, debounce time of 500us
DigitalReadDebounce<2, 4, 500> button(LOW);

// Read analog pin A0, every 1000us
AnalogReadInterval<3, A0, 1000> signal;

void setup()
{
}

void loop()
{
    int msg, arg1;
    unsigned long arg2;

    timer.execute(&mq);
    button.execute(&mq);
    signal.execute(&mq);

    mq.get_message(msg, arg1, arg2);

    switch(msg)
    {
    case TIMER_EVENT:
        if (arg1 == timer.id)
        {
            Serial.println("Timer fired!");
        }
        break;
    case DIGITAL_READ_EVENT:
        if (arg1 == button.id)
        {
            Serial.print("Digital pin value: ");
            Serial.print(arg2);
        }  
        break;
    case ANALOG_READ_EVENT:
        if (arg1 == signal.id)
        {
            Serial.print("Analog pin value: ");
            Serial.print(arg2);
        }
        break;
    case STATE_EVENT:
        break;
    case IDLE_EVENT:
        break;
    }
}
