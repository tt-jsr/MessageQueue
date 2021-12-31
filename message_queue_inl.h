#ifndef MESSAGE_QUEUE_INL_H_
#define MESSAGE_QUEUE_INL_H_

bool DigitalReadExecute(int& state, int ID, int PIN, MessageQueue *mq);
bool AnalogReadExecute(int& value, int ID, int PIN, MessageQueue *mq);
bool AnalogReadIntervalExecute(int& value, unsigned long& readTime, int ID, int PIN, unsigned long INTERVAL, MessageQueue *mq);
bool DigitalReadDebounceExecute(int& state, unsigned long& t, int ID, int PIN, unsigned long DB, MessageQueue *mq);
bool TimerExecute(unsigned long& trigger, int ID, int INTERVAL, bool REPEAT, MessageQueue *mq);
bool PulseRepeatExecute(int& state, unsigned long& trigger, int PIN, unsigned long DURATION_HIGH, unsigned long DURATION_LOW);
bool PulseExecute(unsigned long& trigger, int PIN, unsigned long DURATION, int STATE);
void PulseRepeatRun(int& state, unsigned long& trigger, int PIN, unsigned long DURATION_HIGH, unsigned long DURATION_LOW, int startState);
bool StateExecute(int& state, int ID, unsigned long v);

/*****************************************************************/
template <int ID, int PIN>
DigitalRead<ID, PIN>::DigitalRead(int def, int mode)
:state(def)
{
    pinMode(PIN, mode);
}

template <int ID, int PIN>
bool DigitalRead<ID, PIN>::execute(MessageQueue *mq)
{
    return DigitalReadExecute(state, ID, PIN, mq);
}

template <int ID, int PIN>
int DigitalRead<ID, PIN>::getState() {return state;}

/*****************************************************************/

template <int PIN>
DigitalWrite<PIN>::DigitalWrite()
{
    pinMode(PIN, OUTPUT);
}

template <int PIN>
void DigitalWrite<PIN>::high()
{
    digitalWrite(PIN, HIGH);
}

template <int PIN>
void DigitalWrite<PIN>::low()
{
    digitalWrite(PIN, LOW);
}

template <int PIN>
void DigitalWrite<PIN>::write(int v)
{
    digitalWrite(PIN, v);
}

/*****************************************************************/

template <int ID, int PIN>
AnalogRead<ID, PIN>::AnalogRead()
:value(0)
{
}

template <int ID, int PIN>
bool AnalogRead<ID, PIN>::execute(MessageQueue *mq)
{
    return AnalogReadExecute(value, ID, PIN, mq);
}

template <int ID, int PIN>
int AnalogRead<ID, PIN>::getValue() {return value;}

/****************************************************************/

template <int ID, int PIN, unsigned long INTERVAL>
AnalogReadInterval<ID, PIN, INTERVAL>::AnalogReadInterval()
:value(0)
,readTime(0)
{
}

template <int ID, int PIN, unsigned long INTERVAL>
bool AnalogReadInterval<ID, PIN, INTERVAL>::execute(MessageQueue *mq)
{
    return AnalogReadIntervalExecute(value, readTime, ID, PIN, INTERVAL, mq);
}

template <int ID, int PIN, unsigned long INTERVAL>
int AnalogReadInterval<ID, PIN, INTERVAL>::getValue() {return value;}

/***************************************************************/

template <int ID, int PIN, unsigned long DB>
DigitalReadDebounce<ID, PIN, DB>::DigitalReadDebounce(int def, int mode)
:state(def)
,t(0)
{
    pinMode(PIN, mode);
}

template <int ID, int PIN, unsigned long DB>
bool DigitalReadDebounce<ID, PIN, DB>::execute(MessageQueue *mq)
{
    return DigitalReadDebounceExecute(state, t, ID, PIN, DB, mq);
}

template <int ID, int PIN, unsigned long DB>
int DigitalReadDebounce<ID, PIN, DB>::getState() {return state;}

/*************************************************************/

template <int ID, int INTERVAL, bool REPEAT>
Timer<ID, INTERVAL, REPEAT>::Timer(bool start)
:trigger(0)
{
    if (start)
        reset();
}

template <int ID, int INTERVAL, bool REPEAT>
bool Timer<ID, INTERVAL, REPEAT>::execute(MessageQueue *mq)
{
    return TimerExecute(trigger, ID, INTERVAL, REPEAT, mq);
}

template <int ID, int INTERVAL, bool REPEAT>
void Timer<ID, INTERVAL, REPEAT>::reset()
{
    trigger = millis() + INTERVAL;
}

template <int ID, int INTERVAL, bool REPEAT>
void Timer<ID, INTERVAL, REPEAT>::pause()
{
    trigger = 0;
}

/******************************************************************/

template <int PIN, unsigned long DURATION, int STATE>
Pulse<PIN, DURATION, STATE>::Pulse()
:trigger_(0)
{
    pinMode(PIN, OUTPUT);
    digitalWrite(PIN, !STATE);
}

template <int PIN, unsigned long DURATION, int STATE>
int Pulse<PIN, DURATION, STATE>::execute()
{
    return PulseExecute(trigger_, PIN, DURATION, STATE);
}

template <int PIN, unsigned long DURATION, int STATE>
void Pulse<PIN, DURATION, STATE>::trigger()
{
    trigger_ = micros() + DURATION;
    digitalWrite(PIN, STATE);
}

/********************************************************************/

template <int PIN, unsigned long DURATION_HIGH, unsigned long DURATION_LOW>
PulseRepeat<PIN, DURATION_HIGH, DURATION_LOW>::PulseRepeat()
:trigger_(0)
{
    pinMode(PIN, OUTPUT);
}

template <int PIN, unsigned long DURATION_HIGH, unsigned long DURATION_LOW>
bool PulseRepeat<PIN, DURATION_HIGH, DURATION_LOW>::execute()
{
    return PulseRepeatExecute(state, trigger_, PIN, DURATION_HIGH, DURATION_LOW);
}

template <int PIN, unsigned long DURATION_HIGH, unsigned long DURATION_LOW>
void PulseRepeat<PIN, DURATION_HIGH, DURATION_LOW>::run(int startState)
{
    PulseRepeatRun(state, trigger_, PIN, DURATION_HIGH, DURATION_LOW, startState);
}

template <int PIN, unsigned long DURATION_HIGH, unsigned long DURATION_LOW>
void PulseRepeat<PIN, DURATION_HIGH, DURATION_LOW>::stop(int endState)
{
    digitalWrite(PIN, endState);
    trigger_ = 0;
}

/***************************************************************/

template <int ID, typename T>
State<ID, T>::State(const T& v)
:state(v)
{}

template <int ID, typename T>
bool State<ID, T>::set(const T& v, MessageQueue *mq)
{
    return StateExecute(state, ID, (unsigned long)v);
}

template <int ID, typename T>
bool State<ID, T>::toggle(MessageQueue *mq)
{
    return set(!state, mq);
}

template <int ID, typename T>
T State<ID, T>::getState() {return state;}


#endif
