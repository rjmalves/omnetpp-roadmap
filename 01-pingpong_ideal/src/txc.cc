#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "pingpong_m.h"

using namespace omnetpp;

class Txc : public cSimpleModule
{
    private:
        int seq;
        // Self-message for timing
        cMessage *processingTimer;
        // Memory for sending back the received message
        PingPongMsg *messageBuffer;
        // Timeout for resending message
        simtime_t timeout;
        // The resending message
        cMessage *timeoutEvent;
        // For stats collection
        long numSent;
        long numReceived;
        cLongHistogram processingTimeStats;
        cOutVector processingTimeVector;
    
    public:
        Txc();
        virtual ~Txc();

    protected:
        virtual void initialize() override;
        virtual void finish() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual PingPongMsg* generateNewMessage();
        virtual void sendCopyOf(PingPongMsg *msg);
        virtual void refreshDisplay() const override;
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
    delete messageBuffer;
    cancelAndDelete(processingTimer);
    cancelAndDelete(timeoutEvent);
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

    // Stats collection variables
    numSent = numReceived = 0;
    WATCH(numSent);
    WATCH(numReceived);

    // Schedules the first message sending to t = 5.0s
    // using a scheduled "self-message"
    if (par("sendMsgOnInit").boolValue())
    {
        EV << "Scheduling the first sending to t = 5.0s\n";
        scheduleAt(5.0, processingTimer);
    }
}

void Txc::finish()
{
    // This function is called by OMNeT++ at the end of the simulation.
    EV << "Sent:     " << numSent << endl;
    EV << "Received: " << numReceived << endl;
    EV << "Processing times, min:    " << processingTimeStats.getMin() << endl;
    EV << "Processing times, max:    " << processingTimeStats.getMax() << endl;
    EV << "Processing times, mean:   " << processingTimeStats.getMean() << endl;
    EV << "Processing times, stddev: " << processingTimeStats.getStddev() << endl;

    recordScalar("#sent", numSent);
    recordScalar("#received", numReceived);

    processingTimeStats.recordAs("Processing times");
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
        PingPongMsg *ttmsg = check_and_cast<PingPongMsg *>(msg);
        numReceived++;
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

        // If the arriving message is the tictocMsg, stores it and starts the
        // processing timer
        simtime_t delay = par("delayTime");
        EV << "Message arrived, starting " << delay << " secs processing...\n";
        messageBuffer = ttmsg;
        scheduleAt(simTime() + delay, processingTimer);
        processingTimeVector.record(delay);
        processingTimeStats.collect(delay);
    }
}

PingPongMsg* Txc::generateNewMessage()
{
    // Generates a message with custom name
    char msgName[20];
    sprintf(msgName, "m-%d", ++seq);
    PingPongMsg *msg = new PingPongMsg(msgName);
    msg->setSource(getIndex());
    msg->setDestination(1);
    return msg;
}

void Txc::sendCopyOf(PingPongMsg *msg)
{
    // Duplicates and sends the copy of a message
    PingPongMsg *copy = check_and_cast<PingPongMsg *>(msg->dup());
    send(copy, "out");
    numSent++;
}

void Txc::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "rcvd: %ld sent: %ld", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}