# MessageQueue 
The MessageQueue class provides an event driven framework for processing messages.

## Timers
    Timers are created with create_timer(). You can create one shot timers or repeating timers.
    You can cancel and pause timers and well as reset a timer.

    When a timer fires, a TIMER_EVENT is posted to the event queue

## Digital read
    The digitalRead() sets up the reading of a digital pin. You can also provide 
    a timeout for debouncing the input.

    Only when the state of the pin changes from HIGH to LOW or LOW to HIGH will a VALUE_EVENT
    be posted to the queue.

## Analog read
    The analogRead() call sets up the reading of an analog pin. A VALUE_EVENT will be posted 
    either on every call to get_message() or when a specified number of microseconds has elapsed.

    Each read generates a VALUE_EVENT.

## Pulse output
    When toggleing a output pin for a specified duration, the canonical method to to write HIGH,
    delay, then write low. The delay will prevent any other work from being performed. Using
    the MessageQueue allows events to still be processed even during lengthy pulse durations.

## State values
    State values are a mechanism to define a variable that will generate an event only when the value
    actually changes.


================================================================================
## Examples:

MessageQueue mq(8)

Timer<1, 1000> timer;
DigitalReadDebounce<2, 4> button(LOW);
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
    case DIGITAL_READ:
        if (arg1 == button.id)
        {
            Serial.print("Digital pin value: ");
            Serial.print(arg2);
        }  
        break;
    case ANALOG_READ:
        if (arg1 == signal.id)
        {
            Serial.print("Analog pin value: ");
            Serial.print(arg2);
        }
        break;
    case IDLE_EVENT:
        break;
    }
}
