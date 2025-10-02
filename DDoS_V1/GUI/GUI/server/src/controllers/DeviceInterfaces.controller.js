const config = require("../config/index");
const progressLog = require("../config").progressLog;
const COLORS = require("../config").COLORS;
const { insertSystemLogToDatabase } = require("../helper/dbo/logs.helper");
const { format } = require("date-fns");
const { vi } = require("date-fns/locale");
const {
  sendCommandToCProgram,
  checkMainCConnection,
} = require("../services/socket.service");
const {
  getAllDeviceInterfaces,
  getMirroringDeviceInterfaces,
  getDeviceInterfaceByInterfaceId,
  updateDeviceInterface,
} = require("../models/DeviceInterfaces.model");
const {
  getDefenseProfilebyDefenseProfileId,
  updateDefenseProfile,
} = require("../models/DefenseProfiles.model");

//#region -------------------- HELPER --------------------
const shouldIncludeKey = (key) => {
  return ![
    "DefenseProfileId",
    "UserId",
    "DefenseProfileName",
    "DefenseProfileDescription",
    "DefenseProfileCreateTime",
    "DefenseProfileLastModified",
    "DefenseProfileUsingTime",
    "DefenseProfileStatus",
    "DefenseProfileType",
  ].includes(key);
};

function buildConfigPackage(
  interfaceObj,
  newProfile,
  includeProtect = true,
  currentProfile = null
) {
  const portNum = interfaceObj.InterfaceName.replace("eth", "");
  let config = "";
  let differentFieldsCount = 0;

  if (includeProtect) {
    config += `PORT${portNum}_PROTECT$${interfaceObj.InterfaceProtectionMode}$`;
    differentFieldsCount++;
  }
  const profileFields = [
    { key: "DetectionTime", cmd: `PORT${portNum}_TIME_DETECT_ATTACK` },
    {
      key: "SYNFloodEnable",
      cmd: `PORT${portNum}_SYN_EN_DIS`,
      transform: (val) => (val ? 1 : 0),
    },
    { key: "SYNFloodSYNThreshold", cmd: `PORT${portNum}_SYN_THR` },
    {
      key: "LandAttackEnable",
      cmd: `PORT${portNum}_LAND_EN_DIS`,
      transform: (val) => (val ? 1 : 0),
    },
    { key: "SYNFloodACKThreshold", cmd: `PORT${portNum}_ACK_THR` },
    {
      key: "UDPFloodEnable",
      cmd: `PORT${portNum}_UDP_EN_DIS`,
      transform: (val) => (val ? 1 : 0),
    },
    { key: "UDPFloodThreshold", cmd: `PORT${portNum}_UDP_THR` },
    { key: "UDPFloodRate", cmd: `PORT${portNum}_UDP_THR_PS` },
    {
      key: "DNSFloodEnable",
      cmd: `PORT${portNum}_DNS_EN_DIS`,
      transform: (val) => (val ? 1 : 0),
    },
    { key: "DNSFloodThreshold", cmd: `PORT${portNum}_DNS_THR` },
    {
      key: "ICMPFloodEnable",
      cmd: `PORT${portNum}_ICMP_EN_DIS`,
      transform: (val) => (val ? 1 : 0),
    },
    { key: "ICMPFloodThreshold", cmd: `PORT${portNum}_ICMP_THR` },
    { key: "ICMPFloodRate", cmd: `PORT${portNum}_ICMP_THR_PSS` },
    {
      key: "IPSecIKEEnable",
      cmd: `PORT${portNum}_IPSEC_IKE_EN_DIS`,
      transform: (val) => (val ? 1 : 0),
    },
    { key: "IPSecIKEThreshold", cmd: `PORT${portNum}_IPSEC_IKE_THR` },
    {
      key: "TCPFragmentEnable",
      cmd: `PORT${portNum}_TCP_FRA_EN_DIS`,
      transform: (val) => (val ? 1 : 0),
    },
    {
      key: "UDPFragmentEnable",
      cmd: `PORT${portNum}_UDP_FRA_EN_DIS`,
      transform: (val) => (val ? 1 : 0),
    },
    {
      key: "HTTPFloodEnable",
      cmd: `PORT${portNum}_HTTP_EN_DIS`,
      transform: (val) => (val ? 1 : 0),
    },
    {
      key: "HTTPSFloodEnable",
      cmd: `PORT${portNum}_HTTPS_EN_DIS`,
      transform: (val) => (val ? 1 : 0),
    },
    { key: "SYNFloodWhiteListTimeOut", cmd: `PORT${portNum}_TIME_WHITE_LIST` },
  ];
  if (newProfile.DefenseProfileName === "DEFAULT") {
    for (const field of profileFields) {
      if (shouldIncludeKey(field.key)) {
        let value = newProfile[field.key];
        if (field.transform) {
          value = field.transform(value);
        }
        config += `${field.cmd}$${value}$`;
        differentFieldsCount++; // Tăng đếm cho mỗi trường được thêm
      }
    }
  } else {
    for (const field of profileFields) {
      if (shouldIncludeKey(field.key)) {
        let newValue = newProfile[field.key];
        let currentValue = currentProfile
          ? currentProfile[field.key]
          : undefined;
        if (field.transform) {
          newValue = field.transform(newValue);
          currentValue =
            currentValue !== undefined
              ? field.transform(currentValue)
              : undefined;
        }
        if (newValue !== currentValue) {
          config += `${field.cmd}$${newValue}$`;
          differentFieldsCount++; // Tăng đếm cho trường khác nhau
        }
      }
    }
  }

  const totalProcessingTimeInSeconds = differentFieldsCount * 2;
  progressLog(
    COLORS.spacing,
    `Total time to process for PORT${portNum}: `,
    COLORS.blue,
    `${totalProcessingTimeInSeconds} seconds`,
    COLORS.reset
  );

  return { config, differentFieldsCount };
}

