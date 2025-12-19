#ifndef NS3_CYBERMOD_H
#define NS3_CYBERMOD_H

/* ============================================================
 *  NS-3 CSMA CYBER ATTACK MODULE
 * ------------------------------------------------------------
 *  Description:
 *   Custom ns-3 module for simulating cyber attacks in a
 *   CSMA-based network environment.
 *
 *  Supported Attacks:
 *   - UDP Flood
 *   - TCP Flood
 *   - ICMP-like Flood (via UDP)
 *   - IP Spoofing Attack
 *   - Replay Attack
 *
 *  Features:
 *   - Packet TX/RX tracing
 *   - Per-victim packet counting
 *   - Raw socket based spoofing
 *   - Packet capture and replay
 *
 *  Educational & research use only.
 * ============================================================
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

#include <vector>
#include <string>

namespace ns3 {

/* ================= GLOBAL STATISTICS ================= */

/*
 * Packet counters per victim node
 */
extern uint64_t totalPacketsReceived[100];

/*
 * Packet storage for replay attacks
 */
extern std::vector<Ptr<Packet>> capturedPackets;

/* ================= SIMULATION PARAMETERS ================= */

struct SimParams
{
    uint32_t numAttackers;    // Number of attacker nodes
    uint32_t numVictims;      // Number of victim nodes
    uint16_t attackPort;      // Target port
    std::string attackRate;   // Flood rate (e.g. "20Mbps")
    std::string attackType;   // udp-flood, tcp-flood, icmp-flood,
                              // ip-spoofing, replay

    std::vector<Ipv4InterfaceContainer> iface;

    NodeContainer attackers;
    NodeContainer victims;
};

/* ================= CALLBACK DECLARATIONS ================= */

void PacketReceivedCallback(uint32_t victimId,
                            Ptr<const Packet> packet,
                            const Address &from);

void TxLog(uint32_t attackerId,
           Ptr<const Packet> packet);

void RxLog(uint32_t victimId,
           Ptr<const Packet> packet,
           const Address &from);

/* ================= ATTACK SETUP FUNCTIONS ================= */

/* Flood attacks (UDP / TCP / ICMP-like) */
void SetupOnOffFlood(const SimParams& params,
                     const std::string& socketFactory);

/* IP Spoofing attack */
void SetupIpSpoofingAttack(const SimParams& params);

/* Replay attack */
void SetupReplayAttack(const SimParams& params);

/* Attack selector */
void SetupAttack(const SimParams& params);

/* ================= LOGGING FUNCTIONS ================= */

void SaveLogs(const SimParams& params);

} // namespace ns3

#endif // NS3_CYBERMOD_H
a
