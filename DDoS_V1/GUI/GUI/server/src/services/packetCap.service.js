// const { addNotificationToAllUsers } = require('../models/Notification.model');

const { unlinkSync } = require("fs");
const { format } = require("date-fns");
const net = require("net");
const config = require("../config");
const { isProduction } = require("../config/index");

const {
  getActiveAnomaliesFromDatabase,
  addNewAnomalyToDatabase,
  updateEndTimeForAnomaliesToDatabase,
} = require("../helper/dbo/netAnomalies.helper.js");

const {
  sendAlertEmailAttack,
  sendWarningEmail,
  sendAlertEmailEnd,
} = require("./email.service");

let anomaliesDataCache = [];

function PacketProcessing(io) {
  // syncConfig();
  const mutex = new Map();
  let attackTimeout = null;

  let status = "Normal";

  const resetStatus = () => {
    status = "Normal";
  };

  const trafficTemplate = () => ({
    total: {
      bypass: {
        bits: 0,
        packets: 0,
      },
      attack: {
        bits: 0,
        packets: 0,
      },
    },
    protocol: {
      icmp: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
      udp: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
      tcp: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
      http: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
      dns: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
      esp: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
      unknown: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
    },
    attack: {
      icmpFlood: {
        bits: 0,
        packets: 0,
      },
      udpFlood: {
        bits: 0,
        packets: 0,
      },
      dnsFlood: {
        bits: 0,
        packets: 0,
      },
      udpFrag: {
        bits: 0,
        packets: 0,
      },
      tcpFrag: {
        bits: 0,
        packets: 0,
      },
      synFlood: {
        bits: 0,
        packets: 0,
      },
      land: {
        bits: 0,
        packets: 0,
      },
      httpFlood: {
        bits: 0,
        packets: 0,
      },
      httpsFlood: {
        bits: 0,
        packets: 0,
      },
      ipsec: {
        bits: 0,
        packets: 0,
      },
      unknown: {
        bits: 0,
        packets: 0,
      },
    },
  });

  const interfaceTemplate = () => ({
    interface: {
      eth1: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
      eth2: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
      eth3: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
      eth4: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
      eth5: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
      eth6: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
      eth7: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
      eth8: {
        bypass: {
          bits: 0,
          packets: 0,
        },
        attack: {
          bits: 0,
          packets: 0,
        },
      },
    },
  });

  const interfaceStatusTemplate = () => ({
    interface: {
      eth1: {
        status: 0,
        lastPacketTime: 0,
      },
      eth2: {
        status: 0,
        lastPacketTime: 0,
      },
      eth3: {
        status: 0,
        lastPacketTime: 0,
      },
      eth4: {
        status: 0,
        lastPacketTime: 0,
      },
      eth5: {
        status: 0,
        lastPacketTime: 0,
      },
      eth6: {
        status: 0,
        lastPacketTime: 0,
      },
      eth7: {
        status: 0,
        lastPacketTime: 0,
      },
      eth8: {
        status: 0,
        lastPacketTime: 0,
      },
    },
  });

  let traffic_Summary = trafficTemplate(); // Persistent accumulated store
  let traffic_oneSecStats = trafficTemplate();

  let interface_oneSecStats = interfaceTemplate();
  let interfaceStatus = interfaceStatusTemplate();

  let packetArray = [];

  // NEW: Handle 'get_accumulated' event to send traffic_Summary to client
  io.on("connection", (socket) => {
    socket.on("get_accumulated", () => {
      console.log("Client requested accumulated data");
      socket.emit("accumulated", traffic_Summary);
    });

    // NEW: Handle 'reset_both' event to reset traffic_Summary
    socket.on("reset_both", () => {
      console.log("Resetting accumulated data");
      const previous = { ...traffic_Summary };
      traffic_Summary = trafficTemplate(); // Reset to initial
      socket.emit("reset_confirm", previous); // Confirm with previous data
    });
  });

  const handlePacket = async (packet) => {
    const attackMap = {
      "ICMP Flood": ["icmpFlood", "icmp"],
      "UDP Fragment": ["udpFrag", "udp"],
      "TCP Fragment": ["tcpFrag", "tcp"],
      "SYN Flood": ["synFlood", "tcp"],
      "UDP Flood": ["udpFlood", "udp"],
      "DNS Flood": ["dnsFlood", "dns"],
      "LAND Attack": ["land", "tcp"],
      "IPSec IKE Flood": ["ipsec", "udp"],
      "HTTP Flood": ["httpFlood", "http"],
      "HTTPS Flood": ["httpsFlood", "http"],
      "DNS AUTH PKT": ["dnsAuthFlood", "udp"],
    };

    let [attackType, protocolType] = attackMap[packet.attackType] || [
      "unknown",
      packet.protocol.toLowerCase(),
    ];

    if (
      (packet.srcPort === 80 ||
        packet.dstPort === 80 ||
        packet.srcPort === 443 ||
        packet.dstPort === 443) &&
      packet.protocol === "TCP"
    ) {
      protocolType = "http";
    }

    if (
      (packet.srcPort === 50 || packet.dstPort === 50) &&
      packet.protocol === "UDP"
    ) {
      protocolType = "esp";
    }

    if (packet.attackType === "Normal") {
      traffic_Summary.total.bypass.packets += packet.packetCount;
      traffic_Summary.total.bypass.bits += packet.packetSize;

      traffic_Summary.protocol[protocolType].bypass.packets +=
        packet.packetCount;
      traffic_Summary.protocol[protocolType].bypass.bits += packet.packetSize;

      traffic_oneSecStats.total.bypass.packets += packet.packetCount;
      traffic_oneSecStats.total.bypass.bits += packet.packetSize;
      traffic_oneSecStats.protocol[protocolType].bypass.packets +=
        packet.packetCount;
      traffic_oneSecStats.protocol[protocolType].bypass.bits +=
        packet.packetSize;

      if (interface_oneSecStats.interface[`eth${packet.interface}`]) {
        interface_oneSecStats.interface[
          `eth${packet.interface}`
        ].bypass.packets += packet.packetCount;
        interface_oneSecStats.interface[`eth${packet.interface}`].bypass.bits +=
          packet.packetSize;
      }
    } else {
      traffic_Summary.total.attack.packets += packet.packetCount;
      traffic_Summary.total.attack.bits += packet.packetSize;
      traffic_Summary.protocol[protocolType].attack.packets +=
        packet.packetCount;
      traffic_Summary.protocol[protocolType].attack.bits += packet.packetSize;
      traffic_Summary.attack[attackType].packets += packet.packetCount;
      traffic_Summary.attack[attackType].bits += packet.packetSize;

      traffic_oneSecStats.total.attack.packets += packet.packetCount;
      traffic_oneSecStats.total.attack.bits += packet.packetSize;
      traffic_oneSecStats.protocol[protocolType].attack.packets +=
        packet.packetCount;
      traffic_oneSecStats.protocol[protocolType].attack.bits +=
        packet.packetSize;
      traffic_oneSecStats.attack[attackType].packets += packet.packetCount;
      traffic_oneSecStats.attack[attackType].bits += packet.packetSize;

      if (interface_oneSecStats.interface[`eth${packet.interface}`]) {
        interface_oneSecStats.interface[
          `eth${packet.interface}`
        ].attack.packets += packet.packetCount;
        interface_oneSecStats.interface[`eth${packet.interface}`].attack.bits +=
          packet.packetSize;
      }

      status = "Attack";
      clearTimeout(attackTimeout);
      attackTimeout = setTimeout(resetStatus, 5000);

      //#region Handle Attack Notification
      const anomalyExists = anomaliesDataCache.find(
        (anomaly) =>
          anomaly.AnomaliesTargetIp === packet.dstIP &&
          anomaly.AnomaliesTargetPort === packet.dstPort &&
          anomaly.Anomalies === packet.attackType
      );
      const mutex_key = `${packet.attackType}-${packet.dstIP}-${packet.dstPort}`;
      if (!mutex.has(mutex_key)) {
        mutex.set(mutex_key, true);
        try {
          if (!anomalyExists) {
            const newAnomaly = {
              Anomalies: packet.attackType,
              AnomaliesTargetIp: packet.dstIP,
              AnomaliesTargetPort: packet.dstPort,
              isEmailSent: false,
            };
            newAnomaly.AnomaliesId = await addNewAnomalyToDatabase(
                newAnomaly.Anomalies,
                newAnomaly.AnomaliesTargetIp,
                newAnomaly.AnomaliesTargetPort,
                `Rate: ${packet.packetCount} pps , Throughput: ${packet.packetSize} bps`
            );
            newAnomaly.AnomaliesLastUpdateTime = Date.now();
            anomaliesDataCache.push(newAnomaly);
          } else {
            anomalyExists.AnomaliesStats = `Rate: ${packet.packetCount} pps , Throughput: ${packet.packetSize} bps`;
            anomalyExists.AnomaliesLastUpdateTime = Date.now();
          }
        } catch (err) {
          console.error("Error inserting attack:", err);
        } finally {
          mutex.delete(mutex_key);
        }
      }
      //#endregion
    }

    interfaceStatus.interface[`eth${packet.interface}`].lastPacketTime =
      Date.now();
    interfaceStatus.interface[`eth${packet.interface}`].status = 1;
  };

  const udsSocketPath = config.packet.udsSocketPath;
  try {
    unlinkSync(udsSocketPath);
  } catch (err) {
    if (err.code !== "ENOENT") {
      console.error("Error removing existing UDS file:", err);
    }
  }
  //xử lý gửi email 1 lần duy nhất của mỗi đợt tấn công cho từng user
  const sentFloodEmails = new Map();

  const server = net.createServer((socket) => {
    socket.on("data", (data) => {
      // Assuming each line is terminated with a newline
      const lines = data.toString().trim().split("\n");
      lines.forEach((line) => {
        const lineArr = line.trim().split("  ");
        if (lineArr.length < 10) return;

        const packetObj = {
          srcIP: lineArr[1],
          dstIP: lineArr[2],
          srcPort: parseInt(lineArr[3], 10),
          dstPort: parseInt(lineArr[4], 10),
          protocol: lineArr[5],
          attackType: lineArr[6],
          packetSize:
            (parseInt(lineArr[7], 10) + 18 * parseInt(lineArr[8], 10)) * 8,
          packetCount: parseInt(lineArr[8], 10),
          interface: parseInt(lineArr[9], 10),
        };
        packetArray.push(packetObj);

        if (packetObj.attackType && packetObj.attackType.includes("Flood")) {
          const attackKey = `${packetObj.attackType}-${packetObj.srcIP}-${packetObj.dstIP}`;
          const currentTime = Date.now();

          if (
            !sentFloodEmails.has(attackKey) ||
            currentTime - sentFloodEmails.get(attackKey) > 5 * 60 * 1000
          ) {
            io.emit("floodDetection", "true");
            sendAlertEmailAttack(packetObj);
            sentFloodEmails.set(attackKey, currentTime);
          }
        }
        // const thresholds = {
        //     UDP: defendThreshold.udpthreshold,
        //     TCP: defendThreshold.synthreshold,
        //     ICMP: defendThreshold.icmpthreshold
        // };

        // if (!packetObj.attackType && packetObj.packetCount > (thresholds[packetObj.protocol] || Infinity)) {
        //     notification.NotificationType = 'Warning';
        //     notification.NotificationMessage = `Potential attack: ${packetObj.protocol} from ${packetObj.srcIP} to ${packetObj.dstIP} at ${packetObj.packetCount.toLocaleString()} packets/s`;

        //     io.emit('vulnerableDetection', 'true');
        //     addNotificationToAllUsers(notification);
        // }
      });
    });

    socket.on("error", (err) => {
      console.error("UDS socket error:", err);
    });

    socket.on("close", () => {});
  });

  if (isProduction) {
    server.listen(udsSocketPath, () => {
      console.log(`Listening on UDS: ${udsSocketPath}`);
    });
  } else {
    test(packetArray);
  }

  //#region Handle NetworkAnomalies
  setInterval(() => {
    if (anomaliesDataCache.length !== 0) {
      const currentTime = Date.now();
      const pendingAnomalyUpdates = [];
      anomaliesDataCache = anomaliesDataCache.filter((anomaly) => {
        if (currentTime - anomaly.AnomaliesLastUpdateTime > 5000) {
          const updatePromise = updateEndTimeForAnomaliesToDatabase(
            anomaly.AnomaliesId
          )
            // .then(() => console.log("Updated anomaly:", anomaly.AnomaliesId))
            .catch((err) => {
              console.error("Error updating anomaly:", err);
              return anomaly;
            });
          pendingAnomalyUpdates.push(updatePromise);
          return false;
        }
        return true;
      });
      Promise.all(pendingAnomalyUpdates).then((results) => {
        anomaliesDataCache.push(...results.filter((a) => a));
      });
    }

    for (let i = 1; i <= 8; i++) {
      const interfaceName = `eth${i}`;
      if (interfaceStatus.interface[interfaceName].lastPacketTime) {
        const timeDiff =
          Date.now() - interfaceStatus.interface[interfaceName].lastPacketTime;
        if (timeDiff > 5000) {
          interfaceStatus.interface[interfaceName].status = 0;
        }
      }
    }
  }, 1000);
  //#endregion

  //#region Handle Socket.IO
  setInterval(() => {
    packetArray.forEach(handlePacket);
    const timeNow = Date.now();
    traffic_oneSecStats.timeStamp = timeNow;

    io.emit("traffic", {
      summary: traffic_Summary,
      onsec: traffic_oneSecStats,
      status,
      serverTimestamp: timeNow,
    });
    io.emit("interface", interfaceStatus, interface_oneSecStats, status);
    io.emit("packetMonit", packetArray);

    traffic_oneSecStats = trafficTemplate();
    interface_oneSecStats = interfaceTemplate();
    packetArray.length = 0;
  }, 1000);
  //#endregion
}

