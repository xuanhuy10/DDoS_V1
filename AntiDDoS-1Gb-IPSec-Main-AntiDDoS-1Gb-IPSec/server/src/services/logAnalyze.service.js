const fs = require("fs");
const path = require("path");
const readline = require("readline");

// Helper: Parse timestamp from the beginning of a line (format: "YYYY-MM-DD HH:mm:ss  ...")
const parseLineTime = (line) => {
  const match = line.match(/^(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})/);
  return match ? new Date(match[1].replace(" ", "T") + "Z") : null;
};

// Binary search to find file offset for a target time (start or end)
const findOffsetByTime = (filePath, targetDate, isStart = true) => {
  const stats = fs.statSync(filePath);
  if (stats.size === 0) return isStart ? 0 : stats.size;

  let fd;
  try {
    fd = fs.openSync(filePath, "r");
  } catch (err) {
    console.error(`Error opening file ${filePath}:`, err);
    return isStart ? stats.size : 0;
  }

  let low = 0;
  let high = stats.size - 1;
  let result = isStart ? stats.size : 0; // Default: full file if not found

  const bufferSize = 1024; // Chunk size for reading
  try {
    while (low <= high) {
      const mid = Math.floor((low + high) / 2);
      const buffer = Buffer.alloc(bufferSize);
      let bytesRead;
      try {
        bytesRead = fs.readSync(
          fd,
          buffer,
          0,
          bufferSize,
          Math.max(0, mid - bufferSize / 2)
        );
      } catch (err) {
        // Fallback if read fails
        if (isStart) high = mid - 1;
        else low = mid + 1;
        continue;
      }

      const chunk = buffer.slice(0, bytesRead).toString("utf8");
      // Split into lines and get the last non-empty line in chunk
      const lines = chunk.split("\n").filter((l) => l.trim());
      if (lines.length === 0) {
        // No valid line, adjust search
        if (isStart) high = mid - 1;
        else low = mid + 1;
        continue;
      }

      const lastLine = lines[lines.length - 1];
      const time = parseLineTime(lastLine);
      if (!time || isNaN(time.getTime())) {
        // Invalid time, adjust
        if (isStart) high = mid - 1;
        else low = mid + 1;
        continue;
      }

      if (isStart) {
        if (time >= targetDate) {
          result = mid;
          high = mid - 1;
        } else {
          low = mid + 1;
        }
      } else {
        if (time <= targetDate) {
          result = mid;
          low = mid + 1;
        } else {
          high = mid - 1;
        }
      }
    }

    // Refine to nearest line boundary (approximate for start)
    if (isStart) {
      const buffer = Buffer.alloc(1024);
      const bytesRead = fs.readSync(fd, buffer, 0, 1024, result);
      const chunk = buffer.slice(0, bytesRead).toString("utf8");
      const lineStart = chunk.lastIndexOf("\n") + 1;
      result = Math.max(0, result + lineStart - 1024);
    }
  } finally {
    fs.closeSync(fd);
  }

  return result;
};

// Function groupDataByTime and compactor
const groupDataByTime = (logEntries) => {
  console.log("Grouping data by time...", logEntries.length);
  const groupedData = {};
  logEntries.forEach((logEntry) => {
    const time = new Date(logEntry.Time);
    time.setSeconds(0, 0);
    const minuteKey = time.getTime();
    if (!groupedData[minuteKey]) {
      groupedData[minuteKey] = [];
    }
    groupedData[minuteKey].push({
      SrcIp: logEntry.SrcIp,
      DstIp: logEntry.DstIp,
      SrcPort: logEntry.SrcPort,
      DstPort: logEntry.DstPort,
      Protocol: logEntry.Protocol,
      Type: logEntry.Type,
      pps: logEntry.pps,
      bps: logEntry.bps,
    });
  });
  return groupedData;
};