function convertMirrorSetting(setting) {
  if (setting === "Ingress") return "I";
  if (setting === "Egress") return "E";
  if (setting === "Ingress and Egress") return "IE";
  return setting;
}

//#endregion
// GET
exports.getAllDeviceInterfaces = async (req, res) => {
  config.progressLog(
    config.COLORS.cyan,
    "\n[PROGRESS] : Get All Device Interfaces ...",
    config.COLORS.reset
  );
  try {
    const deviceInterfaces = await getAllDeviceInterfaces();
    if (!deviceInterfaces) {
      return res.status(404).json({ message: "No device interfaces found" });
    }
    return res.status(200).json({
      data: deviceInterfaces,
      progress: "Founded many device interfaces",
    });
  } catch (error) {
    console.log("error ", error);
    return res.status(500).json({
      message: "Get all device interfaces failed",
      progress: "Get all device interfaces failed",
    });
  }
};
exports.getMirroringDeviceInterfaces = async (req, res) => {
  config.progressLog(
    config.COLORS.cyan,
    "\n[PROGRESS] : Get Mirroring Device Interfaces ...",
    config.COLORS.reset
  );
  try {
    const deviceInterfaces = await getMirroringDeviceInterfaces();
    if (!deviceInterfaces) {
      return res.status(404).json({ message: "No device interfaces found" });
    }
    return res.status(200).json({ data: deviceInterfaces });
  } catch (error) {
    console.log("error ", error);
    return res
      .status(500)
      .json({ message: "Get mirroring device interfaces failed" });
  }
};
// OTHER
exports.updateDeviceInterface = async (req, res) => {
  config.progressLog(
    config.COLORS.cyan,
    "\n[PROGRESS] : update Device Interface ...",
    config.COLORS.reset
  );
  try {
    let progressReport = "";
    const deviceInterface = req.body;
    let totalPairs = 0;

    if (!deviceInterface.ports || !Array.isArray(deviceInterface.ports)) {
      return res.status(400).json({
        message: "Request must contain a ports array",
        progress: progressReport,
      });
    }

    for (const port of deviceInterface.ports) {
      if (!port.InterfaceId || !port.InterfaceName) {
        return res.status(400).json({
          message: `Invalid port data: InterfaceId and InterfaceName are required for port ${JSON.stringify(
            port
          )}`,
        });
      }
    }

    const mainCReady = await checkMainCConnection();
    if (!mainCReady) {
      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DeviceInterfaces",
        "Config",
        "Update device interface mode",
        `Update device interface mode: ${deviceInterface}`,
        "Failed",
        "MainC is not running"
      );
      return res.status(500).json({
        message: "MainC is not running. Please check hardware connection.",
        progress: progressReport,
      });
    }

    let allCommands = [];
    let updatesToApply = []; // Store updates to apply only if mainC succeeds

    for (const port of deviceInterface.ports) {
      const databaseDeviceInterface = await getDeviceInterfaceByInterfaceId(
        port.InterfaceId
      );
      if (!databaseDeviceInterface) {
        return res.status(404).json({
          message: `Interface ${port.InterfaceId} not found`,
          progress: progressReport,
        });
      }
      const portNum = port.InterfaceName.replace("eth", "");
      const currentDate = format(new Date(), "yyyy/MM/dd HH:mm:ss", {
        locale: vi,
      });

      const protectionModeChanged =
        port.InterfaceProtectionMode !==
        databaseDeviceInterface.InterfaceProtectionMode;
      const profileChanged =
        port.DefenseProfileId !== databaseDeviceInterface.DefenseProfileId;
      const typeChanged =
        port.InterfaceType !== databaseDeviceInterface.InterfaceType;

      if (profileChanged) {
        const oldProfileId = databaseDeviceInterface.DefenseProfileId;
        const newProfileId = port.DefenseProfileId;

        if (oldProfileId) {
          const oldProfile = await getDefenseProfilebyDefenseProfileId(
            oldProfileId
          );
          if (oldProfile) {
            let usingTime = oldProfile.DefenseProfileUsingTime
              ? JSON.parse(oldProfile.DefenseProfileUsingTime)
              : [];
            usingTime = usingTime.filter(
              (entry) => entry.name !== port.InterfaceName
            );
            oldProfile.DefenseProfileUsingTime =
              usingTime.length > 0 ? JSON.stringify(usingTime) : null;
            oldProfile.DefenseProfileLastModified = currentDate;
            oldProfile.DefenseProfileStatus =
              usingTime.length > 0 ? "Active" : "Inactive";
            config.progressLog(
              config.COLORS.spacing,
              "Preparing update for defense profile with profile id :",
              config.COLORS.yellow,
              oldProfileId,
              config.COLORS.reset
            );
            updatesToApply.push(() =>
              updateDefenseProfile(oldProfileId, oldProfile)
            );
          }
        }

        if (newProfileId) {
          const newProfile = await getDefenseProfilebyDefenseProfileId(
            newProfileId
          );
          if (newProfile) {
            let newUsingTime = newProfile.DefenseProfileUsingTime
              ? JSON.parse(newProfile.DefenseProfileUsingTime)
              : [];
            if (
              !newUsingTime.some((entry) => entry.name === port.InterfaceName)
            ) {
              newUsingTime.push({
                name: port.InterfaceName,
                date: currentDate,
              });
            }
            newProfile.DefenseProfileUsingTime =
              newUsingTime.length > 0 ? JSON.stringify(newUsingTime) : null;
            newProfile.DefenseProfileLastModified = currentDate;
            newProfile.DefenseProfileStatus =
              newUsingTime.length > 0 ? "Active" : "Inactive";
            config.progressLog(
              config.COLORS.spacing,
              "Preparing update for defense profile with profile id :",
              config.COLORS.yellow,
              newProfileId,
              config.COLORS.reset
            );
            updatesToApply.push(() =>
              updateDefenseProfile(newProfileId, newProfile)
            );
          }
        }
      }

      let portCommands = [];
      const newProfile = await getDefenseProfilebyDefenseProfileId(
        port.DefenseProfileId
      );
      const currentProfile = await getDefenseProfilebyDefenseProfileId(
        databaseDeviceInterface.DefenseProfileId
      );
      const isDefaultProfile =
        (newProfile && newProfile.DefenseProfileName === "DEFAULT") ||
        port.DefenseProfileId === 1;

      if (protectionModeChanged && !profileChanged) {
        portCommands.push(
          `PORT${portNum}_PROTECT$${port.InterfaceProtectionMode}$`
        );
        totalPairs++;
      }

      if (!protectionModeChanged && profileChanged) {
        if (newProfile) {
          const { config: fullConfig, differentFieldsCount } =
            buildConfigPackage(port, newProfile, false, currentProfile);
          if (fullConfig) {
            totalPairs += differentFieldsCount;
            portCommands.push(fullConfig);
          }
        }
      }

      if (protectionModeChanged && profileChanged) {
        if (newProfile) {
          const { config: fullConfig, differentFieldsCount } =
            buildConfigPackage(port, newProfile, true, currentProfile);
          if (fullConfig) {
            totalPairs += differentFieldsCount;
            portCommands.push(fullConfig);
          }
        }
      }

      if (portCommands.length > 0) {
        allCommands.push(portCommands.join(""));
      }

      const updatingDeviceInterface = {
        ...databaseDeviceInterface,
        InterfaceType:
          port.InterfaceType || databaseDeviceInterface.InterfaceType,
        InterfaceProtectionMode: port.InterfaceProtectionMode || "IP",
        DefenseProfileId: port.DefenseProfileId,
      };
      config.progressLog(
        config.COLORS.spacing,
        "Preparing update for device interface with interface id :",
        config.COLORS.yellow,
        port.InterfaceId,
        config.COLORS.reset
      );
      updatesToApply.push(() =>
        updateDeviceInterface(port.InterfaceId, updatingDeviceInterface)
      );
    }

    const combinedCommand = allCommands.join("");
    const estimatedTime = totalPairs * 2;
    config.progressLog(
      config.COLORS.spacing,
      `Total time to process for all ports: `,
      config.COLORS.blue,
      `${estimatedTime} seconds`,
      config.COLORS.reset
    );

    if (combinedCommand) {
      const configResult = await sendCommandToCProgram(combinedCommand);

      if (configResult.includes("ERROR")) {
        config.progressLog(
          config.COLORS.spacing,
          config.COLORS.green,
          "Response sent : status code 500 : result from mainC include ERROR!",
          config.COLORS.reset
        );
        await insertSystemLogToDatabase(
          req.user.payload.Id,
          "DeviceInterfaces",
          "Config",
          "Update device interface mode",
          `Update device interface mode: ${combinedCommand}`,
          "Failed",
          `MainC error: ${configResult}`
        );
        return res.status(500).json({
          message: "Failed to update device interface",
          error: `MainC returned error: ${configResult}`,
          progress: progressReport,
        });
      }

      // If mainC success, apply all updates to DB
      progressReport += "mainC updated";
      for (const updateFn of updatesToApply) {
        await updateFn();
      }
      progressReport += " database updated";

      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DeviceInterfaces",
        "Config",
        "Update device interface mode",
        `Update device interface mode: ${combinedCommand}`,
        "Success",
        null
      );

      return res.status(200).json({
        message: "Update command sent to MainC successfully and DB updated",
        progress: progressReport,
        estimatedTime,
      });
    } else {
      // No changes, no command sent, but still success
      return res.status(200).json({
        message: "No changes detected, no command sent to MainC",
        estimatedTime: 0,
        progress: progressReport,
      });
    }
  } catch (error) {
    console.error("Error:", error);
    await insertSystemLogToDatabase(
      req.user.payload.Id,
      "DeviceInterfaces",
      "Config",
      "Update device interface mode",
      `Update device interface mode: ${combinedCommand}`,
      "Failed",
      error.message
    );
    return res.status(500).json({
      message: "Failed to update device interface",
      error: error.message,
      progress: "Failed to update device interface",
    });
  }
};
exports.createMirroringInterface = async (req, res) => {
  progressLog(
    COLORS.cyan,
    "\n[PROGRESS] : Create mirror interface ...",
    COLORS.reset
  );
  let progressReport = "";
  try {
    const {
      MonitorInterfaceId,
      MirrorInterfaceId,
      MirrorSetting,
      MirrorType,
      Value,
    } = req.body;

    if (!MonitorInterfaceId || !MirrorInterfaceId || !MirrorSetting) {
      progressLog(
        COLORS.spacing,
        COLORS.red,
        "[ERROR] : Missing required fields",
        COLORS.reset
      );
      return res
        .status(400)
        .json({ message: "Missing required fields", progress: progressReport });
    }

    if (MonitorInterfaceId === MirrorInterfaceId) {
      progressLog(
        COLORS.spacing,
        COLORS.red,
        "[ERROR] : Cannot mirror to itself",
        COLORS.reset
      );
      return res
        .status(400)
        .json({ message: "Cannot mirror to itself", progress: progressReport });
    }

    const monitorInterface = await getDeviceInterfaceByInterfaceId(
      MonitorInterfaceId
    );
    const mirrorInterface = await getDeviceInterfaceByInterfaceId(
      MirrorInterfaceId
    );

    if (!monitorInterface || !mirrorInterface) {
      progressLog(
        COLORS.spacing,
        COLORS.red,
        "[ERROR] : Monitor or Mirror interface not found",
        COLORS.reset
      );
      return res.status(404).json({
        message: "Monitor or Mirror interface not found",
        progress: progressReport,
      });
    }

    let valueObj = {};
    try {
      valueObj = Value ? JSON.parse(Value) : {};
    } catch (error) {
      console.error("Error parsing Value:", error);
      return res
        .status(400)
        .json({ message: "Invalid Value format", progress: progressReport });
    }

    const mirrorSettingShort = convertMirrorSetting(MirrorSetting);

    let configPackage = `PORT${MirrorInterfaceId}_MIRRORING$${mirrorSettingShort}$`;
    if (MirrorType && Array.isArray(MirrorType)) {
      if (MirrorType.includes("Dest Mac") && valueObj.DestMac) {
        configPackage += `PORT${MirrorInterfaceId}_MONITORED_DST_MAC$${valueObj.DestMac}$`;
      }
      if (MirrorType.includes("Source Mac") && valueObj.SourceMac) {
        configPackage += `PORT${MirrorInterfaceId}_MONITORED_SRC_MAC$${valueObj.SourceMac}$`;
      }
      if (MirrorType.includes("Dest IP")) {
        if (valueObj.DestIPv4) {
          configPackage += `PORT${MirrorInterfaceId}_MONITORED_DST_IPV4$${valueObj.DestIPv4}$`;
        }
        if (valueObj.DestIPv6) {
          configPackage += `PORT${MirrorInterfaceId}_MONITORED_DST_IPV6$${valueObj.DestIPv6}$`;
        }
      }
      if (MirrorType.includes("Source IP")) {
        if (valueObj.SourceIPv4) {
          configPackage += `PORT${MirrorInterfaceId}_MONITORED_SRC_IPV4$${valueObj.SourceIPv4}$`;
        }
        if (valueObj.SourceIPv6) {
          configPackage += `PORT${MirrorInterfaceId}_MONITORED_SRC_IPV6$${valueObj.SourceIPv6}$`;
        }
      }
      if (
        MirrorType.includes("Dest Port") ||
        MirrorType.includes("Source Port")
      ) {
        const src =
          MirrorType.includes("Source Port") && valueObj.SourcePort
            ? valueObj.SourcePort
            : 0;
        const dst =
          MirrorType.includes("Dest Port") && valueObj.DestPort
            ? valueObj.DestPort
            : 0;
        configPackage += `PORT${MirrorInterfaceId}_MONITORED_PORT$${src}-${dst}$`;
      }
      if (MirrorType.includes("Protocol") && valueObj.Protocol) {
        configPackage += `PORT${MirrorInterfaceId}_MONITORED_PROTOCOL$${valueObj.Protocol}$`;
      }
    }

    const fullCmd = `PORT${MirrorInterfaceId}_MONITORED$1$` + configPackage;

    const mainCReady = await checkMainCConnection();
    if (!mainCReady) {
      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DeviceInterfaces",
        "Create",
        "Port Mirroring",
        `MirrorInterface: ${mirrorInterface.InterfaceName}`,
        "Failed",
        "MainC is not running"
      );
      return res.status(500).json({
        message: "MainC is not running. Please check hardware connection.",
        progress: progressReport,
      });
    }

    const pairCount = (fullCmd.match(/PORT\d+_[A-Z0-9_]+\$[^$]*\$/gi) || [])
      .length;
    const estimatedTime = (pairCount === 0 ? 1 : pairCount) * 2;

    try {
      const configResult = await sendCommandToCProgram(fullCmd);

      if (configResult.includes("ERROR")) {
        await insertSystemLogToDatabase(
          req.user.payload.Id,
          "DeviceInterfaces",
          "Create",
          "Port Mirroring",
          `MirrorInterface: ${mirrorInterface.InterfaceName}`,
          "Failed",
          `MainC error: ${configResult}`
        );
        progressLog(
          COLORS.spacing,
          COLORS.green,
          "Response sent : status code 500 : result from mainC include ERROR!",
          COLORS.reset
        );
        return res.status(500).json({
          message: "Failed to create mirroring interface",
          error: `MainC returned error: ${configResult}`,
        });
      } else {
        if (configResult.includes("OK")) {
          progressLog(
            COLORS.reset,
            `Successfully created mirroring for ${mirrorInterface.InterfaceName}`,
            COLORS.reset
          );
          progressReport += "mainC updated";
        } else {
          progressLog(
            COLORS.spacing,
            COLORS.red,
            "[ERROR] : MainC response has unexpected output",
            COLORS.reset
          );
          await insertSystemLogToDatabase(
            req.user.payload.Id,
            "DeviceInterfaces",
            "Create",
            "Port Mirroring",
            `MirrorInterface: ${mirrorInterface.InterfaceName}`,
            "Failed",
            `Unexpected response: ${configResult}`
          );
          return res.status(500).json({
            message: "Failed to create mirroring interface",
            error: `Unexpected response: ${configResult}`,
            progress: progressReport,
          });
        }
      }

      mirrorInterface.InterfaceIsMirroring = 1;
      mirrorInterface.InterfaceToMonitorInterfaceId = MonitorInterfaceId;
      mirrorInterface.InterfaceMirrorSetting = MirrorSetting;
      mirrorInterface.MirrorType = JSON.stringify(MirrorType || []);
      mirrorInterface.Value = JSON.stringify(valueObj);

      await updateDeviceInterface(MirrorInterfaceId, mirrorInterface);
      progressReport += " database updated";

      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DeviceInterfaces",
        "Create",
        "Port Mirroring",
        `MirrorInterface: ${mirrorInterface.InterfaceName}`,
        "Success",
        null
      );
      return res.status(200).json({
        message: "Mirroring interface created successfully",
        estimatedTime,
        progress: progressReport,
      });
    } catch (error) {
      progressLog(
        COLORS.spacing,
        COLORS.red,
        "[ERROR] : cannot processing mainC command!",
        COLORS.reset
      );
      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DeviceInterfaces",
        "Create",
        "Port Mirroring",
        `MirrorInterface: ${mirrorInterface.InterfaceName}`,
        "Failed",
        `Error processing mainC command: ${error.message}`
      );
      return res.status(500).json({
        message: "Failed to create mirroring interface",
        error: error.message,
        progress: progressReport,
      });
    }
  } catch (error) {
    progressLog(COLORS.spacing, COLORS.red, "[ERROR] : ", error, COLORS.reset);
    await insertSystemLogToDatabase(
      req.user.payload.Id,
      "DeviceInterfaces",
      "Create",
      "Port Mirroring",
      `MirrorInterface`,
      "Failed",
      error.message
    );
    return res.status(500).json({
      message: "Internal server error",
      error: error.message,
      progress: progressReport,
    });
  }
};
exports.updateMirroringDeviceInterface = async (req, res) => {
  progressLog(
    COLORS.cyan,
    "\n[PROGRESS] : update mirror interface ...",
    COLORS.reset
  );
  let progressReport = "";
  try {
    const {
      MonitorInterfaceId,
      MirrorInterfaceId,
      MirrorSetting,
      MirrorType,
      Value,
    } = req.body;

    console.log(req.body);

    if (!MonitorInterfaceId || !MirrorInterfaceId) {
      progressLog(
        COLORS.spacing,
        COLORS.red,
        "[ERROR] : Missing MonitorInterfaceId or MirrorInterfaceId",
        COLORS.reset
      );
      return res.status(400).json({
        message: "Missing MonitorInterfaceId or MirrorInterfaceId",
        progress: progressReport,
      });
    }
    if (MonitorInterfaceId === MirrorInterfaceId) {
      progressLog(
        COLORS.spacing,
        COLORS.red,
        "[ERROR] : Cannot mirror to itself",
        COLORS.reset
      );
      return res
        .status(400)
        .json({ message: "Cannot mirror to itself", progress: progressReport });
    }

    const mirroringInterface = await getDeviceInterfaceByInterfaceId(
      MirrorInterfaceId
    );
    if (!mirroringInterface) {
      progressLog(
        COLORS.spacing,
        COLORS.red,
        "[ERROR] : No device interfaces found",
        COLORS.reset
      );
      return res.status(404).json({
        message: "No device interfaces found",
        progress: progressReport,
      });
    }

    let valueObj = {};
    try {
      valueObj = Value ? JSON.parse(Value) : {};
    } catch (error) {
      return res
        .status(400)
        .json({ message: "Invalid Value format", progress: progressReport });
    }

    const oldMirrorTypes = mirroringInterface.MirrorType
      ? JSON.parse(mirroringInterface.MirrorType)
      : [];
    const oldValueObj = mirroringInterface.Value
      ? JSON.parse(mirroringInterface.Value)
      : {};
    const mirrorSettingShort = convertMirrorSetting(MirrorSetting);

    let configPackage = "";

    if (MirrorSetting !== mirroringInterface.InterfaceMirrorSetting) {
      configPackage += `PORT${MirrorInterfaceId}_MIRRORING$${mirrorSettingShort}$`;
    }

    const allTypes = [
      {
        key: "Dest Mac",
        field: "DestMac",
        addCmd: "MONITORED_DST_MAC",
        removeCmd: "MONITORED_DST_MAC_REMOVE",
      },
      {
        key: "Source Mac",
        field: "SourceMac",
        addCmd: "MONITORED_SRC_MAC",
        removeCmd: "MONITORED_SRC_MAC_REMOVE",
      },
      {
        key: "Dest IP",
        field: ["DestIPv4", "DestIPv6"],
        addCmd: ["MONITORED_DST_IPV4", "MONITORED_DST_IPV6"],
        removeCmd: ["MONITORED_DST_IPV4_REMOVE", "MONITORED_DST_IPV6_REMOVE"],
      },
      {
        key: "Source IP",
        field: ["SourceIPv4", "SourceIPv6"],
        addCmd: ["MONITORED_SRC_IPV4", "MONITORED_SRC_IPV6"],
        removeCmd: ["MONITORED_SRC_IPV4_REMOVE", "MONITORED_SRC_IPV6_REMOVE"],
      },
      {
        key: "Dest Port",
        field: "DestPort",
        addCmd: "MONITORED_DST_PORT",
        removeCmd: "MONITORED_DST_PORT_REMOVE",
      },
      {
        key: "Source Port",
        field: "SourcePort",
        addCmd: "MONITORED_SRC_PORT",
        removeCmd: "MONITORED_SRC_PORT_REMOVE",
      },
      {
        key: "Protocol",
        field: "Protocol",
        addCmd: "MONITORED_PROTOCOL",
        removeCmd: "MONITORED_PROTOCOL_REMOVE",
      },
    ];

    for (const type of oldMirrorTypes) {
      if (!MirrorType.includes(type)) {
        const t = allTypes.find((t) => t.key === type);
        if (t) {
          if (Array.isArray(t.field)) {
            t.field.forEach((f, idx) => {
              if (oldValueObj[f]) {
                configPackage += `PORT${MirrorInterfaceId}_${t.removeCmd[idx]}$${oldValueObj[f]}$`;
              }
            });
          } else {
            if (oldValueObj[t.field]) {
              configPackage += `PORT${MirrorInterfaceId}_${t.removeCmd}$${
                oldValueObj[t.field]
              }$`;
            }
          }
        }
      }
    }

    for (const type of MirrorType) {
      const t = allTypes.find((t) => t.key === type);
      if (t) {
        if (!oldMirrorTypes.includes(type)) {
          if (Array.isArray(t.field)) {
            t.field.forEach((f, idx) => {
              if (valueObj[f]) {
                configPackage += `PORT${MirrorInterfaceId}_${t.addCmd[idx]}$${valueObj[f]}$`;
              }
            });
          } else {
            if (valueObj[t.field]) {
              configPackage += `PORT${MirrorInterfaceId}_${t.addCmd}$${
                valueObj[t.field]
              }$`;
            }
          }
        }
      }
    }

    if (!configPackage) {
      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DeviceInterfaces",
        "Config",
        "Update device interfaces mode",
        `Update device interface: ${mirroringInterface.InterfaceName}`,
        "Failed",
        "No changes detected"
      );
      return res.status(400).json({ message: "No changes detected" });
    }

    const mainCReady = await checkMainCConnection();
    if (!mainCReady) {
      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DeviceInterfaces",
        "Config",
        "Update device interfaces mode",
        `Update device interface`,
        "Failed",
        "MainC is not running"
      );
      return res.status(500).json({
        message: "MainC is not running. Please check hardware connection.",
        progress: progressReport,
      });
    }

    const pairCount = (
      configPackage.match(/PORT\d+_[A-Z0-9_]+\$[^$]*\$/gi) || []
    ).length;
    const estimatedTime = (pairCount === 0 ? 1 : pairCount) * 2;

    try {
      if (configPackage) {
        const configResult = await sendCommandToCProgram(configPackage);

        if (configResult.includes("ERROR")) {
          progressLog(
            COLORS.spacing,
            COLORS.green,
            "Response sent : status code 500 : result from mainC include ERROR!",
            COLORS.reset
          );
          await insertSystemLogToDatabase(
            req.user.payload.Id,
            "DeviceInterfaces",
            "Config",
            "Update device interfaces mode",
            `Update device interface: ${mirroringInterface.InterfaceName}`,
            "Failed",
            `MainC error: ${configResult}`
          );
          return res.status(500).json({
            message: "Failed to update mirroring interface",
            error: `MainC returned error: ${configResult}`,
          });
        } else {
          if (configResult.includes("OK")) {
            progressLog(
              COLORS.reset,
              `Successfully updated mirroring for ${mirroringInterface.InterfaceName}`,
              COLORS.reset
            );
            progressReport += "mainC updated";
          } else {
            progressLog(
              COLORS.spacing,
              COLORS.red,
              "[ERROR] : MainC response has unexpected output",
              COLORS.reset
            );
            await insertSystemLogToDatabase(
              req.user.payload.Id,
              "DeviceInterfaces",
              "Config",
              "Update device interfaces mode",
              `Update device interface: ${mirroringInterface.InterfaceName}`,
              "Failed",
              `Unexpected response: ${configResult}`
            );
            return res.status(500).json({
              message: "Failed to update mirroring interface",
              error: `Unexpected response: ${configResult}`,
              progress: progressReport,
            });
          }
        }

        mirroringInterface.InterfaceIsMirroring = 1;
        mirroringInterface.InterfaceToMonitorInterfaceId = MonitorInterfaceId;
        mirroringInterface.InterfaceMirrorSetting = MirrorSetting;
        mirroringInterface.MirrorType = JSON.stringify(MirrorType || []);
        mirroringInterface.Value = JSON.stringify(valueObj);

        await updateDeviceInterface(MirrorInterfaceId, mirroringInterface);
        progressReport += " database updated";

        await insertSystemLogToDatabase(
          req.user.payload.Id,
          "DeviceInterfaces",
          "Config",
          "Update device interfaces mode",
          `Update device interface: ${mirroringInterface.InterfaceName}`,
          "Success",
          null
        );
        progressLog(
          COLORS.spacing,
          COLORS.green,
          "Response sent : status code 200 : Mirroring interface updated successfully!",
          COLORS.reset
        );
        return res.status(200).json({
          message: "Mirroring interface updated successfully",
          estimatedTime,
          progress: progressReport,
        });
      } else {
        progressLog(
          COLORS.spacing,
          COLORS.green,
          "Response sent : status code 200 : Mirroring interface updated successfully!",
          COLORS.reset
        );
        return res.status(200).json({
          message: "No changes detected, no command sent to MainC",
          estimatedTime: 0,
          progress: progressReport,
        });
      }
    } catch (error) {
      console.error("Error processing mainC command:", error);
      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DeviceInterfaces",
        "Config",
        "Update device interfaces mode",
        `Update device interface: ${mirroringInterface.InterfaceName}`,
        "Failed",
        `Error processing mainC command: ${error.message}`
      );
      progressLog(
        COLORS.spacing,
        COLORS.red,
        "[ERROR] : Failed to update mirroring interface!",
        COLORS.reset
      );
      return res.status(500).json({
        message: "Failed to update mirroring interface",
        error: error.message,
        progress: progressReport,
      });
    }
  } catch (error) {
    console.error("error", error);
    await insertSystemLogToDatabase(
      req.user.payload.Id,
      "DeviceInterfaces",
      "Config",
      "Update device interfaces mode",
      `Update device interface`,
      "Failed",
      error.message
    );
    progressLog(
      COLORS.spacing,
      COLORS.red,
      "[ERROR] : Update device interface failed!",
      COLORS.reset
    );
    return res.status(500).json({
      message: "Update device interface failed",
      error: error.message,
      progress: progressReport,
    });
  }
};
exports.deleteMirroringInterface = async (req, res) => {
  progressLog(
    COLORS.cyan,
    "\n[PROGRESS] : delete mirror Interface ...",
    COLORS.reset
  );
  let progressReport = "";
  try {
    const MirrorInterfaceId = req.params.MirrorInterfaceId;
    const mirrorInterface = await getDeviceInterfaceByInterfaceId(
      MirrorInterfaceId
    );
    if (!mirrorInterface) {
      progressLog(
        COLORS.spacing,
        COLORS.red,
        "[ERROR] : Interface not found!",
        COLORS.reset
      );
      return res
        .status(404)
        .json({ message: "Interface not found", progress: progressReport });
    }

    const mirrorSettingShort = convertMirrorSetting(
      mirrorInterface.InterfaceMirrorSetting
    );
    const valueObj = mirrorInterface.Value
      ? JSON.parse(mirrorInterface.Value)
      : {};
    const mirrorTypes = mirrorInterface.MirrorType
      ? JSON.parse(mirrorInterface.MirrorType)
      : [];

    let configPackage = `PORT${MirrorInterfaceId}_MIRRORING_REMOVE$${mirrorSettingShort}$`;

    if (Array.isArray(mirrorTypes)) {
      if (mirrorTypes.includes("Dest Mac") && valueObj.DestMac) {
        configPackage += `PORT${MirrorInterfaceId}_MONITORED_DST_MAC_REMOVE$${valueObj.DestMac}$`;
      }
      if (mirrorTypes.includes("Source Mac") && valueObj.SourceMac) {
        configPackage += `PORT${MirrorInterfaceId}_MONITORED_SRC_MAC_REMOVE$${valueObj.SourceMac}$`;
      }
      if (mirrorTypes.includes("Dest IP")) {
        if (valueObj.DestIPv4) {
          configPackage += `PORT${MirrorInterfaceId}_MONITORED_DST_IPV4_REMOVE$${valueObj.DestIPv4}$`;
        }
        if (valueObj.DestIPv6) {
          configPackage += `PORT${MirrorInterfaceId}_MONITORED_DST_IPV6_REMOVE$${valueObj.DestIPv6}$`;
        }
      }
      if (mirrorTypes.includes("Source IP")) {
        if (valueObj.SourceIPv4) {
          configPackage += `PORT${MirrorInterfaceId}_MONITORED_SRC_IPV4_REMOVE$${valueObj.SourceIPv4}$`;
        }
        if (valueObj.SourceIPv6) {
          configPackage += `PORT${MirrorInterfaceId}_MONITORED_SRC_IPV6_REMOVE$${valueObj.SourceIPv6}$`;
        }
      }
      if (
        mirrorTypes.includes("Dest Port") ||
        mirrorTypes.includes("Source Port")
      ) {
        configPackage += `PORT${MirrorInterfaceId}_MONITORED_PORT_REMOVE$0-0$`;
      }
      if (mirrorTypes.includes("Protocol") && valueObj.Protocol) {
        configPackage += `PORT${MirrorInterfaceId}_MONITORED_PROTOCOL_REMOVE$${valueObj.Protocol}$`;
      }
    }

    const monitoredCmd = `PORT${MirrorInterfaceId}_MONITORED$0$`;
    const fullCmd = monitoredCmd + configPackage;

    const mainCReady = await checkMainCConnection();
    if (!mainCReady) {
      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DeviceInterfaces",
        "Delete",
        "Delete port mirroring",
        `MirrorInterfaceId: ${MirrorInterfaceId}`,
        "Failed",
        "MainC is not running"
      );
      return res.status(500).json({
        message: "MainC is not running. Please check hardware connection.",
        progress: progressReport,
      });
    }

    const pairCount = (fullCmd.match(/PORT\d+_[A-Z0-9_]+\$[^$]*\$/gi) || [])
      .length;
    const estimatedTime = (pairCount === 0 ? 1 : pairCount) * 2;

    try {
      const configResult = await sendCommandToCProgram(fullCmd);

      if (configResult.includes("ERROR")) {
        progressLog(
          COLORS.spacing,
          COLORS.green,
          "Response sent : status code 500 : result from mainC include ERROR!",
          COLORS.reset
        );
        await insertSystemLogToDatabase(
          req.user.payload.Id,
          "DeviceInterfaces",
          "Delete",
          "Delete port mirroring",
          `MirrorInterfaceId: ${MirrorInterfaceId}`,
          "Failed",
          `MainC error: ${configResult}`
        );
        return res.status(500).json({
          message: "Failed to delete mirroring interface",
          error: `MainC returned error: ${configResult}`,
          progress: progressReport,
        });
      } else {
        if (configResult.includes("OK")) {
          progressLog(
            COLORS.spacing,
            COLORS.green,
            `Successfully deleted mirroring for interface ${MirrorInterfaceId}`,
            COLORS.reset
          );
          progressReport += `mainC updated`;
        } else {
          progressLog(
            COLORS.spacing,
            COLORS.red,
            "[ERROR] : MainC response has unexpected output",
            COLORS.reset
          );
          await insertSystemLogToDatabase(
            req.user.payload.Id,
            "DeviceInterfaces",
            "Delete",
            "Delete port mirroring",
            `MirrorInterfaceId: ${MirrorInterfaceId}`,
            "Failed",
            `Unexpected response: ${configResult}`
          );
          return res.status(500).json({
            message: "Failed to delete mirroring interface",
            error: `Unexpected response: ${configResult}`,
            progress: progressReport,
          });
        }
      }

      mirrorInterface.InterfaceIsMirroring = 0;
      mirrorInterface.InterfaceToMonitorInterfaceId = null;
      mirrorInterface.InterfaceMirrorSetting = null;
      mirrorInterface.MirrorType = null;
      mirrorInterface.Value = null;

      await updateDeviceInterface(MirrorInterfaceId, mirrorInterface);
      progressReport += ` database updated`;

      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DeviceInterfaces",
        "Delete",
        "Delete port mirroring",
        `MirrorInterfaceId: ${MirrorInterfaceId}`,
        "Success",
        null
      );

      return res.status(200).json({
        message: "Mirroring interface deleted successfully",
        estimatedTime,
        progress: progressReport,
      });
    } catch (error) {
      console.error("Error processing mainC command:", error);
      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DeviceInterfaces",
        "Delete",
        "Delete port mirroring",
        `MirrorInterfaceId: ${MirrorInterfaceId}`,
        "Failed",
        `Error processing mainC command: ${error.message}`
      );
      return res.status(500).json({
        message: "Failed to delete mirroring interface",
        error: error.message,
        progress: progressReport,
      });
    }
  } catch (error) {
    console.error("error", error);
    await insertSystemLogToDatabase(
      req.user.payload.Id,
      "DeviceInterfaces",
      "Delete",
      "Delete port mirroring",
      `MirrorInterfaceId: ${req.params.MirrorInterfaceId}`,
      "Failed",
      error.message
    );
    return res.status(500).json({
      message: "Internal server error",
      error: error.message,
      progress: progressReport,
    });
  }
};
