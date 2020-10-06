#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "pingpong_m.h"

using namespace omnetpp;

class Node : public cSimpleModule
{
    private:
        // Self-message for trigger the end of message processing
        cMessage *processingEvent;
        // Self-message for trigger the resending (timeout)
        cMessage *timeoutEvent;
        // Memory that stores the message to be sent (needs in case of timeout)
        PingPongMsg *messageBuffer;
        // For stats collection
        simsignal_t processingTimeSignal;
        simsignal_t latencySignal;

        //// External parameters
        // timeout for resending message
        simtime_t timeout;
        // message loss chance
        double loss;
    
    public:
        Node();
        virtual ~Node();

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual PingPongMsg* generateNewMessage();
        virtual void sendCopyOf(PingPongMsg *msg);
};

Define_Module(Node);

Node::Node()
{
    // This is nedded, otherwise the destructor will crash if
    // the initialize() isn't called because of runtime error
    processingEvent = nullptr;
    messageBuffer = nullptr;
    timeoutEvent = nullptr;
}

Node::~Node()
{
    // Force delete dynamically allocated objs
    cancelAndDelete(messageBuffer);
    cancelAndDelete(processingEvent);
    cancelAndDelete(timeoutEvent);
}

void Node::initialize()
{
    // Creates the processing time self-message
    processingEvent = new cMessage("processingEvent");
    // No message yet
    messageBuffer = nullptr;

    // Timeout configs and self-message
    timeout = par("timeout");
    timeoutEvent = new cMessage("timeoutEvent");

    // Message loss chance
    loss = par("loss");

    // Stats collection variables
    latencySignal = registerSignal("arrived");
    processingTimeSignal = registerSignal("processed");

    // Schedules the first message sending to t = 5.0s
    // using a scheduled self-message
    if (par("sendMsgOnInit").boolValue())
    {
        EV << "Scheduling the first sending to t = 5.0s\n";
        scheduleAt(5.0, processingEvent);
    }
}

void Node::handleMessage(cMessage *msg)
{
    if (msg == processingEvent)
    {
        // If the arriving message is the processing self-message
        // sends the message stored in the buffer.
        EV << "Internal processing finished. Sending a new message.\n";
        messageBuffer = generateNewMessage();
        sendCopyOf(messageBuffer);
        // Begins the timeout counter
        scheduleAt(simTime() + timeout, timeoutEvent);
    }
    else if (msg == timeoutEvent)
    {
        // If the arriving message is the timeout,
        // resends the message in the buffer.
        EV << "Message timeout reached. Sending again and restarting timer.\n";
        messageBuffer->setSendingTime(simTime());
        sendCopyOf(messageBuffer);
        scheduleAt(simTime() + timeout, timeoutEvent);
    }
    else
    {
        // If the arriving message came from another host
        PingPongMsg *extmsg = check_and_cast<PingPongMsg *>(msg);
        // Cancels the timeout
        cancelEvent(timeoutEvent);
        // If was processing the previous message, abort
        cancelEvent(processingEvent);
        // With a small probability, the message will be lost
        if (uniform(0, 1) < loss) {
            EV << "Losing message\n";
            delete msg;
            return;
        }
        // Gets the transmission delay
        simtime_t latency = simTime() - extmsg->getSendingTime();
        // Stores it and starts the processing timer
        simtime_t delay = par("processingTime");
        EV << "Message arrived, starting " << delay << " secs processing...\n";
        messageBuffer = extmsg;
        messageBuffer->setProcessingTime(delay);
        messageBuffer->setRecvTime(0);
        scheduleAt(simTime() + delay, processingEvent);
        // Emits the signal to store the processing time and latency
        emit(latencySignal, latency);
        emit(processingTimeSignal, delay);
    }
}

PingPongMsg* Node::generateNewMessage()
{
    // Generates a message
    PingPongMsg *msg = new PingPongMsg("message");
    return msg;
}

void Node::sendCopyOf(PingPongMsg *msg)
{
    // Duplicates and sends the copy of a message
    msg->setSendingTime(simTime());
    send(check_and_cast<PingPongMsg *>(msg->dup()), "out");
}
