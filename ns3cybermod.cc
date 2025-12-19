#include "ns3/ns3cybermod.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/ipv4-raw-socket-factory.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/tcp-header.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include <fstream>

namespace ns3 {

/* ================= GLOBAL ================= */

uint64_t totalPacketsReceived[100] = {0};
std::vector<Ptr<Packet>> capturedPackets;

/* ================= CALLBACKS ================= */

void PacketReceivedCallback(uint32_t victimId,
                            Ptr<const Packet> packet,
                            const Address &from)
{
    totalPacketsReceived[victimId]++;
}

void TxLog(uint32_t attackerId, Ptr<const Packet> packet)
{
    NS_LOG_UNCOND("[TX] Attacker " << attackerId
        << " sent packet at "
        << Simulator::Now().GetSeconds() << "s");
}

void RxLog(uint32_t victimId,
           Ptr<const Packet> packet,
           const Address &from)
{
    NS_LOG_UNCOND("[RX] Victim " << victimId
        << " received packet at "
        << Simulator::Now().GetSeconds() << "s");
}

/* ================= FLOOD ATTACK ================= */

void SetupOnOffFlood(const SimParams& params,
                     const std::string& socketFactory)
{
    uint32_t A = params.numAttackers;
    uint32_t V = params.numVictims;

    for (uint32_t v = 0; v < V; v++)
    {
        PacketSinkHelper sink(
            socketFactory,
            InetSocketAddress(
                params.iface[0].GetAddress(A + v),
                params.attackPort));

        ApplicationContainer sinkApp =
            sink.Install(params.victims.Get(v));

        sinkApp.Start(Seconds(0.0));
        sinkApp.Stop(Seconds(12.0));

        Ptr<PacketSink> sinkPtr =
            DynamicCast<PacketSink>(sinkApp.Get(0));

        sinkPtr->TraceConnectWithoutContext(
            "Rx", MakeBoundCallback(&RxLog, v));

        sinkPtr->TraceConnectWithoutContext(
            "Rx", MakeBoundCallback(&PacketReceivedCallback, v));
    }

    for (uint32_t a = 0; a < A; a++)
    {
        for (uint32_t v = 0; v < V; v++)
        {
            OnOffHelper onoff(
                socketFactory,
                InetSocketAddress(
                    params.iface[0].GetAddress(A + v),
                    params.attackPort));

            onoff.SetConstantRate(
                DataRate(params.attackRate));

            onoff.SetAttribute(
                "PacketSize", UintegerValue(1024));

            ApplicationContainer app =
                onoff.Install(params.attackers.Get(a));

            app.Start(Seconds(1.0));
            app.Stop(Seconds(11.0));

            Ptr<OnOffApplication> onoffApp =
                DynamicCast<OnOffApplication>(app.Get(0));

            onoffApp->TraceConnectWithoutContext(
                "Tx", MakeBoundCallback(&TxLog, a));
        }
    }
}

/* ================= IP SPOOFING ATTACK ================= */

void SetupIpSpoofingAttack(const SimParams& params)
{
    uint32_t A = params.numAttackers;
    uint32_t V = params.numVictims;

    for (uint32_t a = 0; a < A; a++)
    {
        Ptr<Socket> rawSocket =
            Socket::CreateSocket(
                params.attackers.Get(a),
                Ipv4RawSocketFactory::GetTypeId());

        rawSocket->SetAttribute(
            "Protocol", UintegerValue(17)); // UDP

        for (uint32_t v = 0; v < V; v++)
        {
            Simulator::Schedule(
                Seconds(1.0 + a),
                [=]() {
                    Ptr<Packet> packet =
                        Create<Packet>(512);

                    Ipv4Header ip;
                    ip.SetSource(
                        Ipv4Address("10.10.10.10")); // spoofed
                    ip.SetDestination(
                        params.iface[0].GetAddress(A + v));
                    ip.SetProtocol(17);
                    ip.SetPayloadSize(packet->GetSize());

                    UdpHeader udp;
                    udp.SetSourcePort(9999);
                    udp.SetDestinationPort(params.attackPort);

                    packet->AddHeader(udp);
                    packet->AddHeader(ip);

                    rawSocket->SendTo(
                        packet,
                        0,
                        InetSocketAddress(
                            params.iface[0].GetAddress(A + v),
                            params.attackPort));
                });
        }
    }
}

/* ================= REPLAY ATTACK ================= */

void CapturePacket(Ptr<const Packet> packet)
{
    capturedPackets.push_back(packet->Copy());
}

void SetupReplayAttack(const SimParams& params)
{
    uint32_t A = params.numAttackers;
    uint32_t V = params.numVictims;

    for (uint32_t v = 0; v < V; v++)
    {
        PacketSinkHelper sink(
            "ns3::UdpSocketFactory",
            InetSocketAddress(
                params.iface[0].GetAddress(A + v),
                params.attackPort));

        ApplicationContainer sinkApp =
            sink.Install(params.victims.Get(v));

        sinkApp.Start(Seconds(0.0));
        sinkApp.Stop(Seconds(12.0));

        Ptr<PacketSink> sinkPtr =
            DynamicCast<PacketSink>(sinkApp.Get(0));

        sinkPtr->TraceConnectWithoutContext(
            "Rx", MakeCallback(&CapturePacket));
    }

    Simulator::Schedule(
        Seconds(6.0),
        [&]() {
            for (uint32_t a = 0; a < A; a++)
            {
                Ptr<Socket> sock =
                    Socket::CreateSocket(
                        params.attackers.Get(a),
                        UdpSocketFactory::GetTypeId());

                for (auto pkt : capturedPackets)
                {
                    sock->SendTo(
                        pkt->Copy(),
                        0,
                        InetSocketAddress(
                            params.iface[0].GetAddress(A),
                            params.attackPort));
                }
            }
        });
}

/* ================= ATTACK SELECTOR ================= */

void SetupAttack(const SimParams& params)
{
    if (params.attackType == "udp-flood")
        SetupOnOffFlood(params, "ns3::UdpSocketFactory");

    else if (params.attackType == "tcp-flood")
        SetupOnOffFlood(params, "ns3::TcpSocketFactory");

    else if (params.attackType == "icmp-flood")
        SetupOnOffFlood(params, "ns3::UdpSocketFactory");

    else if (params.attackType == "ip-spoofing")
        SetupIpSpoofingAttack(params);

    else if (params.attackType == "replay")
        SetupReplayAttack(params);

    else
        NS_ABORT_MSG("Unsupported attack type");
}

/* ================= SAVE LOGS ================= */

void SaveLogs(const SimParams& params)
{
    std::ofstream file("ddos_log.txt");

    file << "===== CSMA CYBER ATTACK LOG =====\n";
    file << "Attack Type: " << params.attackType << "\n";
    file << "Attackers : " << params.numAttackers << "\n";
    file << "Victims   : " << params.numVictims << "\n";
    file << "Rate      : " << params.attackRate << "\n\n";

    for (uint32_t i = 0; i < params.numVictims; i++)
        file << "Victim " << i
             << " packets received: "
             << totalPacketsReceived[i] << "\n";

    file.close();
}

} // namespace ns3