const compactor = (groupedData) => {
  const attackMap = {
    "ICMP Flood": ["icmpFlood"],
    "UDP Fragment": ["udpFrag"],
    "TCP Fragment": ["tcpFrag"],
    "SYN Flood": ["synFlood"],
    "UDP Flood": ["udpFlood"],
    "DNS Flood": ["dnsFlood"],
    "LAND Attack": ["land"],
    "IPSec IKE Flood": ["ipsec"],
    "HTTP Flood": ["httpFlood"],
    "DNS AUTH PKT": ["dnsAuthFlood"],
  };

  let additionalData = {
    peak: { bypass: { bps: 0, pps: 0 }, attack: { bps: 0, pps: 0 } },
    total: { bypass: { bits: 0, packets: 0 }, attack: { bits: 0, packets: 0 } },
    srcIp: {},
    dstIp: {},
    protocol: {
      icmp: {
        bypass: { bits: 0, packets: 0 },
        attack: { bits: 0, packets: 0 },
      },
      udp: { bypass: { bits: 0, packets: 0 }, attack: { bits: 0, packets: 0 } },
      tcp: { bypass: { bits: 0, packets: 0 }, attack: { bits: 0, packets: 0 } },
      http: {
        bypass: { bits: 0, packets: 0 },
        attack: { bits: 0, packets: 0 },
      },
      dns: { bypass: { bits: 0, packets: 0 }, attack: { bits: 0, packets: 0 } },
      esp: { bypass: { bits: 0, packets: 0 }, attack: { bits: 0, packets: 0 } },
      unknown: {
        bypass: { bits: 0, packets: 0 },
        attack: { bits: 0, packets: 0 },
      },
    },
    attack: {
      icmpFlood: { bits: 0, packets: 0 },
      udpFlood: { bits: 0, packets: 0 },
      dnsFlood: { bits: 0, packets: 0 },
      udpFrag: { bits: 0, packets: 0 },
      tcpFrag: { bits: 0, packets: 0 },
      synFlood: { bits: 0, packets: 0 },
      land: { bits: 0, packets: 0 },
      httpFlood: { bits: 0, packets: 0 },
      ipsec: { bits: 0, packets: 0 },
      unknown: { bits: 0, packets: 0 },
    },
  };

  let timeSeriesData = [];
  const sortedTimes = Object.keys(groupedData)
    .map(Number)
    .sort((a, b) => a - b);

  if (sortedTimes.length === 0) {
    console.log("No timestamps found in groupedData.");
    return { timeSeries: [], summary: additionalData };
  }

  sortedTimes.forEach((time) => {
    let tmpPktPayload = {};
    tmpPktPayload[time] = [];

    groupedData[time].forEach((record) => {
      let found = false;
      for (let k = 0; k < tmpPktPayload[time].length; k++) {
        if (
          record.SrcIp === tmpPktPayload[time][k].SrcIp &&
          record.DstIp === tmpPktPayload[time][k].DstIp &&
          record.DstPort === tmpPktPayload[time][k].DstPort &&
          record.Protocol === tmpPktPayload[time][k].Protocol &&
          record.Type === tmpPktPayload[time][k].Type
        ) {
          tmpPktPayload[time][k].pps += record.pps;
          tmpPktPayload[time][k].bps += record.bps;

          if (!additionalData.srcIp[record.SrcIp]) {
            additionalData.srcIp[record.SrcIp] = {
              packets: record.pps,
              bits: record.bps,
            };
          } else {
            additionalData.srcIp[record.SrcIp].packets += record.pps;
            additionalData.srcIp[record.SrcIp].bits += record.bps;
          }

          if (!additionalData.dstIp[record.DstIp]) {
            additionalData.dstIp[record.DstIp] = {
              packets: record.pps,
              bits: record.bps,
            };
          } else {
            additionalData.dstIp[record.DstIp].packets += record.pps;
            additionalData.dstIp[record.DstIp].bits += record.bps;
          }

          let protocol = record.Protocol.toLowerCase();
          const attackType = attackMap[record.Type]
            ? attackMap[record.Type][0]
            : "unknown";

          if (
            (record.DstPort === "80" || record.DstPort === "443") &&
            protocol === "tcp"
          ) {
            protocol = "http";
          }
          if (record.DstPort === "53" && protocol === "udp") {
            protocol = "dns";
          }
          if (record.DstPort === "50" && protocol === "udp") {
            protocol = "esp";
          }

          if (record.Type === "Normal") {
            additionalData.protocol[protocol].bypass.bits += record.bps;
            additionalData.protocol[protocol].bypass.packets += record.pps;
            additionalData.total.bypass.bits += record.bps;
            additionalData.total.bypass.packets += record.pps;

            if (record.pps > additionalData.peak.bypass.pps) {
              additionalData.peak.bypass.pps = record.pps;
            }
            if (record.bps > additionalData.peak.bypass.bps) {
              additionalData.peak.bypass.bps = record.bps;
            }
          } else {
            additionalData.protocol[protocol].attack.bits += record.bps;
            additionalData.protocol[protocol].attack.packets += record.pps;
            additionalData.attack[attackType].bits += record.bps;
            additionalData.attack[attackType].packets += record.pps;
            additionalData.total.attack.bits += record.bps;
            additionalData.total.attack.packets += record.pps;

            if (record.pps > additionalData.peak.attack.pps) {
              additionalData.peak.attack.pps = record.pps;
            }
            if (record.bps > additionalData.peak.attack.bps) {
              additionalData.peak.attack.bps = record.bps;
            }
          }
          found = true;
          break;
        }
      }

      if (!found) {
        tmpPktPayload[time].push(record);
      }
    });

    timeSeriesData.push(tmpPktPayload);
  });

  return { timeSeries: timeSeriesData, summary: additionalData };
};

