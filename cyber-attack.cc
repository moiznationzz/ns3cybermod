#include "ns3/ns3cybermod.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/core-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CyberAttackCSMA");

int main(int argc, char *argv[])
{
    SimParams params;

    /* ================= CONFIGURATION ================= */
    params.numAttackers = 5;
    params.numVictims   = 5;
    params.attackPort   = 8080;
    params.attackRate   = "10Mbps";       // slower flood for readable logs
    params.attackType   = "udp-flood";   // udp-flood, tcp-flood, icmp-flood, ip-spoofing, replay
    params.attackMode   = "dsos";        // "dos" or "ddos"

    double simTime       = 12.0;
    std::string csmaRate = "100Mbps";

    NS_LOG_UNCOND("=== CSMA CYBER ATTACK SIMULATION STARTED ===");

    /* ================= NODES ================= */
    params.attackers.Create(params.numAttackers);
    params.victims.Create(params.numVictims);

    NodeContainer allNodes;
    allNodes.Add(params.attackers);
    allNodes.Add(params.victims);

    /* ================= INTERNET STACK ================= */
    InternetStackHelper internet;
    internet.Install(allNodes);

    /* ================= CSMA NETWORK ================= */
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue(csmaRate));
    csma.SetChannelAttribute("Delay", StringValue("6560ns"));

    NetDeviceContainer devices = csma.Install(allNodes);

    /* ================= IP ADDRESSES ================= */
    Ipv4AddressHelper ip;
    ip.SetBase("10.1.1.0", "255.255.255.0");
    params.iface.push_back(ip.Assign(devices));

    /* ================= ATTACK SETUP ================= */
    SetupAttack(params);

    /* ================= RUN SIMULATION ================= */
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();

    /* ================= SAVE LOGS ================= */
    SaveLogs(params);

    NS_LOG_UNCOND("=== SIMULATION FINISHED ===");
    return 0;
}
