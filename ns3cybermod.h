#ifndef NS3_CYBERMOD_H
#define NS3_CYBERMOD_H

/* ==============================================================
 *  NS-3 CYBER ATTACK MODULE (CSMA ONLY)
 * --------------------------------------------------------------
 *  Supported attacks:
 *    - udp-flood
 *    - tcp-syn-flood
 *    - icmp-flood
 *    - queue-saturation
 *    - low-rate-dos
 *    - bandwidth-starvation
 * ==============================================================
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

#include <vector>
#include <string>

namespace ns3 {

/* ================= GLOBAL STATS ================= */

extern uint64_t totalPacketsReceived[100];

/* ================= SIMULATION PARAMS ================= */

struct SimParams
{
    uint32_t numAttackers;
    uint32_t numVictims;
    uint16_t attackPort;
    std::string attackRate;
    std::string attackType;
    std::string saveMode;

    std::vector<Ipv4InterfaceContainer> iface;
    NodeContainer attackers;
    NodeContainer victims;
};

/* ================= CALLBACK ================= */

void PacketReceivedCallback(uint32_t victimIndex,
                            Ptr<const Packet> packet,
                            const Address &addr);

/* ================= ATTACK FUNCTIONS ================= */

// Generic UDP / TCP flood
void SetupOnOffFlood(const SimParams& params,
                     const std::string& socketFactory);

// ICMP flood (volumetric)
void SetupIcmpFlood(const SimParams& params);

// TCP SYN flood (short TCP bursts)
void SetupTcpSynFlood(const SimParams& params);

// Queue saturation (many small packets)
void SetupQueueSaturation(const SimParams& params);

// Low-rate DoS (Shrew attack)
void SetupLowRateDos(const SimParams& params);

// Bandwidth starvation attack
void SetupBandwidthStarvation(const SimParams& params);

/* ================= ATTACK SELECTOR ================= */

void SetupAttack(const SimParams& params);

/* ================= LOGGING ================= */

void SaveLogs(const SimParams& params);

} // namespace ns3

#endif // NS3_CYBERMOD_H