const compactData = (groupedData) => {
  const times = Object.keys(groupedData)
    .map(Number)
    .sort((a, b) => a - b);
  if (times.length === 0) {
    console.log("No data available in groupedData.");
    return { timeSeries: [], summary: {} };
  }

  const minTime = new Date(times[0]);
  const maxTime = new Date(times[times.length - 1]);
  console.log(
    `Time range: ${
      (maxTime - minTime) / 1000
    }s, Min: ${minTime.toISOString()}, Max: ${maxTime.toISOString()}`
  );

  if (Object.keys(groupedData).length > 1000) {
    const sortedKeys = Object.keys(groupedData).sort((a, b) => a - b);
    const limitedKeys = sortedKeys.slice(-1000);
    const limitedGroupedData = {};
    limitedKeys.forEach((k) => (limitedGroupedData[k] = groupedData[k]));
    const compactedData = compactor(limitedGroupedData);
    return { ...compactedData, status: "success" };
  }

  const compactedData = compactor(groupedData);
  return { ...compactedData, status: "success" };
};

// Helper function to read at position using fd
const readAtPosition = (filePath, buffer, offset, length, position) => {
  let fd;
  try {
    fd = fs.openSync(filePath, "r");
    return fs.readSync(fd, buffer, offset, length, position);
  } finally {
    if (fd) fs.closeSync(fd);
  }
};

// Function to get time range of a log file (optimized with binary search)
const getFileTimeRange = async (filePath) => {
  if (!fs.existsSync(filePath)) return null;

  const stats = fs.statSync(filePath);
  if (stats.size === 0) return null;

  // Approximate first time (search for earliest possible)
  const firstOffset = findOffsetByTime(filePath, new Date(0), true);
  const buffer = Buffer.alloc(1024);
  let bytesRead = readAtPosition(filePath, buffer, 0, 1024, firstOffset);
  let chunk = buffer.slice(0, bytesRead).toString("utf8");
  const firstLines = chunk.split("\n").filter((l) => l.trim());
  const firstTime = firstLines.length > 0 ? parseLineTime(firstLines[0]) : null;

  // Approximate last time
  const lastOffset = findOffsetByTime(filePath, new Date(9999, 11, 31), false);
  bytesRead = readAtPosition(
    filePath,
    buffer,
    0,
    1024,
    Math.max(0, lastOffset - 1023)
  );
  chunk = buffer.slice(0, bytesRead).toString("utf8");
  const lastLines = chunk.split("\n").filter((l) => l.trim());
  const lastTime =
    lastLines.length > 0
      ? parseLineTime(lastLines[lastLines.length - 1])
      : null;

  if (
    !firstTime ||
    !lastTime ||
    isNaN(firstTime.getTime()) ||
    isNaN(lastTime.getTime())
  ) {
    // Fallback to full scan if binary fails
    return new Promise((resolve) => {
      const stream = fs.createReadStream(filePath, { encoding: "utf8" });
      const rl = readline.createInterface({ input: stream });
      let firstTimeFallback = null,
        lastTimeFallback = null;

      rl.on("line", (line) => {
        if (line.trim() === "") return;
        const parts = line.split("  ");
        if (parts.length < 9) return;

        const time = new Date(parts[0].replace(" ", "T") + "Z");
        if (isNaN(time.getTime())) return;

        if (!firstTimeFallback || time < firstTimeFallback)
          firstTimeFallback = time;
        if (!lastTimeFallback || time > lastTimeFallback)
          lastTimeFallback = time;
      });

      rl.on("close", () =>
        resolve({ startTime: firstTimeFallback, endTime: lastTimeFallback })
      );
      rl.on("error", () => resolve(null));
      stream.on("error", () => resolve(null));
    });
  }

  return { startTime: firstTime, endTime: lastTime };
};

