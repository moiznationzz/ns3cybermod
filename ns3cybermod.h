#ifndef NS3_CYBERMOD_H
#define NS3_CYBERMOD_H

/* ============================================================
 *  NS-3 CSMA CYBER ATTACK MODULE
 * ------------------------------------------------------------
 *  Supported attacks:
 *   - UDP Flood
 *   - TCP Flood
 *   - ICMP Flood (UDP-based volumetric)
 *
 *  Features:
 *   - Real-time TX/RX logging
 *   - Packet counting per victim
 *   - CSMA compatible
 * ============================================================
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

#include <vector>
#include <string>

namespace ns3 {

/* ================= GLOBAL STATS ================= */

// Packet counter for each victim
extern uint64_t totalPacketsReceived[100];

/* ================= SIMULATION PARAMETERS ================= */

struct SimParams
{
    uint32_t numAttackers;
    uint32_t numVictims;
    uint16_t attackPort;
    std::string attackRate;
    std::string attackType;

    std::vector<Ipv4InterfaceContainer> iface;
    NodeContainer attackers;
    NodeContainer victims;
};

/* ================= CALLBACKS ================= */

// Count packets at victim
void PacketReceivedCallback(uint32_t victimId,
                            Ptr<const Packet> packet,
                            const Address &from);

// Real-time logging
void TxLog(uint32_t attackerId, Ptr<const Packet> packet);
void RxLog(uint32_t victimId, Ptr<const Packet> packet,
           const Address &from);

/* ================= ATTACK FUNCTIONS ================= */

void SetupOnOffFlood(const SimParams& params,
                     const std::string& socketFactory);

void SetupAttack(const SimParams& params);

/* ================= LOGGING ================= */

void SaveLogs(const SimParams& params);

} // namespace ns3

#endif // NS3_CYBERMOD_H
