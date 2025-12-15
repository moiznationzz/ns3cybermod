#include "ns3/ns3cybermod.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink.h"
#include <fstream>

namespace ns3 {

/* ============================================================
 *  GLOBAL STATS DEFINITION
 * ============================================================
 */
uint64_t totalPacketsReceived[100] = {0};

/* ============================================================
 *  RX CALLBACK
 * ============================================================
 */
void PacketReceivedCallback(uint32_t victimIndex,
                            Ptr<const Packet> packet,
                            const Address &addr)
{
    // Count packets received by each victim
    if (victimIndex < 100)
        totalPacketsReceived[victimIndex]++;
}

/* ============================================================
 *  INSTALL PACKET SINKS (ONCE)
 * ============================================================
 * PacketSink listens on victim nodes and avoids socket conflicts
 */
static void InstallPacketSinks(const SimParams& params,
                               const std::string& socketType)
{
    uint32_t offset = params.numAttackers;

    for (uint32_t v = 0; v < params.numVictims; v++)
    {
        PacketSinkHelper sink(
            socketType,
            InetSocketAddress(params.iface[0].GetAddress(offset + v),
                              params.attackPort));

        auto apps = sink.Install(params.victims.Get(v));
        apps.Start(Seconds(0.0));
        apps.Stop(Seconds(20.0));

        Ptr<PacketSink> ps = DynamicCast<PacketSink>(apps.Get(0));
        ps->TraceConnectWithoutContext(
            "Rx", MakeBoundCallback(&PacketReceivedCallback, v));
    }
}

/* ============================================================
 *  UDP FLOOD ATTACK
 * ============================================================
 */
void SetupUdpFlood(const SimParams& params)
{
    InstallPacketSinks(params, "ns3::UdpSocketFactory");

    for (uint32_t a = 0; a < params.numAttackers; a++)
    {
        OnOffHelper onoff(
            "ns3::UdpSocketFactory",
            InetSocketAddress(
                params.iface[0].GetAddress(params.numAttackers),
                params.attackPort));

        onoff.SetConstantRate(DataRate(params.attackRate));
        onoff.SetAttribute("PacketSize", UintegerValue(1024));

        auto app = onoff.Install(params.attackers.Get(a));
        app.Start(Seconds(1.0));
        app.Stop(Seconds(10.0));
    }
}

/* ============================================================
 *  TCP FLOOD ATTACK
 * ============================================================
 */
void SetupTcpFlood(const SimParams& params)
{
    InstallPacketSinks(params, "ns3::TcpSocketFactory");

    for (uint32_t a = 0; a < params.numAttackers; a++)
    {
        OnOffHelper onoff(
            "ns3::TcpSocketFactory",
            InetSocketAddress(
                params.iface[0].GetAddress(params.numAttackers),
                params.attackPort));

        onoff.SetConstantRate(DataRate(params.attackRate));
        onoff.SetAttribute("PacketSize", UintegerValue(512));

        auto app = onoff.Install(params.attackers.Get(a));
        app.Start(Seconds(1.0));
        app.Stop(Seconds(10.0));
    }
}

/* ============================================================
 *  ICMP FLOOD (UDP-BASED SIMULATION)
 * ============================================================
 */
void SetupIcmpFlood(const SimParams& params)
{
    // ICMP flood simulated as high-rate UDP traffic
    SetupUdpFlood(params);
}

/* ============================================================
 *  ATTACK SELECTOR
 * ============================================================
 */
void SetupAttack(const SimParams& params)
{
    if (params.attackType == "udp-flood")
        SetupUdpFlood(params);
    else if (params.attackType == "tcp-flood")
        SetupTcpFlood(params);
    else if (params.attackType == "icmp-flood")
        SetupIcmpFlood(params);
    else
        NS_ABORT_MSG("Unsupported CSMA attack type");
}

/* ============================================================
 *  LOGGING FUNCTION
 * ============================================================
 */
void SaveLogs(const SimParams& params)
{
    std::ofstream file("ddos_log.txt");

    file << "===== CSMA CYBER ATTACK LOG =====\n";
    file << "Attack Type: " << params.attackType << "\n\n";

    for (uint32_t i = 0; i < params.numVictims; i++)
        file << "Victim " << i
             << " received packets: "
             << totalPacketsReceived[i] << "\n";

    file.close();
}

} // namespace ns3