// Read file async with stream (updated with offsets)
const readLogFileAsync = (fileInfo, startDate, endDate) => {
  return new Promise((resolve, reject) => {
    const filePath = path.join(fileInfo.dir, fileInfo.file);
    if (!fs.existsSync(filePath)) {
      resolve([]);
      return;
    }

    const stats = fs.statSync(filePath);
    const startOffset = findOffsetByTime(filePath, startDate, true);
    const endOffset = findOffsetByTime(filePath, endDate, false);

    if (
      startOffset >= stats.size ||
      endOffset <= 0 ||
      startOffset >= endOffset
    ) {
      resolve([]);
      return;
    }

    const logEntries = [];
    const stream = fs.createReadStream(filePath, {
      encoding: "utf8",
      start: startOffset,
      end: endOffset,
    });
    const rl = readline.createInterface({ input: stream });

    let linesProcessed = 0;
    rl.on("line", (logLine) => {
      if (logLine.trim() === "") return;

      const logParts = logLine.split("  ");
      if (logParts.length < 9) return;

      const timeStr = logParts[0];
      const Time = new Date(timeStr.replace(" ", "T") + "Z");
      if (isNaN(Time.getTime())) return;

      if (Time >= startDate && Time <= endDate) {
        const pps = parseInt(logParts[7], 10) || 0;
        const bps = (parseInt(logParts[8], 10) || 0 + 18 * pps) * 8;

        logEntries.push({
          Time,
          SrcIp: logParts[1],
          DstIp: logParts[2],
          SrcPort: logParts[3],
          DstPort: logParts[4],
          Protocol: logParts[5],
          Type: logParts[6],
          pps,
          bps,
        });
      }
      linesProcessed++;
    });

    rl.on("close", () => {
      console.log(
        `Processed ${linesProcessed} lines from ${filePath}, added ${logEntries.length} entries`
      );
      resolve(logEntries);
    });

    rl.on("error", reject);
    stream.on("error", reject);
  });
};

// Function logAnalyze with startTime and endTime (updated to pass dates to readLogFileAsync)
async function logAnalyze(
  startTime,
  endTime,
  isServer = process.env.NODE_ENV === "production"
) {
  const startDate = new Date(startTime + "Z");
  const endDate = new Date(endTime + "Z");
  console.log(
    `Analyzing logs from ${startDate.toISOString()} to ${endDate.toISOString()}`
  );

  // Select file depend on environment
  const logsDirs = isServer
    ? [
        "/home/antiddos/DDoS_V1/Log/Log_Flood",
        "/home/antiddos/DDoS_V1/Log/Log_Normal",
      ]
    : [path.join(__dirname, "./logs")];

  // Garther file from all log directories
  let allLogFiles = [];
  for (const dir of logsDirs) {
    if (fs.existsSync(dir)) {
      const files = fs.readdirSync(dir).map((file) => ({ file, dir }));
      allLogFiles.push(...files);
    }
  }

  if (allLogFiles.length === 0) {
    console.log("No log directories or files found.");
    return { timeSeries: [], summary: {}, status: "no_data" };
  }

  // Filter file with time range
  const relevantFiles = [];
  for (const fileInfo of allLogFiles) {
    const filePath = path.join(fileInfo.dir, fileInfo.file);
    const timeRange = await getFileTimeRange(filePath);
    if (
      timeRange &&
      timeRange.endTime >= startDate &&
      timeRange.startTime <= endDate
    ) {
      relevantFiles.push(fileInfo);
    }
  }

  console.log(`Relevant files: ${relevantFiles.length}`);

  try {
    // Read file song song
    console.log(`Reading ${relevantFiles.length} files in parallel...`);
    const allEntriesPromises = relevantFiles.map((fileInfo) =>
      readLogFileAsync(fileInfo, startDate, endDate)
    );
    const allFileEntries = await Promise.all(allEntriesPromises);

    // Flatten entries
    const logEntries = allFileEntries.flat();
    console.log("Total log entries:", logEntries.length);

    if (logEntries.length === 0) {
      console.log("No log entries within time range.");
      return { timeSeries: [], summary: {}, status: "no_data" };
    }

    // Merge group và compact
    const groupedData = groupDataByTime(logEntries);
    const compactedData = compactData(groupedData);

    return { ...compactedData, status: "success" };
  } catch (error) {
    console.error("Error in logAnalyze:", error);
    return { timeSeries: [], summary: {}, status: "error" };
  }
}

module.exports = { logAnalyze };
