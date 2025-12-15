#include "../ns3cybermod.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink.h"
#include <fstream>

namespace ns3 {

/* ================= GLOBAL STATS ================= */

uint64_t totalPacketsReceived[100] = {0};

/* ================= RX CALLBACK ================= */

void PacketReceivedCallback(uint32_t victimIndex,
                            Ptr<const Packet> packet,
                            const Address &addr)
{
    if (victimIndex < 100)
        totalPacketsReceived[victimIndex]++;
}

/* ================= COMMON FLOOD BASE ================= */

void SetupOnOffFlood(const SimParams& params,
                     const std::string& socketFactory,
                     uint32_t pktSize,
                     std::string rate,
                     double onTime,
                     double offTime)
{
    uint32_t A = params.numAttackers;
    uint32_t V = params.numVictims;

    // ---- Victims ----
    for (uint32_t v = 0; v < V; v++)
    {
        PacketSinkHelper sink(socketFactory,
            InetSocketAddress(params.iface[0].GetAddress(A + v),
                              params.attackPort));

        auto app = sink.Install(params.victims.Get(v));
        app.Start(Seconds(0.0));
        app.Stop(Seconds(15.0));

        auto sinkPtr = DynamicCast<PacketSink>(app.Get(0));
        sinkPtr->TraceConnectWithoutContext(
            "Rx", MakeBoundCallback(&PacketReceivedCallback, v));
    }

    // ---- Attackers ----
    for (uint32_t a = 0; a < A; a++)
    {
        for (uint32_t v = 0; v < V; v++)
        {
            OnOffHelper onoff(socketFactory,
                InetSocketAddress(
                    params.iface[0].GetAddress(A + v),
                    params.attackPort));

            onoff.SetAttribute("PacketSize", UintegerValue(pktSize));
            onoff.SetAttribute("DataRate", DataRateValue(DataRate(rate)));
            onoff.SetAttribute("OnTime",
                StringValue("ns3::ConstantRandomVariable[Constant=" + std::to_string(onTime) + "]"));
            onoff.SetAttribute("OffTime",
                StringValue("ns3::ConstantRandomVariable[Constant=" + std::to_string(offTime) + "]"));

            auto app = onoff.Install(params.attackers.Get(a));
            app.Start(Seconds(1.0));
            app.Stop(Seconds(14.0));
        }
    }
}

/* ================= ATTACK IMPLEMENTATIONS ================= */

void SetupIcmpFlood(const SimParams& params)
{
    SetupOnOffFlood(params, "ns3::UdpSocketFactory",
                    1024, params.attackRate, 1.0, 0.0);
}

void SetupTcpSynFlood(const SimParams& params)
{
    SetupOnOffFlood(params, "ns3::TcpSocketFactory",
                    64, "5Mbps", 0.2, 0.1);
}

void SetupQueueSaturation(const SimParams& params)
{
    SetupOnOffFlood(params, "ns3::UdpSocketFactory",
                    64, "50Mbps", 1.0, 0.0);
}

void SetupLowRateDos(const SimParams& params)
{
    SetupOnOffFlood(params, "ns3::UdpSocketFactory",
                    1024, "20Mbps", 0.05, 0.95);
}

void SetupBandwidthStarvation(const SimParams& params)
{
    SetupOnOffFlood(params, "ns3::UdpSocketFactory",
                    1500, "80Mbps", 1.0, 0.0);
}

/* ================= ATTACK SELECTOR ================= */

void SetupAttack(const SimParams& params)
{
    if (params.attackType == "udp-flood")
        SetupOnOffFlood(params, "ns3::UdpSocketFactory",
                        1024, params.attackRate, 1.0, 0.0);

    else if (params.attackType == "tcp-syn-flood")
        SetupTcpSynFlood(params);

    else if (params.attackType == "icmp-flood")
        SetupIcmpFlood(params);

    else if (params.attackType == "queue-saturation")
        SetupQueueSaturation(params);

    else if (params.attackType == "low-rate-dos")
        SetupLowRateDos(params);

    else if (params.attackType == "bandwidth-starvation")
        SetupBandwidthStarvation(params);

    else
        NS_ABORT_MSG("Unsupported CSMA attack type");
}

/* ================= LOGGING ================= */

void SaveLogs(const SimParams& params)
{
    std::ofstream file("ddos_log.txt");

    file << "===== NS-3 CSMA Cyber Attack Log =====\n";
    file << "Attack Type : " << params.attackType << "\n";
    file << "Attackers   : " << params.numAttackers << "\n";
    file << "Victims     : " << params.numVictims << "\n";
    file << "Rate        : " << params.attackRate << "\n\n";

    for (uint32_t i = 0; i < params.numVictims; i++)
        file << "Victim " << i
             << " received packets: "
             << totalPacketsReceived[i] << "\n";

    file.close();
}

} // namespace ns3
