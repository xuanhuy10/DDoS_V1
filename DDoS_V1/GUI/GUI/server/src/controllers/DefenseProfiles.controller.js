const { format } = require("date-fns");
const { vi } = require("date-fns/locale");
const config = require("../config/index");
const progressLog = require("../config").progressLog;
const COLORS = require("../config").COLORS;

const { mapConfig } = require("../helper/packet.helper");
const { insertSystemLogToDatabase } = require("../helper/dbo/logs.helper");
const {
  sendCommandToCProgram,
  checkMainCConnection,
} = require("../services/socket.service");
const { getAllDeviceInterfaces } = require("../models/DeviceInterfaces.model");
const { updateDeviceInterface } = require("../models/DeviceInterfaces.model");
const {
  getAllDefenseProfiles,
  getDefenseProfilesbyOffset,
  getActiveDefenseProfile,
  getDefenseProfilebyDefenseProfileId,
  getAllDefenseProfilesbyUserId,
  getAttackTypeOfDefenseProfile,
  getAllThresholdByActiveDefenseProfile,
  insertDefenseProfile,
  updateDefenseProfile,
  deleteDefenseProfile,
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

const updateUsingTime = (profile, isCurrent = false) => {
  let usingTimeArray = profile.DefenseProfileUsingTime
    ? JSON.parse(profile.DefenseProfileUsingTime)
    : [];
  usingTimeArray.push({
    start: format(new Date(), "yyyy/MM/dd HH:mm:ss", { locale: vi }),
    end: isCurrent
      ? format(new Date(), "yyyy/MM/dd HH:mm:ss", { locale: vi })
      : null,
  });
  profile.DefenseProfileUsingTime = JSON.stringify(usingTimeArray);
};

const buildConfigPackage = (config, currentConfig, portName = "PORT1") => {
  let configPackage = "";
  let differentFieldsCount = 0;
  const isDefault = config.DefenseProfileName === "DEFAULT";
  for (const key in config) {
    if (config.hasOwnProperty(key) && shouldIncludeKey(key)) {
      let value = config[key];
      if (value === true) value = 1;
      if (value === false) value = 0;
      if (isDefault || currentConfig[key] !== value) {
        differentFieldsCount++;
        const mappedKey = mapConfig.getKeyByValue(key);
        if (mappedKey) {
          configPackage += `${portName}_${mappedKey}$${value}$`;
        }
      }
    }
  }
  const totalProcessingTimeInSeconds = differentFieldsCount * 2;
  progressLog(
    COLORS.spacing,
    `Total time to process for ${portName}: `,
    COLORS.blue,
    `${totalProcessingTimeInSeconds} seconds`,
    COLORS.reset
  );
  return { configPackage, differentFieldsCount };
};

const buildFullConfigPackage = (config, portName = "PORT1") => {
  let configPackage = "";
  let fieldsCount = 0;
  for (const key in config) {
    if (config.hasOwnProperty(key) && shouldIncludeKey(key)) {
      let value = config[key];
      if (value === true) value = 1;
      if (value === false) value = 0;
      fieldsCount++;
      const mappedKey = mapConfig.getKeyByValue(key);
      if (mappedKey) {
        configPackage += `${portName}_${mappedKey}$${value}$`;
      }
    }
  }
  const totalProcessingTimeInSeconds = fieldsCount * 2;
  progressLog(
    COLORS.spacing,
    `Total time to process for ${portName}: `,
    COLORS.blue,
    `${totalProcessingTimeInSeconds} seconds`,
    COLORS.reset
  );
  return { configPackage, fieldsCount };
};

const checkDuplicateName = async (name) => {
  const profiles = await getAllDefenseProfiles();
  return profiles.some(
    (profile) => profile.DefenseProfileName.toLowerCase() === name.toLowerCase()
  );
};

function buildConfigPackageFromDevice(
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
        differentFieldsCount++;
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
          differentFieldsCount++;
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

//#endregion

// GET
exports.getDefenseProfiles = async (req, res) => {
  config.progressLog(
    config.COLORS.cyan,
    "\n[PROGRESS] : Get Defense Profiles ...",
    config.COLORS.reset
  );
  try {
    const offset = req.query.offset ? parseInt(req.query.offset) : 0;
    if (offset === 0) {
      const defenseProfiles = await getAllDefenseProfiles();
      return res.status(200).json({
        data: defenseProfiles,
        progress: "Founded many defense profiles",
      });
    } else {
      const defenseProfiles = await getDefenseProfilesbyOffset(offset);
      return res.status(200).json({
        data: defenseProfiles,
        progress: "Founded many defense profiles",
      });
    }
  } catch (error) {
    console.log("error ", error);
    return res.status(500).json({
      message: "Get all defense profiles failed",
      progress: "Get all defense profiles failed",
    });
  }
};
exports.getAllDefenseProfilesByActive = async (req, res) => {
  progressLog(
    config.COLORS.cyan,
    "\n[PROGRESS] : Get Defense Profiles By Active ...",
    config.COLORS.reset
  );
  try {
    const defenseProfiles = await getActiveDefenseProfile();
    return res.status(200).json({ data: defenseProfiles });
  } catch (error) {
    console.log("error ", error);
    return res
      .status(500)
      .json({ message: "Get all active defense profiles failed" });
  }
};
exports.getDefenseProfileAttackTypesConfig = async (req, res) => {
  progressLog(
    config.COLORS.cyan,
    "\n[PROGRESS] : Get Defense Profile Attack Type " +
      req.params.attackType +
      " Config ...",
    config.COLORS.reset
  );
  try {
    const attackType = req.params.attackType;
    const attackTypeConfig = await getAttackTypeOfDefenseProfile(attackType);
    if (!attackTypeConfig) {
      return res.status(404).json({
        message: `No active defense profile found for attack type: ${attackType}`,
      });
    }
    return res.status(200).json({ data: attackTypeConfig });
  } catch (error) {
    console.log("error ", error);
    return res
      .status(500)
      .json({ message: "Get all active defense profiles failed" });
  }
};
exports.getAllThresholdByActiveDefenseProfile = async (req, res) => {
  progressLog(
    config.COLORS.cyan,
    "\n[PROGRESS] : Get All Threshold By Active Defense Profile ...",
    config.COLORS.reset
  );
  try {
    const profileAllThreshold = await getAllThresholdByActiveDefenseProfile();

    // Check if profileAllThreshold is undefined
    if (!profileAllThreshold) {
      return res
        .status(404)
        .json({ message: "No active defense profile found" });
    }

    // Calculate rate values
    profileAllThreshold.SYNFloodRate =
      profileAllThreshold.SYNFloodSYNThreshold /
      profileAllThreshold.DetectionTime;
    profileAllThreshold.DNSFloodRate =
      profileAllThreshold.DNSFloodThreshold / profileAllThreshold.DetectionTime;
    profileAllThreshold.IPSecFloodRate =
      profileAllThreshold.IPSecIKEThreshold / profileAllThreshold.DetectionTime;

    return res.status(200).json({ data: profileAllThreshold });
  } catch (error) {
    console.log("error ", error);
    return res
      .status(500)
      .json({ message: "Get all active defense profiles failed" });
  }
};
// OTHER
exports.insertDefenseProfile = async (req, res) => {
  config.progressLog(
    config.COLORS.cyan,
    "\n[PROGRESS] : insert Defense Profile ...",
    config.COLORS.reset
  );
  let progressReport = "";
  try {
    const userId = req.user.payload.Id;
    const currentDate = new Date();
    const defenseProfileData = req.body;

    if (await checkDuplicateName(defenseProfileData.DefenseProfileName)) {
      await insertSystemLogToDatabase(
        userId,
        "DefenseProfile",
        "Config",
        "Create Defense Profiles",
        `Create defense profile: ${defenseProfileData.DefenseProfileName}`,
        "Failed",
        "Profile name already exists"
      );
      return res.status(400).json({
        message: "Profile name already exists",
        progress: progressReport,
      });
    }

    // Validate input fields
    for (let key in defenseProfileData) {
      if (
        defenseProfileData[key] == null ||
        defenseProfileData[key] === undefined
      ) {
        return res.status(400).json({ message: `Missing field: ${key}` });
      }
    }

    // Prepare defense profile data
    defenseProfileData.UserId = userId;
    defenseProfileData.DefenseProfileCreateTime = format(
      currentDate,
      "yyyy/MM/dd HH:mm:ss",
      { locale: vi }
    );
    defenseProfileData.DefenseProfileStatus = "Inactive";
    defenseProfileData.DefenseProfileType = "UserProfile";
    defenseProfileData.DefenseProfileLastModified = format(
      currentDate,
      "yyyy/MM/dd HH:mm:ss",
      { locale: vi }
    );
    defenseProfileData.DefenseProfileUsingTime = null;

    // Build full config package for the new profile
    const { configPackage, fieldsCount } = buildFullConfigPackage(
      defenseProfileData,
      "PORT1"
    );

    if (!configPackage) {
      return res.status(400).json({
        message: "No configuration generated",
        progress: progressReport,
      });
    }

    // Check connection to C program
    const isMainCConnected = await checkMainCConnection();
    if (!isMainCConnected) {
      await insertSystemLogToDatabase(
        userId,
        "DefenseProfile",
        "Config",
        "Create Defense Profiles",
        `Create defense profile: ${defenseProfileData.DefenseProfileName}`,
        "Failed",
        "Cannot connect to C program"
      );
      return res.status(500).json({ message: "Cannot connect to C program" });
    }

    // Send command to C program
    const timeout = Math.max(fieldsCount * 2 * 1000, 5000); // Minimum 5 seconds
    config.progressLog(
      config.COLORS.spacing,
      `Set timeout for command: `,
      config.COLORS.blue,
      `${timeout / 1000} seconds`,
      config.COLORS.reset
    );
    const result = await sendCommandToCProgram(configPackage, timeout);

    const isError = config.isProduction
      ? result.includes("ERROR")
      : result.includes("ERROR");
    if (isError) {
      await insertSystemLogToDatabase(
        userId,
        "DefenseProfile",
        "Config",
        "Create Defense Profiles",
        `Create defense profile: ${defenseProfileData.DefenseProfileName}`,
        "Failed",
        `C error: ${result}`
      );
      return res.status(500).json({
        message: "Failed to create defense profile",
        error: result,
        progress: progressReport,
      });
    } else {
      progressReport += "mainC updated";
    }

    // If mainC is successful, insert into database
    const profileId = await insertDefenseProfile(defenseProfileData);
    progressReport += " database updated";
    // Log success to the system
    await insertSystemLogToDatabase(
      userId,
      "DefenseProfile",
      "Config",
      "Create Defense Profiles",
      `Create defense profile: ${defenseProfileData.DefenseProfileName}`,
      "Success",
      null
    );

    return res.status(200).json({
      message: "Insert defense profile success",
      data: profileId,
      progress: progressReport,
    });
  } catch (error) {
    console.log("error ", error);
    await insertSystemLogToDatabase(
      req.user.payload.Id,
      "DefenseProfile",
      "Config",
      "Create Defense Profiles",
      `Create defense profile`,
      "Failed",
      error.message
    );
    return res.status(500).json({
      message: "Insert defense profile failed",
      error: error.message,
      progress: progressReport,
    });
  }
};
/**
 * FIXED: mainC not updated
 * MAIN CAUSE: filter logic works wrongly
 */
exports.updateDefenseProfile = async (req, res) => {
  config.progressLog(
    config.COLORS.cyan,
    "\n[PROGRESS] : Update Defense Profile ...",
    config.COLORS.reset
  );
  const { profileId } = req.params;
  const updatedData = req.body;
  const userId = req.user?.payload?.Id || null;
  let progressReport = "";
  let totalProcessingTimeInSeconds = 0;
  try {
    const currentProfile = await getDefenseProfilebyDefenseProfileId(profileId);
    if (!currentProfile) {
      await insertSystemLogToDatabase(
        userId,
        "DefenseProfile",
        "Config",
        "Update Defense Profile",
        `Failed to update profile: ${profileId}`,
        "Failed",
        "Defense profile not found"
      );
      return res.status(404).json({
        success: false,
        message: "Defense profile not found",
        progress: progressReport,
      });
    }

    if (
      updatedData.DefenseProfileName &&
      updatedData.DefenseProfileName !== currentProfile.DefenseProfileName
    ) {
      const profiles = await getAllDefenseProfiles();
      if (
        profiles.some(
          (profile) =>
            profile.DefenseProfileName.toLowerCase() ===
              updatedData.DefenseProfileName.toLowerCase() &&
            profile.DefenseProfileId !== profileId
        )
      ) {
        await insertSystemLogToDatabase(
          userId,
          "DefenseProfile",
          "Config",
          "Update Defense Profile",
          `Failed to update profile: ${updatedData.DefenseProfileName}`,
          "Failed",
          "Profile name already exists"
        );
        return res.status(400).json({
          success: false,
          message: "Profile name already exists",
          progress: progressReport,
        });
      }
    }

    const mergedData = {
      ...currentProfile,
      ...updatedData,
      DefenseProfileLastModified: format(new Date(), "yyyy/MM/dd HH:mm:ss", {
        locale: vi,
      }),
    };

    const interfaces = await getAllDeviceInterfaces();
    const usingInterfaces = interfaces.filter((i) => {
      return String(i.DefenseProfileId) === String(profileId);
    });

    if (usingInterfaces.length === 0) {
      progressLog(
        COLORS.spacing,
        `No interfaces found for profile ${profileId}, updating database only`,
        COLORS.reset
      );
      await updateDefenseProfile(profileId, mergedData);
      progressReport += `database updated`;

      await insertSystemLogToDatabase(
        userId,
        "DefenseProfile",
        "Config",
        "Update Defense Profile",
        `Updated profile: ${mergedData.DefenseProfileName}`,
        "Success",
        "No interfaces to update"
      );

      return res.status(200).json({
        success: true,
        message:
          "Defense profile updated successfully (no interfaces to update)",
        progress: progressReport,
        estimatedTime: 0,
        data: { profileId, changes: Object.keys(updatedData) },
      });
    }

    // Kiểm tra kết nối tới mainC
    const isMainCConnected = await checkMainCConnection();
    if (!isMainCConnected) {
      await insertSystemLogToDatabase(
        userId,
        "DefenseProfile",
        "Config",
        "Update Defense Profile",
        `Failed to update profile: ${profileId}`,
        "Failed",
        "Cannot connect to C program"
      );
      return res.status(500).json({
        success: false,
        message: "Cannot connect to C program",
        progress: progressReport,
      });
    }

    // Gửi lệnh mainC cho tất cả interfaces
    const configPackages = [];
    for (const iface of usingInterfaces) {
      const { config, differentFieldsCount } = buildConfigPackageFromDevice(
        iface,
        mergedData,
        false, // Không cần includeProtect vì chỉ cập nhật profile
        currentProfile
      );
      if (config) {
        configPackages.push({ config, differentFieldsCount });
        totalProcessingTimeInSeconds += differentFieldsCount * 2;
      } else {
        progressLog(
          COLORS.spacing,
          `No config changes for interface ${iface.InterfaceName}`,
          COLORS.reset
        );
      }
    }

    // Hiển thị tổng thời gian xử lý
    progressLog(
      COLORS.spacing,
      `Total time to process for all ports: `,
      COLORS.blue,
      `${totalProcessingTimeInSeconds} seconds`,
      COLORS.reset
    );

    if (configPackages.length === 0) {
      progressLog(
        COLORS.spacing,
        `No changes detected, updating database only`,
        COLORS.reset
      );
      await updateDefenseProfile(profileId, mergedData);
      progressReport += `database updated`;

      await insertSystemLogToDatabase(
        userId,
        "DefenseProfile",
        "Config",
        "Update Defense Profile",
        `Updated profile: ${mergedData.DefenseProfileName}`,
        "Success",
        "No changes to send to mainC"
      );

      return res.status(200).json({
        success: true,
        message: "Defense profile updated successfully (no changes to mainC)",
        progress: progressReport,
        estimatedTime: 0,
        data: { profileId, changes: Object.keys(updatedData) },
      });
    }

    // Gửi từng config package tới mainC
    for (const { config, differentFieldsCount } of configPackages) {
      const timeout = Math.max(differentFieldsCount * 2 * 1000, 5000);
      const result = await sendCommandToCProgram(config, timeout);
      const isError = config.isProduction
        ? result.includes("ERROR")
        : result.includes("ERROR");

      if (isError) {
        await insertSystemLogToDatabase(
          userId,
          "DefenseProfile",
          "Config",
          "Update Defense Profile",
          `Failed to update profile: ${profileId}`,
          "Failed",
          `C error: ${result}`
        );
        return res.status(500).json({
          success: false,
          message: `Failed to update profile`,
          error: result,
          progress: progressReport,
        });
      }
    }

    // Nếu tất cả mainC OK thì cập nhật DB
    progressReport += `mainC updated`;
    await updateDefenseProfile(profileId, mergedData);
    progressReport += ` database updated`;

    await insertSystemLogToDatabase(
      userId,
      "DefenseProfile",
      "Config",
      "Update Defense Profile",
      `Updated profile: ${mergedData.DefenseProfileName}`,
      "Success",
      null
    );

    return res.status(200).json({
      success: true,
      message: "Defense profile updated successfully",
      progress: progressReport,
      estimatedTime: totalProcessingTimeInSeconds,
      data: { profileId, changes: Object.keys(updatedData) },
    });
  } catch (error) {
    console.error("Update defense profile error:", error);
    await insertSystemLogToDatabase(
      userId,
      "DefenseProfile",
      "Config",
      "Update Defense Profile",
      `Failed to update profile: ${profileId}`,
      "Failed",
      error.message
    );
    return res.status(500).json({
      success: false,
      message: "Failed to update defense profile",
      error: error.message,
      progress: progressReport,
    });
  }
};
exports.applyDefenseProfileToInterfaces = async (req, res) => {
  config.progressLog(
    config.COLORS.cyan,
    "\n[PROGRESS] : apply Defense Profile To Interfaces ...",
    config.COLORS.reset
  );
  let progressReport = "";
  try {
    const profileId = req.params.profileId;
    const interfaces = req.body;

    if (!Array.isArray(interfaces) || interfaces.length === 0) {
      return res.status(400).json({
        message: "Invalid or empty interfaces array",
        progress: progressReport,
      });
    }

    const newProfile = await getDefenseProfilebyDefenseProfileId(profileId);
    if (!newProfile) {
      return res.status(404).json({
        message: "Defense profile not found",
        progress: progressReport,
      });
    }

    const allInterfaces = await getAllDeviceInterfaces();
    const configPackages = [];
    let totalDifferentFields = 0;

    for (const ifaceData of interfaces) {
      const { name, date } = ifaceData;
      const iface = allInterfaces.find((i) => i.InterfaceName === name);
      if (!iface) {
        return res.status(404).json({
          message: `Interface ${name} not found`,
          progress: progressReport,
        });
      }

      let portName = name.toLowerCase();
      if (portName.startsWith("eth")) {
        portName = "PORT" + portName.replace("eth", "");
      }

      const currentConfig = iface.DefenseProfileId
        ? await getDefenseProfilebyDefenseProfileId(iface.DefenseProfileId)
        : {};
      const result = buildConfigPackage(
        newProfile,
        currentConfig || {},
        portName
      );

      if (result.configPackage) {
        configPackages.push({
          configPackage: result.configPackage,
          differentFieldsCount: result.differentFieldsCount,
        });
        totalDifferentFields += result.differentFieldsCount;
      }
    }

    // Hiển thị thời gian xử lý cho tất cả các port
    const totalProcessingTimeInSeconds = totalDifferentFields * 2;
    config.progressLog(
      config.COLORS.spacing,
      `Total time to process for all ports: `,
      config.COLORS.blue,
      `${totalProcessingTimeInSeconds} seconds`,
      config.COLORS.reset
    );

    // Kiểm tra kết nối tới chương trình C
    const isMainCConnected = await checkMainCConnection();
    if (!isMainCConnected) {
      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DefenseProfile",
        "Config",
        "Apply Defense Profiles to Interfaces",
        `Apply profile ${profileId}`,
        "Failed",
        "Cannot connect to C program"
      );
      return res.status(500).json({
        message: "Cannot connect to C program",
        progress: progressReport,
      });
    }

    // Gửi từng configPackage riêng lẻ
    progressReport += `mainC updated`;
    for (const { configPackage, differentFieldsCount } of configPackages) {
      config.progressLog(
        config.COLORS.spacing,
        "Command sent to C program: ",
        config.COLORS.magenta,
        configPackage,
        config.COLORS.reset
      );
      const timeout = Math.max(differentFieldsCount * 2 * 1000, 5000); // Tối thiểu 5 giây
      config.progressLog(
        config.COLORS.spacing,
        `Set timeout for command: `,
        config.COLORS.blue,
        `${timeout / 1000} seconds`,
        config.COLORS.reset
      );
      const result = await sendCommandToCProgram(configPackage, timeout);

      const isError = config.isProduction
        ? result.includes("ERROR")
        : result.includes("ERROR");

      if (isError) {
        await insertSystemLogToDatabase(
          req.user.payload.Id,
          "DefenseProfile",
          "Config",
          "Apply Defense Profiles to Interfaces",
          `Apply profile ${profileId}`,
          "Failed",
          `C error: ${result}`
        );
        progressReport = "";
        return res.status(500).json({
          message: "Failed to apply defense profile to interfaces",
          error: result,
          progress: progressReport,
        });
      }
    }

    // Đến đây mới update DB
    for (const ifaceData of interfaces) {
      const iface = allInterfaces.find(
        (i) => i.InterfaceName === ifaceData.name
      );
      await updateDeviceInterface(iface.InterfaceId, {
        ...iface,
        DefenseProfileId: profileId,
      });
    }

    let newUsingTime = newProfile.DefenseProfileUsingTime
      ? JSON.parse(newProfile.DefenseProfileUsingTime)
      : [];
    for (const ifaceData of interfaces) {
      if (!newUsingTime.some((entry) => entry.name === ifaceData.name)) {
        newUsingTime.push({ name: ifaceData.name, date: ifaceData.date });
      }
    }
    newProfile.DefenseProfileUsingTime =
      newUsingTime.length > 0 ? JSON.stringify(newUsingTime) : null;
    newProfile.DefenseProfileLastModified = format(
      new Date(),
      "yyyy/MM/dd HH:mm:ss",
      { locale: vi }
    );
    newProfile.DefenseProfileStatus = "Active";
    await updateDefenseProfile(profileId, newProfile);
    progressReport += `database updated`;

    await insertSystemLogToDatabase(
      req.user.payload.Id,
      "DefenseProfile",
      "Config",
      "Apply Defense Profiles to Interfaces",
      `Applied profile ${profileId} to interfaces: ${interfaces
        .map((i) => i.name)
        .join(", ")}`,
      "Success",
      null
    );

    return res.status(200).json({
      message: "Defense profile applied to interfaces successfully",
      progress: progressReport,
    });
  } catch (error) {
    console.log("error:", error);
    await insertSystemLogToDatabase(
      req.user.payload.Id,
      "DefenseProfile",
      "Config",
      "Apply Defense Profiles to Interfaces",
      `Apply profile ${req.params.profileId}`,
      "Failed",
      error.message
    );
    return res.status(500).json({
      message: "Failed to apply defense profile to interfaces",
      error: error.message,
      progress: progressReport,
    });
  }
};
exports.deleteDefenseProfile = async (req, res) => {
  config.progressLog(
    config.COLORS.cyan,
    "\n[PROGRESS] : delete Defense Profile ...",
    config.COLORS.reset
  );
  let progressReport = "";
  try {
    const profileId = req.params.profileId;
    const defenseProfileData = await getDefenseProfilebyDefenseProfileId(
      profileId
    );
    if (!defenseProfileData) {
      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DefenseProfile",
        "Config",
        "Delete Defense Profiles",
        `Delete defense profile: ${defenseProfileData.DefenseProfileName}`,
        "Failed",
        "Defense profile not found"
      );
      return res.status(404).json({
        message: "Defense profile not found",
        progress: progressReport,
      });
    }
    if (defenseProfileData.DefenseProfileStatus === "Active") {
      await insertSystemLogToDatabase(
        req.user.payload.Id,
        "DefenseProfile",
        "Config",
        "Delete Defense Profiles",
        `Delete defense profile: ${defenseProfileData.DefenseProfileName}`,
        "Failed",
        "Defense profile is currently active. Please deactivate it before deleting."
      );
      return res.status(400).json({
        message:
          "Defense profile is currently active. Please deactivate it before deleting.",
        progress: progressReport,
      });
    }
    await deleteDefenseProfile(profileId);
    progressReport += "database updated";

    await insertSystemLogToDatabase(
      req.user.payload.Id,
      "DefenseProfile",
      "Config",
      "Delete Defense Profiles",
      `Delete defense profile: ${defenseProfileData.DefenseProfileName}`,
      "Success",
      null
    );
    return res.status(200).json({
      message: "Delete defense profile success",
      progress: progressReport,
    });
  } catch (error) {
    console.log("error ", error);
    await insertSystemLogToDatabase(
      req.user.payload.Id,
      "DefenseProfile",
      "Config",
      "Delete Defense Profiles",
      `Delete defense profile`,
      "Failed",
      error.message
    );
    return res.status(500).json({
      message: "Delete defense profile failed",
      progress: progressReport,
    });
  }
};
