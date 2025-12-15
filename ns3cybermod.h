#ifndef NS3_CYBERMOD_H
#define NS3_CYBERMOD_H

/* ============================================================
 *  NS-3 CYBER ATTACK MODULE (CSMA SAFE)
 * ------------------------------------------------------------
 *  Supported Attacks:
 *   - UDP Flood
 *   - TCP Flood
 *   - ICMP Flood (UDP-based volumetric)
 *
 *  This module is designed to be:
 *   - Stable
 *   - Error-free
 *   - Compatible with CSMA networks
 * ============================================================
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

#include <vector>
#include <string>

namespace ns3 {

/* ============================================================
 *  GLOBAL STATISTICS
 * ============================================================
 */

/**
 * Stores the number of packets received by each victim.
 * Updated via Rx callback during simulation.
 */
extern uint64_t totalPacketsReceived[100];

/* ============================================================
 *  SIMULATION PARAMETERS STRUCT
 * ============================================================
 */
struct SimParams
{
    uint32_t numAttackers;     // Number of attacker nodes
    uint32_t numVictims;       // Number of victim nodes
    uint16_t attackPort;       // Target port
    std::string attackRate;    // Attack rate (e.g. 10Mbps)
    std::string attackType;    // udp-flood | tcp-flood | icmp-flood

    // Network objects
    std::vector<Ipv4InterfaceContainer> iface;
    NodeContainer attackers;
    NodeContainer victims;
};

/* ============================================================
 *  CALLBACK FUNCTION
 * ============================================================
 */

/**
 * Called whenever a victim receives a packet.
 */
void PacketReceivedCallback(uint32_t victimIndex,
                            Ptr<const Packet> packet,
                            const Address &addr);

/* ============================================================
 *  ATTACK FUNCTIONS
 * ============================================================
 */

void SetupUdpFlood(const SimParams& params);
void SetupTcpFlood(const SimParams& params);
void SetupIcmpFlood(const SimParams& params);

/* ============================================================
 *  ATTACK SELECTOR
 * ============================================================
 */
void SetupAttack(const SimParams& params);

/* ============================================================
 *  LOGGING
 * ============================================================
 */
void SaveLogs(const SimParams& params);

} // namespace ns3
#endif
