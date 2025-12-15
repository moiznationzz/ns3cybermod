#include "ns3/ns3cybermod.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CyberAttackCSMA");

// --- Logging: Attacker packet sent ---
void TxTrace(uint32_t attackerId, Ptr<const Packet> pkt) {
    NS_LOG_UNCOND("[TX] Attacker " << attackerId
                   << " sent packet | Size=" << pkt->GetSize()
                   << " bytes | Time=" << Simulator::Now().GetSeconds());
}

// --- Logging: Victim packet received ---
void RxTrace(uint32_t victimId, Ptr<const Packet> pkt, const Address &from) {
    NS_LOG_UNCOND("[RX] Victim " << victimId
                   << " received packet | Size=" << pkt->GetSize()
                   << " bytes | Time=" << Simulator::Now().GetSeconds());
}

int main(int argc, char *argv[])
{
    // ==================== SIMULATION PARAMETERS ====================
    SimParams params;

    params.numAttackers = 5;
    params.numVictims   = 5;
    params.attackPort   = 8080;
    params.attackRate   = "10Mbps";
    params.saveMode     = "file";

    double simDuration  = 15.0;
    std::string csmaRate = "100Mbps";

    // Create nodes
    params.attackers.Create(params.numAttackers);
    params.victims.Create(params.numVictims);

    NodeContainer allNodes;
    allNodes.Add(params.attackers);
    allNodes.Add(params.victims);

    // ==================== CSMA LAN SETUP ====================
    InternetStackHelper stack;
    stack.Install(allNodes);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue(csmaRate));
    csma.SetChannelAttribute("Delay", StringValue("6560ns"));

    NetDeviceContainer devices = csma.Install(allNodes);

    Ipv4AddressHelper ip;
    ip.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ifaces = ip.Assign(devices);
    params.iface.push_back(ifaces);

    // ==================== NORMAL TRAFFIC ====================
    // Each victim runs a BulkSend TCP application to simulate normal service
    for (uint32_t v = 0; v < params.numVictims; v++) {
        BulkSendHelper normalApp("ns3::TcpSocketFactory",
                                 InetSocketAddress(ifaces.GetAddress(0), 9000 + v));
        normalApp.SetAttribute("MaxBytes", UintegerValue(0));
        auto app = normalApp.Install(params.victims.Get(v));
        app.Start(Seconds(0.5));
        app.Stop(Seconds(simDuration));
    }

    // ==================== ATTACK SELECTION ====================
    // You can change this string to test different attacks:
    // "udp-flood", "tcp-syn-flood", "icmp-flood", "queue-saturation", "low-rate-dos", "bandwidth-starvation"
    std::vector<std::string> attacks = {
        "udp-flood",
        "tcp-syn-flood",
        "icmp-flood",
        "queue-saturation",
        "low-rate-dos",
        "bandwidth-starvation"
    };

    // Run all attacks sequentially with 1-second spacing
    double attackStart = 1.0;
    for (auto &atype : attacks) {
        Simulator::Schedule(Seconds(attackStart), [&params, atype]() {
            NS_LOG_UNCOND("=== Starting Attack: " << atype << " at time " 
                           << Simulator::Now().GetSeconds() << "s ===");
            params.attackType = atype;
            SetupAttack(params);
        });
        attackStart += 1.0; // next attack starts 1 sec later
    }

    // ==================== TRACE LOGGING ====================
    for (uint32_t i = 0; i < params.numAttackers; i++)
        params.attackers.Get(i)->TraceConnectWithoutContext("Tx",
                MakeBoundCallback(&TxTrace, i));

    for (uint32_t i = 0; i < params.numVictims; i++)
        params.victims.Get(i)->TraceConnectWithoutContext("Rx",
                MakeBoundCallback(&RxTrace, i));

    // ==================== RUN SIMULATION ====================
    Simulator::Stop(Seconds(simDuration));
    Simulator::Run();
    Simulator::Destroy();

    // ==================== SAVE LOGS ====================
    SaveLogs(params);
    NS_LOG_UNCOND("=== CSMA Cyber-Attacks Simulation Finished ===");
    return 0;
}
