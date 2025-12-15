#include "ns3/ns3cybermod.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/core-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CyberAttackCSMA");

int main(int argc, char *argv[])
{
    SimParams params;

    /* ================= USER SETTINGS ================= */

    params.numAttackers = 3;
    params.numVictims   = 2;
    params.attackPort   = 8080;
    params.attackRate   = "10Mbps";
    params.attackType   = "udp-flood"; // change here

    double simTime = 12.0;
    std::string csmaRate = "100Mbps";

    NS_LOG_UNCOND("=== CSMA CYBER ATTACK SIMULATION STARTED ===");

    /* ================= NODE CREATION ================= */

    params.attackers.Create(params.numAttackers);
    params.victims.Create(params.numVictims);

    NodeContainer all;
    all.Add(params.attackers);
    all.Add(params.victims);

    /* ================= NETWORK ================= */

    InternetStackHelper internet;
    internet.Install(all);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue(csmaRate));
    csma.SetChannelAttribute("Delay", StringValue("6560ns"));

    NetDeviceContainer devices = csma.Install(all);

    Ipv4AddressHelper ip;
    ip.SetBase("10.1.1.0", "255.255.255.0");
    params.iface.push_back(ip.Assign(devices));

    /* ================= ATTACK ================= */

    SetupAttack(params);

    /* ================= RUN ================= */

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();

    SaveLogs(params);

    NS_LOG_UNCOND("=== SIMULATION FINISHED ===");
    return 0;
}
