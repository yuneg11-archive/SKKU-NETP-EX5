#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/bridge-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Exercise7");

// Function outputs the maximum value of an input vector a
double maximum(double *arr) {
    double max = arr[0];
    for (int i = 0; i < 100; i++) {
        if (max < arr[i]) {
            max = arr[i];
        }
    }
    return max;
}

// Function outputs the minimum value of an input vector a
double minimum(double *arr) {
    double min = arr[0];
    for (int i = 0; i < 100; i++) {
        if (min > arr[i]) {
            min = arr[i];
        }
    }
    return min;
}

// Fuction that outputs the index of the minimum value of an input vector a
int minp(double *arr) {
    double min = arr[0];
    int minIndex = 0;
    for (int i = 0; i < 100; i++) {
        if (min > arr[i]) {
            min = arr[i];
            minIndex = i;
        }
    }
    return minIndex;
}

int countGlobal[2] = { 0 };
double timePointGlobal[2][100] = { 0 };
uint32_t totalPacketSizeGlobal[2] = { 0 };

// Trace sink that prints moving average throughput of Flow 1
static void UdpThroughput(std::string flowIndexString, Ptr<const Packet> packetP, const Address &address) {
    int flowIndex = std::stoi(flowIndexString);

    int &count = countGlobal[flowIndex];
    double *timePoint = timePointGlobal[flowIndex];
    uint32_t &totalPacketSize = totalPacketSizeGlobal[flowIndex];

    if (count < 100) {
        timePoint[count] = Simulator::Now().GetSeconds();
        totalPacketSize += packetP->GetSize();
        count++;
    }

    if (count >= 100) {
        double timeInterval = maximum(timePoint) - minimum(timePoint);
        double throughput = (double)totalPacketSize / timeInterval;

        timePoint[minp(timePoint)] = Simulator::Now().GetSeconds();

        NS_LOG_UNCOND((flowIndex + 1) << "\t" << Simulator::Now().GetSeconds() << "\t" << throughput);
    }
}

int main(int argc, char *argv[]) {
    // Create nodes
    NodeContainer terminals;
    terminals.Create(4);

    NodeContainer csmaSwitch;
    csmaSwitch.Create(1);

    // Create links
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
    csma.SetChannelAttribute("Delay", TimeValue(MicroSeconds(10)));

    NetDeviceContainer terminalDevices;
    NetDeviceContainer switchDevices;

    // Connect switch node and terminal nodes
    for (int i = 0; i < 4; i++) {
        NetDeviceContainer link = csma.Install(NodeContainer(terminals.Get(i), csmaSwitch));
        terminalDevices.Add(link.Get(0));
        switchDevices.Add(link.Get(1));
    }

    Ptr<Node> switchNode = csmaSwitch.Get(0);
    BridgeHelper bridge;
    bridge.Install(switchNode, switchDevices);

    // Install Internet stack and assign IP addresses
    uint16_t port = 9;
    InternetStackHelper terminalStack;
    InternetStackHelper csmaSwitchStack;

    terminalStack.Install(terminals);
    csmaSwitchStack.Install(csmaSwitch);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    address.Assign(terminalDevices);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Implement packet sink application for Flow 1
    PacketSinkHelper sink("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));

    // Implement Onoff application (Flow 1)
    OnOffHelper onoff1("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address("10.1.1.2"), port)));
    onoff1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onoff1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    onoff1.SetAttribute("DataRate", StringValue("5Mbps"));

    ApplicationContainer app1 = onoff1.Install(terminals.Get(0));
    app1.Start(Seconds(1));
    app1.Stop(Seconds(10));

    ApplicationContainer sinkApp1 = sink.Install(terminals.Get(1));
    sinkApp1.Start(Seconds(0));
    sinkApp1.Get(0)->TraceConnect("Rx", "0", MakeCallback(&UdpThroughput));

    // Create a similar flow (Flow 2)
    OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address("10.1.1.1"), port)));
    onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    onoff.SetAttribute("DataRate", StringValue("10Mbps"));

    ApplicationContainer app2 = onoff.Install(terminals.Get(3));
    app2.Start(Seconds(3));
    app2.Stop(Seconds(13));

    ApplicationContainer sinkApp2 = sink.Install(terminals.Get(0));
    sinkApp2.Start(Seconds(0));
    sinkApp2.Get(0)->TraceConnect("Rx", "1", MakeCallback(&UdpThroughput));

    // Run simulator
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
