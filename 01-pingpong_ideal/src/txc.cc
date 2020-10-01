#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Txc : public cSimpleModule
{
    private:
        int seq;
        // Self-message for timing
        cMessage *processingTimer;
        // Memory for sending back the received message
        cMessage *messageBuffer;
        // Timeout for resending message
        simtime_t timeout;
        // The resending message
        cMessage *timeoutEvent;
    
    public:
        Txc();
        virtual ~Txc();

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual cMessage* generateNewMessage();
        virtual void sendCopyOf(cMessage *msg);
};

Define_Module(Txc);

Txc::Txc()
{
    // This is nedded, otherwise the destructor will crash if
    // the initialize() isn't called because of runtime error
    processingTimer = messageBuffer = nullptr;
    timeoutEvent = nullptr;
}

Txc::~Txc()
{
    // Force delete dynamically allocated objs
    cancelAndDelete(processingTimer);
    cancelAndDelete(timeoutEvent);
    delete messageBuffer;
}

void Txc::initialize()
{
    // Counter for message names
    seq = 0;

    // Creates the timer event -- an ordinary message
    processingTimer = new cMessage("processingTimer");
    // No message yet
    messageBuffer = nullptr;

    // Timeout configs
    timeout = 10.0;
    timeoutEvent = new cMessage("timeoutEvent");

    // Schedules the first message sending to t = 5.0s
    // using a scheduled "self-message"
    if (par("sendMsgOnInit").boolValue())
    {
        EV << "Scheduling the first sending to t = 5.0s\n";
        messageBuffer = new cMessage("messageBuffer");
        scheduleAt(5.0, processingTimer);
    }
}

void Txc::handleMessage(cMessage *msg)
{
    if (msg == processingTimer)
    {
        // If the arriving message is the timing event, sends the tictocMsg
        EV << "Waiting period is over. Sending a new message.\n";
        messageBuffer = generateNewMessage();
        sendCopyOf(messageBuffer);
        // Begins the timeout counter
        scheduleAt(simTime() + timeout, timeoutEvent);
    }
    else if (msg == timeoutEvent)
    {
        EV << "Message timeout reached. Sending again and restarting timer.\n";
        sendCopyOf(messageBuffer);
        scheduleAt(simTime() + timeout, timeoutEvent);
    }
    else
    {
        // If the arriving message is the tictocMsg, stores it and starts the
        // processing timer
        simtime_t delay = par("delayTime");
        EV << "Message arrived, starting " << delay << " secs processing...\n";
        messageBuffer = msg;

        // Cancels the timeout
        cancelEvent(timeoutEvent);
        // If was processing the previous message, abort
        cancelEvent(processingTimer);
        // With a small probability, the message will be lost
        if (uniform(0, 1) < 0.1) {
            EV << "Losing message\n";
            bubble("message lost");  // displays in the UI
            delete msg;
            return;
        }

        scheduleAt(simTime() + delay, processingTimer);
    }
}

cMessage* Txc::generateNewMessage()
{
    // Generates a message with custom name
    char msgName[20];
    sprintf(msgName, "m-%d", ++seq);
    cMessage *msg = new cMessage(msgName);
    return msg;
}

void Txc::sendCopyOf(cMessage *msg)
{
    // Duplicates and sends the copy of a message
    cMessage *copy = (cMessage *) msg->dup();
    send(copy, "out");
}