function test(packetArray) {
  const generateRandomPacket = (
    srcIP,
    dstIP,
    srcPort,
    dstPort,
    protocol,
    attackType,
    iface
  ) => ({
    srcIP,
    dstIP,
    srcPort,
    dstPort,
    protocol,
    attackType,
    packetSize: Math.floor(Math.random() * 7000000),
    packetCount: Math.floor(Math.random() * 7000000),
    interface: iface,
  });

  const packets = [
    {
      srcIP: "132.170.1.6",
      dstIP: "192.170.123.124",
      srcPort: 90,
      dstPort: 50,
      protocol: "TCP",
      attackType: "TCP Fragment",
      iface: 2,
    },
    {
      srcIP: "132.170.1.1",
      dstIP: "192.170.123.123",
      srcPort: 90,
      dstPort: 50,
      protocol: "UDP",
      attackType: "Normal",
      iface: 1,
    },
    {
      srcIP: "132.170.1.1",
      dstIP: "192.170.123.123",
      srcPort: 90,
      dstPort: 50,
      protocol: "UDP",
      attackType: "UDP Flood",
      iface: 4,
    },
    {
      srcIP: "142.170.1.2",
      dstIP: "172.10.100.102",
      srcPort: 70,
      dstPort: 70,
      protocol: "TCP",
      attackType: "SYN Flood",
      iface: 1,
    },
    {
      srcIP: "152.170.1.4",
      dstIP: "172.10.100.103",
      srcPort: 70,
      dstPort: 70,
      protocol: "ICMP",
      attackType: "ICMP Flood",
      iface: 5,
    },
  ];

  setTimeout(() => {
    const interval = setInterval(() => {
      packets.forEach((packet) =>
        packetArray.push(
          generateRandomPacket(
            packet.srcIP,
            packet.dstIP,
            packet.srcPort,
            packet.dstPort,
            packet.protocol,
            packet.attackType,
            packet.iface
          )
        )
      );
    }, 200);

    setTimeout(() => {
      clearInterval(interval);
      console.log("Stopped packet generation.");
    }, 10000000);
  }, 1000);
}

async function packetCapture(io) {
  anomaliesDataCache = await getActiveAnomaliesFromDatabase();
  PacketProcessing(io);
}

module.exports = { packetCapture };
