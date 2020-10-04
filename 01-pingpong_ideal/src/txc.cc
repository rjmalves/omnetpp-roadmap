#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "pingpong_m.h"

using namespace omnetpp;

class Txc : public cSimpleModule
{
    private:
        // Node identifier
        int address;
        int msgDestination;
        // Self-message for timing
        cMessage *processingTimer;
        // Memory for sending back the received message
        PingPongMsg *messageBuffer;
        // Timeout for resending message
        simtime_t timeout;
        // The resending message
        cMessage *timeoutEvent;
        // For stats collection
        simsignal_t processingTime;
        simsignal_t latencyTime;
    
    public:
        Txc();
        virtual ~Txc();

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual PingPongMsg* generateNewMessage();
        virtual void sendCopyOf(PingPongMsg *msg);
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
    cancelAndDelete(messageBuffer);
    cancelAndDelete(processingTimer);
    cancelAndDelete(timeoutEvent);
}

void Txc::initialize()
{
    // Creates the timer event -- an ordinary message
    processingTimer = new cMessage("processingTimer");
    // No message yet
    messageBuffer = nullptr;
    // Gets the self address the messages' destination
    address = par("address");
    msgDestination = par("msgDestination");

    // Timeout configs
    timeout = 10.0;
    timeoutEvent = new cMessage("timeoutEvent");

    // Stats collection variables
    latencyTime = registerSignal("arrived");
    processingTime = registerSignal("processed");

    // Schedules the first message sending to t = 5.0s
    // using a scheduled "self-message"
    if (par("sendMsgOnInit").boolValue())
    {
        EV << "Scheduling the first sending to t = 5.0s\n";
        scheduleAt(5.0, processingTimer);
    }
}

void Txc::handleMessage(cMessage *msg)
{
    if (msg == processingTimer)
    {
        // If the arriving message is the timing event, sends the message
        EV << "Waiting period is over. Sending a new message.\n";
        messageBuffer = generateNewMessage();
        sendCopyOf(messageBuffer);
        // Begins the timeout counter
        scheduleAt(simTime() + timeout, timeoutEvent);
    }
    else if (msg == timeoutEvent)
    {
        EV << "Message timeout reached. Sending again and restarting timer.\n";
        messageBuffer->setSendingTime(simTime());
        sendCopyOf(messageBuffer);
        scheduleAt(simTime() + timeout, timeoutEvent);
    }
    else
    {
        PingPongMsg *ttmsg = check_and_cast<PingPongMsg *>(msg);
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
        // Gets the transmission delay
        simtime_t latency = simTime() - ttmsg->getSendingTime();
        // Stores it and starts the processing timer
        simtime_t delay = par("delayTime");
        EV << "Message arrived, starting " << delay << " secs processing...\n";
        messageBuffer = ttmsg;
        messageBuffer->setProcessingTime(delay);
        messageBuffer->setRecvTime(0);
        scheduleAt(simTime() + delay, processingTimer);
        // Emits the signal to store the processing time and latency
        emit(latencyTime, latency);
        emit(processingTime, delay);
    }
}

PingPongMsg* Txc::generateNewMessage()
{
    // Generates a message with custom name
    char msgName[20];
    sprintf(msgName, "from:%d,to:%d", address, address + 1);
    PingPongMsg *msg = new PingPongMsg(msgName);
    msg->setSource(address);
    msg->setDestination(msgDestination);
    return msg;
}

void Txc::sendCopyOf(PingPongMsg *msg)
{
    // Duplicates and sends the copy of a message
    msg->setSendingTime(simTime());
    send(check_and_cast<PingPongMsg *>(msg->dup()), "out");
}
