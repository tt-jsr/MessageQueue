#include "message_queue.h"

MessageQueue::MessageQueue(const int nmsg)
:consumer_(0)
,producer_(0)
,MAX_MESSAGES(nmsg)
,msg_queue_(new Message[MAX_MESSAGES])
, debug_(false)
{
   memset(msg_queue_, 0, MAX_MESSAGES*sizeof(Message));
}

void MessageQueue::post_message(int msg, int arg1, unsigned long arg2)
{
    msg_queue_[producer_].msg_type = msg;
    msg_queue_[producer_].arg1 = arg1;
    msg_queue_[producer_].arg2 = arg2;
    producer_ = (++producer_) & (MAX_MESSAGES-1);
    if (producer_ == consumer_)
    {
        // If we just posted and reached the consumer_, our queue is full
        // so advance the consumer and drop the oldest message
        ++consumer_;
        //if (debug_)
            //Serial.println("MessageQueue: Queue full, message dropped");
    }
}

void MessageQueue::get_message(int& msg, int& arg1, unsigned long& arg2)
{
    if (producer_ == consumer_)
    {
        msg = IDLE_EVENT;
        arg1 = 0;
        arg2 = 0;
        return;
    }

    msg = msg_queue_[consumer_].msg_type;
    arg1 = msg_queue_[consumer_].arg1;
    arg2 = msg_queue_[consumer_].arg2;
    consumer_ = (++consumer_) & (MAX_MESSAGES-1);
}

#if defined(INCLUDE_PEEK_MESSAGE)
bool MessageQueue::peek_message(int& msg, int& arg1, unsigned long& arg2)
{
    if (producer_ == consumer_)
    {
        msg_queue_[consumer_].msg_type = IDLE_EVENT;
        msg_queue_[consumer_].arg1 = 0;
        msg_queue_[consumer_].arg2 = 0;
        return false;
    }

    msg = msg_queue_[consumer_].msg_type;
    arg1 = msg_queue_[consumer_].arg1;
    arg2 = msg_queue_[consumer_].arg2;
    return true;
}
#endif

void MessageQueue::setDebug(bool dbg)
{
    debug_ = dbg;
}

/***************************************************************************/
bool DigitalReadExecute(int& state, int ID, int PIN, MessageQueue *mq)
{
    int s = digitalRead(PIN);
    if (s != state)
    {
        if (mq)
            mq->post_message(DIGITAL_READ_EVENT, ID, s);
        state = s;
        return true;
    }
    return false;
}

bool AnalogReadExecute(int& value, int ID, int PIN, MessageQueue *mq)
{
    int s = analogRead(PIN);
    if (s != value)
    {
        if (mq)
            mq->post_message(ANALOG_READ_EVENT, ID, s);
        value = s;
        return true;
    }
    return false;
}

bool AnalogReadIntervalExecute(int& value, unsigned long& readTime, int ID, int PIN, unsigned long INTERVAL, MessageQueue *mq)
{
    unsigned long now = micros();
    if (readTime > now)
        return false;
    unsigned long delay = now-readTime;
    readTime = now + (INTERVAL - delay);
    int s = analogRead(PIN);
    if (s != value)
    {
        if (mq)
            mq->post_message(ANALOG_READ_EVENT, ID, s);
        value = s;
        return true;
    }
    return false;
}


bool DigitalReadDebounceExecute(int& state, unsigned long& t, int ID, int PIN, unsigned long DB, MessageQueue *mq)
{
    int s = digitalRead(PIN);
    if (t == 0)
    {
        if (s == state)
            return false;
        t = micros() + DB;
        return false;
    }
    else
    {
        if (t < micros())
        {
            if (s != state)
            {
                if (mq)
                    mq->post_message(DIGITAL_READ_EVENT, ID, 0);
                state = s;
                t = 0;
                return true;
            }
            else
            {
                t = 0;
            }
        }
    }
    return false;
}

bool TimerExecute(int& trigger, int ID, int INTERVAL, bool REPEAT, MessageQueue *mq)
{
    if (trigger == 0)
        return false;
    if (trigger < millis())
    {
        if (mq)
            mq->post_message(TIMER_EVENT, ID, 0);
        if (REPEAT)
            trigger = millis() + INTERVAL;
        else
            trigger = 0;
        return true;
    }
    return false;
}

bool PulseExecute(unsigned long& trigger, int ID, int PIN, unsigned long DURATION, int STATE)
{
    if (trigger == 0)
        return false;
    if (trigger < micros())
    {
        digitalWrite(PIN, !STATE);
        trigger = 0;
        return true;
    }
    return false;
}

bool PulseRepeatExecute(int& state, unsigned long& trigger, int ID, int PIN, unsigned long DURATION_HIGH, unsigned long DURATION_LOW)
{
    if (trigger == 0)
        return false;
    unsigned long now = micros();
    unsigned long delayed = now-trigger;
    if (trigger < now)
    {
        if (state == HIGH)
        {
            state = LOW;
            digitalWrite(PIN, LOW);
            trigger = now + (DURATION_LOW-delayed);
        }
        else
        {
            state = HIGH;
            digitalWrite(PIN, HIGH);
            trigger = now + (DURATION_HIGH-delayed);
        }
        return true;
    }
}

void PulseRepeatRun(int& state, unsigned long& trigger, int PIN, unsigned long DURATION_HIGH, unsigned long DURATION_LOW, int startState)
{
    if (startState == HIGH)
        trigger = micros() + DURATION_HIGH;
    else
        trigger = micros() + DURATION_LOW;
    state = startState;
    digitalWrite(PIN, state);
}

bool StateExecute(int& state, int ID, unsigned long v, MessageQueue *mq)
{
    if (v != state)
    {
        if (mq)
            mq->post_message(STATE_EVENT, ID, v);
        state = v;
        return true;
    }
    return false;
}

