#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/bridge-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("week7_exer");

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
        if (min > a[i]) {
            min = a[i];
        }
    }
    return min;
}

// Fuction that outputs the index of the minimum value of an input vector a
int minp(double *arr) {
    double min = a[0];
    int minIndex = 0;
    for (int i = 0; i < 100; i++) {
        if (min > a[i]) {
            min = a[i];
            minIndex = i;
        }
    }
    return minIndex;
}

double timeInterval1;
double timeInterval2;

int count1 = 0;
int count2 = 0;

double time1[100];
double time2[100];

uint32_t pkt_size1[100];
uint32_t pkt_size2[100];

// Trace sink that prints moving average throughput of Flow 1
static void UdpThroughput1(Ptr<const Packet> p, const Address &ad) {
    pkt_size1[count1] = p->GetSize();

    if (count1 < 100) {
        time1[count1] = Simulator::Now().GetSeconds();
        count1++;
    }
    if (count1 >= 100) {
        timeInterval1 = maximum(time1) - minimum(time1);
        time1[minp(time1)] = Simulator::Now().GetSeconds();
    }
    uint32_t total_pkt_size = 0;
    for(int i = 0; i < 100; i++) {
        total_pkt_size += pkt_size1[i];
    }

    double throughput = (double)total_pkt_size / timeInterval1;

    NS_LOG_UNCOND("1\t" << Simulator::Now().GetSeconds() << "\t" << throughput);
}

static void UdpThroughput2(Ptr<const Packet> p, const Address &ad) {
    //===========================================================================================
    /* ToDo: Create a trace sink that prints moving average throughput with window size 100
             When the number of received packets is equal to or larger than 100, it can be calculated correctly
             When 100th packet is received, then throughput of the time t = NOW is
             (total size of 1~100 packet) / [(time when 100th packet arrived or NOW) - (time when 1st packet arrived)]
             When 101th packet is received, then throughput of the time t = NOW is
             (total size of 2~101 packet) / [(time when 101th packet arrived or NOW) - (time when 2nd packet arrived)]
             ...
             You can get the packet size by p->GetSize() (in bytes) whose return type is uint32_t
       Hint: Refer to the above trace sink for Flow 1 */

    //==========================================================================================
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
    Ipv4InterfaceContainer Interfaces;
    Interfaces = address.Assign(terminalDevices);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Implement Onoff application (Flow 1)
    OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address("10.1.1.2"), port)));

    //=========================================================================================
    /* ToDo: Configure OnOff Application configuration for Flow1,  i.e., Datarate= 5 Mbps and always on  */
    onoff.(...);
    onoff.(...);
    onoff.(...);

    ApplicationContainer app1 (...);
    app1.(...);
    app1.(...);
    //==========================================================================================

    // Implement packet sink application for Flow 1
    PacketSinkHelper sink("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    ApplicationContainer sinkApp1 = sink.Install(terminals.Get(1));
    sinkApp1.Start(Seconds(0.0));

    //=========================================================================================
    /* ToDo: Conncet trace source "Rx" to the trace sink for Flow 1 */
    sinkApp1.Get(0)-> (...);
    //=========================================================================================

    // Create a similar flow (Flow 2)
    onoff.SetAttribute("Remote", AddressValue(InetSocketAddress(Ipv4Address("10.1.1.1"), port)));
    onoff.SetAttribute("DataRate", StringValue("10Mb/s"));
    ApplicationContainer app2 = onoff.Install(terminals.Get(3));
    app2.Start(Seconds(3.0));
    app2.Stop(Seconds(13.0));

    // Implement packet sink application for Flow 2
    ApplicationContainer sinkApp2 = sink.Install(terminals.Get(0));
    sinkApp2.Start(Seconds(0.0));

    //=========================================================================================
    /* ToDo: Conncet trace source "Rx" to the trace sink for Flow 2 */
    sinkApp2.Get(0)-> (...);
    //=========================================================================================


    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
