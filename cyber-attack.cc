#include "ns3/ns3cybermod.h"      // <-- OUR CUSTOM MODULE (.h + .cc)
#include "ns3/csma-module.h"     // CSMA channel and devices
#include "ns3/internet-module.h" // TCP/IP stack
#include "ns3/core-module.h"     // Core ns-3 functionality

using namespace ns3;

/*
 * Logging component for this scenario file.
 * Used by NS_LOG_UNCOND statements in this program.
 */
NS_LOG_COMPONENT_DEFINE("CyberAttackCSMA");

int main(int argc, char *argv[])
{
    /* =====================================================
     * SimParams STRUCT (DEFINED IN ns3cybermod.h)
     * -----------------------------------------------------
     * This structure is shared between:
     *   - cyber-attack-csma.cc (this file)
     *   - ns3cybermod.cc       (attack implementation)
     *
     * It carries all configuration parameters
     * required by the attack setup functions.
     * =====================================================
     */
    SimParams params;

    /* ================= ATTACK CONFIGURATION ================= */

    /*
     * These parameters are READ by:
     *   SetupAttack(params)
     *
     * SetupAttack() is IMPLEMENTED in ns3cybermod.cc
     * and DECLARED in ns3cybermod.h
     *
     * Supported attack types (handled in ns3cybermod.cc):
     *   - "udp-flood"
     *   - "tcp-flood"
     *   - "icmp-flood"
     *   - "ip-spoofing"
     *   - "replay"
     */
    params.numAttackers = 3;          // Used in SetupOnOffFlood()
    params.numVictims   = 2;          // Used in PacketSink setup
    params.attackPort   = 8080;       // Victim listening port
    params.attackRate   = "10Mbps";   // Flood rate (OnOffApplication)
    params.attackType   = "udp-flood";// Selected attack (SetupAttack)

    double simTime       = 12.0;
    std::string csmaRate = "100Mbps";

    NS_LOG_UNCOND("=== CSMA CYBER ATTACK SIMULATION STARTED ===");

    /* ================= NODE CREATION ================= */

    /*
     * NodeContainers declared inside SimParams (ns3cybermod.h)
     * These are later accessed by attack functions
     * in ns3cybermod.cc
     */
    params.attackers.Create(params.numAttackers);
    params.victims.Create(params.numVictims);

    NodeContainer allNodes;
    allNodes.Add(params.attackers);
    allNodes.Add(params.victims);

    /* ================= INTERNET STACK ================= */

    /*
     * Required for TCP, UDP, ICMP, and raw sockets.
     * Attack traffic generated in ns3cybermod.cc
     * depends on this stack.
     */
    InternetStackHelper internet;
    internet.Install(allNodes);

    /* ================= CSMA NETWORK ================= */

    /*
     * CSMA channel used by attackers and victims.
     * All packets generated in ns3cybermod.cc
     * flow through this CSMA channel.
     */
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue(csmaRate));
    csma.SetChannelAttribute("Delay", StringValue("6560ns"));

    NetDeviceContainer devices = csma.Install(allNodes);

    /* ================= IP ADDRESS ASSIGNMENT ================= */

    /*
     * IP interfaces are STORED in:
     *   params.iface (vector)
     *
     * This vector is later USED in ns3cybermod.cc to:
     *   - Get victim IP addresses
     *   - Configure attack destinations
     */
    Ipv4AddressHelper ip;
    ip.SetBase("10.1.1.0", "255.255.255.0");
    params.iface.push_back(ip.Assign(devices));

    /* ================= ATTACK SETUP ================= */

    /*
     * SetupAttack(params)
     * -------------------------------------------------
     * DECLARED in: ns3cybermod.h
     * IMPLEMENTED in: ns3cybermod.cc
     *
     * Internally, it:
     *   - Selects attack type
     *   - Calls SetupOnOffFlood(), SetupIpSpoofingAttack(),
     *     or SetupReplayAttack()
     *   - Installs PacketSink on victims
     *   - Installs attack applications on attackers
     *   - Attaches TX/RX callbacks:
     *       • TxLog()
     *       • RxLog()
     *       • PacketReceivedCallback()
     */
    SetupAttack(params);

    /* ================= SIMULATION EXECUTION ================= */

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();

    /* ================= SAVE RESULTS ================= */

    /*
     * SaveLogs(params)
     * -------------------------------------------------
     * DECLARED in: ns3cybermod.h
     * IMPLEMENTED in: ns3cybermod.cc
     *
     * Writes:
     *   - Attack type
     *   - Number of attackers & victims
     *   - Total packets received per victim
     *
     * Uses global array:
     *   totalPacketsReceived[]
     */
    SaveLogs(params);

    NS_LOG_UNCOND("=== SIMULATION FINISHED ===");
    return 0;
}
