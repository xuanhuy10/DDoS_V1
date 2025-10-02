const { format } = require("date-fns");
const { vi } = require("date-fns/locale");
const config = require("../config/index");
const { insertSystemLogToDatabase } = require("../helper/dbo/logs.helper");
const { sendCommandToCProgram } = require("../services/socket.service");
const { getAllDeviceInterfaces } = require("../models/DeviceInterfaces.model");
const { v4: uuidv4 } = require("uuid");
const {
  getIpSecProfiles,
  getIpSecProfilesByOffset,
  getIpSecProfileById,
  insertIpSecProfile,
  updateIpSecProfile,
  deleteIpSecProfile,
} = require("../models/IPSecProfiles.model");

const path = require("path");
const fs = require("fs");
const isFullPackage = true;

exports.getIpSecProfiles = async (req, res) => {
  config.progressLog(
      config.COLORS.cyan,
      "\n[PROGRESS] : Get IPSec Profiles ...",
      config.COLORS.reset
  );
  try {
    const offset = req.params.offset ? parseInt(req.params.offset) : 0;
    let profiles;
    if (offset === 0) {
      profiles = await getIpSecProfiles();
    } else {
      profiles = await getIpSecProfilesByOffset(offset);
    }

    // Parse comma-separated values thành array cho frontend (tùy chọn)
    profiles.forEach(profile => {
      if (profile.RemoteGateway) {
        profile.RemoteGatewayArray = profile.RemoteGateway.split(',').map(ip => ip.trim());
      }
      if (profile.SubnetRemoteGateway) {
        profile.SubnetRemoteGatewayArray = profile.SubnetRemoteGateway.split(',').map(subnet => subnet.trim());
      }
    });

    return res.status(200).json({ data: profiles });
  } catch (error) {
    console.log("error ", error);
    return res.status(500).json({ message: "Get all IPSec profiles failed" });
  }
};

exports.insertIpSecProfileAndCertificates = async (req, res) => {
  config.progressLog(config.COLORS.cyan, "\n[PROGRESS] : insert IpSec Profile and Certificates ...", config.COLORS.reset);
  let progressReport = "";
  try {
    if (!req.user || !req.user.payload || !req.user.payload.Id) {
      throw new Error(
          "User authentication failed: Invalid or missing user data"
      );
    }

    const userId = req.user.payload.Id;
    const currentDate = new Date();
    const files = req.files;
    const profileData = JSON.parse(req.body.profileData);

    console.log("profileData ", profileData);

    const requiredFiles = ["ca-cert", "cert", "private-key"];
    for (const key of requiredFiles) {
      if (!files[key] || !files[key][0]) {
        throw new Error(`Missing required file: ${key}`);
      }
    }

    if (!profileData.ProfileName) {
      return res.status(400).json({ message: "ProfileName is required" });
    }

    const existingProfile = await getIpSecProfiles();
    if (
        existingProfile.some(
            (profile) => profile.ProfileName === profileData.ProfileName
        )
    ) {
      return res.status(400).json({ message: "ProfileName already exists" });
    }

    // Xử lý multiple remote gateways và subnets (fallback nếu cần)
    const connectionCount = profileData.ConnectionCount || 1;
    profileData.ConnectionCount = connectionCount;

    // Không cần delete fields, giữ nguyên RemoteGateway và SubnetRemoteGateway từ frontend
    // (đã là comma-separated nếu ConnectionCount > 1)

    const uniqueKey = uuidv4();
    const baseUploadDir = config.packet.ipSecCertificatePath;
    const subDirs = {
      cacerts: path.join(baseUploadDir, "cacerts"),
      certs: path.join(baseUploadDir, "certs"),
      private: path.join(baseUploadDir, "private"),
    };

    // Mapping from file key to subdir key
    const dirMap = {
      "ca-cert": "cacerts",
      cert: "certs",
      "private-key": "private",
    };

    await Promise.all(
        Object.values(subDirs).map((dir) =>
            fs.promises.mkdir(dir, { recursive: true })
        )
    );

    const savedFiles = {};
    await Promise.all(
        requiredFiles.map(async (key) => {
          const file = files[key][0];
          const fileExtension = path.extname(file.originalname) || ".der";
          const fileName = `${key}-${uniqueKey}${fileExtension}`;
          const subDirKey = dirMap[key];
          const filePath = path.join(subDirs[subDirKey], fileName);
          await fs.promises.writeFile(filePath, file.buffer);
          savedFiles[key] = {
            path: filePath,
            originalName: file.originalname,
            savedName: fileName,
          };
        })
    );

    profileData.UserId = userId;
    profileData.CreateTime = format(currentDate, "yyyy/MM/dd HH:mm:ss", {
      locale: vi,
    });
    profileData.LastModified = format(currentDate, "yyyy/MM/dd HH:mm:ss", {
      locale: vi,
    });
    profileData.Enable = profileData.Enable ? 1 : 0;

    let configPackage = "";
    let commandCount = 0;

    if (isFullPackage) {
      requiredFiles.forEach((key) => {
        configPackage += `IPS_ADD$${savedFiles[key].savedName}$`;
        commandCount++;
      });
    }

    const result = await sendCommandToCProgram(configPackage);

    if (config.isProduction) {
      const tokens = result.split("$").filter(Boolean);
      let hasError = false;
      let errorDetails = [];
      for (let i = 0; i < tokens.length; i++) {
        if (tokens[i] === "ERROR") {
          hasError = true;
          errorDetails.push(tokens[i - 1] || "UnknownParam");
        }
      }
      if (hasError) {
        throw new Error(
            `Failed to apply config. Errors at: ${errorDetails.join(
                ", "
            )} | Raw response: ${result}`
        );
      } else {
        config.progressLog(
            config.COLORS.reset,
            ` Response: ${result}`,
            config.COLORS.reset
        );
      }
    } else {
      if (result.toUpperCase().includes("ERROR")) {
        return res.status(500).json({
          message: "Insert IPSec profile and certificates failed",
          error: result,
        });
      }
    }

    const profileId = await insertIpSecProfile(profileData);

    await insertSystemLogToDatabase(
        userId,
        "IPSecProfileAndCertificate",
        "Config",
        "Create IPSec Profile and Certificates",
        `Created IPSec profile: ${profileData.ProfileName}, Certificates: ca-cert=${savedFiles["ca-cert"].originalName}, cert=${savedFiles["cert"].originalName}, private-key=${savedFiles["private-key"].originalName}`,
        "Success",
        null
    );

    return res.status(200).json({
      message: "Insert IPSec profile and certificates success",
      data: { profileId },
    });
  } catch (error) {
    console.error("error:", error);
    await insertSystemLogToDatabase(
        req.user?.payload?.Id || "unknown",
        "IPSecProfileAndCertificate",
        "Config",
        "Create IPSec Profile and Certificates",
        "Failed to create IPSec profile and certificates",
        "Failed",
        error.message
    );
    return res.status(500).json({
      message: "Insert IPSec profile and certificates failed",
      error: error.message,
    });
  }
};

exports.updateIpSecProfile = async (req, res) => {
  config.progressLog(config.COLORS.cyan, "\n[PROGRESS] : update IPSec Profile ...", config.COLORS.reset);
  const { profileId } = req.params;
  const updatedData = req.body;
  const userId = req.user?.payload?.Id || null;
  let progressReport = "";

  console.log("profile id : ", profileId);
  console.log("updated data : ", updatedData);

  try {
    const currentProfile = await getIpSecProfileById(profileId);
    if (!currentProfile) {
      return res.status(404).json({success: false, message: "IPSec profile not found", progress: progressReport,});
    }

    if (
        updatedData.ProfileName &&
        updatedData.ProfileName !== currentProfile.ProfileName
    ) {
      const existingProfile = await getIpSecProfiles();
      if (
          existingProfile.some(
              (profile) =>
                  profile.ProfileName === updatedData.ProfileName &&
                  profile.IPSecProfileId !== parseInt(profileId)
          )
      ) {
        return res.status(400).json({
          success: false,
          message: "ProfileName already exists",
          progress: progressReport,
        });
      }
    }

    // Xử lý multiple remote gateways và subnets khi update
    const connectionCount = updatedData.ConnectionCount || currentProfile.ConnectionCount || 1;
    if (connectionCount > 1) {
      updatedData.RemoteGateway = updatedData.RemoteGateways || currentProfile.RemoteGateway || null;
      updatedData.SubnetRemoteGateway = updatedData.SubnetRemoteGateways || currentProfile.SubnetRemoteGateway || null;
    } else {
      updatedData.RemoteGateway = updatedData.RemoteGateway || currentProfile.RemoteGateway || null;
      updatedData.SubnetRemoteGateway = updatedData.SubnetRemoteGateway || currentProfile.SubnetRemoteGateway || null;
    }
    updatedData.ConnectionCount = connectionCount;

    // Xóa fields tạm
    delete updatedData.RemoteGateways;
    delete updatedData.SubnetRemoteGateways;

    const { configPackage, commandCount } = await buildConfigPackage(
        updatedData,
        currentProfile,
        false
    );

    if (configPackage) {
      const result = await sendCommandToCProgram(configPackage);

      if (config.isProduction) {
        const tokens = result.split("$").filter(Boolean);
        let hasError = false;
        let errorDetails = [];
        for (let i = 0; i < tokens.length; i++) {
          if (tokens[i] === "ERROR") {
            hasError = true;
            errorDetails.push(tokens[i - 1] || "UnknownParam");
          }
        }
        if (hasError) {
          return res.status(500).json({
            success: false,
            message: "Failed to update IPSec profile",
            error: `Errors at: ${errorDetails.join(", ")} | Raw: ${result}`,
            progress: progressReport,
          });
        } else {
          progressReport = `mainC updated`;
        }
      } else {
        if (result.toUpperCase().includes("ERROR")) {
          return res.status(500).json({
            success: false,
            message: "Failed to update IPSec profile",
            error: result,
            progress: progressReport,
          });
        } else {
          progressReport += `mainC updated`;
        }
      }
    }

    await updateIpSecProfile(profileId, {
      ...currentProfile,
      ...updatedData,
      LastModified: format(new Date(), "yyyy/MM/dd HH:mm:ss", { locale: vi }),
    });
    progressReport += `database updated`;

    return res.status(200).json({
      success: true,
      message: "IPSec profile updated successfully",
      progress: progressReport,
    });
  } catch (error) {
    console.error("Update IPSec profile error:", error);
    return res.status(500).json({
      success: false,
      message: "Failed to update IPSec profile",
      error: error.message,
    });
  }
};

exports.deleteIpSecProfile = async (req, res) => {
  config.progressLog(
      config.COLORS.cyan,
      "\n[PROGRESS] : Delete IPSec Profiles ...",
      config.COLORS.reset
  );
  let progressReport = "";
  try {
    const profileId = req.params.profileId;
    const ipSecProfileData = await getIpSecProfileById(profileId);
    if (!ipSecProfileData) {
      await insertSystemLogToDatabase(
          req.user.payload.Id,
          "IPSecProfile",
          "Config",
          "Delete IPSec Profile",
          `Delete IPSec profile: ${ipSecProfileData.ProfileName}`,
          "Failed",
          "IPSec profile not found"
      );
      return res.status(404).json({ message: "IPSec profile not found" });
    }
    if (ipSecProfileData.Status === "Active") {
      await insertSystemLogToDatabase(
          req.user.payload.Id,
          "IPSecProfile",
          "Config",
          "Delete IPSec Profile",
          `Delete IPSec profile: ${ipSecProfileData.ProfileName}`,
          "Failed",
          "IPSec profile is currently active. Please deactivate it before deleting."
      );
      return res.status(400).json({
        message:
            "IPSec profile is currently active. Please deactivate it before deleting.",
      });
    }
    await deleteIpSecProfile(profileId);
    progressReport += "database updated";
    await insertSystemLogToDatabase(
        req.user.payload.Id,
        "IPSecProfile",
        "Config",
        "Delete IPSec Profile",
        `Delete IPSec profile: ${ipSecProfileData.ProfileName}`,
        "Success",
        null
    );
    return res.status(200).json({ message: "Delete IPSec profile success" });
  } catch (error) {
    console.log("error ", error);
    await insertSystemLogToDatabase(
        req.user.payload.Id,
        "IPSecProfile",
        "Config",
        "Delete IPSec Profile",
        `Delete IPSec profile`,
        "Failed",
        error.message
    );
    return res.status(500).json({ message: "Delete IPSec profile failed" });
  }
};

//#region -------------------- HELPER --------------------

const buildConfigPackage = async (
    newConfig,
    currentConfig = {},
    isInsert = false
) => {
  let configPackage = "";
  let commandCount = 0;

  const ipSecFields = {
    Enable: "IPS_EN_DIS",
    LocalGateway: "IPS_LOCAL_IP",
    SubnetLocalGateway: "IPS_LOCAL_SUBNET",
    RemoteGateway: "IPS_REMOTE_IP",
    SubnetRemoteGateway: "IPS_REMOTE_SUBNET",
    IKEVersion: "IPS_IKE_VERSION",
    Mode: "IPS_MODE",
    ESPAHProtocol: "IPS_PROTOCOL",
    IKEReauthTime: "IPS_IKE_TIME",
    EncryptionAlgorithm: "IPS_ENCRY",
    HashAlgorithm: "IPS_HASH",
    ReKeyTime: "IPS_KEY",
    ConnectionCount: "IPS_CONNECTION_COUNT",
  };

  const normalizedCurrentConfig = {};
  Object.keys(newConfig).forEach((key) => {
    if (key in ipSecFields) {
      normalizedCurrentConfig[key] =
          currentConfig[key] !== undefined ? currentConfig[key] : null;
    }
  });

  for (const [field, command] of Object.entries(ipSecFields)) {
    if (newConfig[field] !== undefined) {
      let newValue =
          field === "Enable" ? (newConfig[field] ? 1 : 0) : newConfig[field];
      let currentValue =
          field === "Enable"
              ? normalizedCurrentConfig[field] !== undefined
                  ? normalizedCurrentConfig[field]
                      ? 1
                      : 0
                  : 0
              : normalizedCurrentConfig[field];

      // Logic đặc biệt cho RemoteGateway và SubnetRemoteGateway (hỗ trợ multiple comma-separated)
      if (field === 'RemoteGateway' || field === 'SubnetRemoteGateway') {
        const isMultiple = newValue && typeof newValue === 'string' && newValue.includes(',');
        const valueToSend = isMultiple ? newValue : (newValue !== null ? newValue : "null");
        const currentValueToCompare = normalizedCurrentConfig[field];

        if (isInsert || valueToSend !== currentValueToCompare) {
          configPackage += `${command}$${valueToSend}$`;
          commandCount++;
        }
        continue; // Skip logic cũ
      }

      if (isInsert || newValue !== currentValue) {
        configPackage += `${command}$${newValue !== null ? newValue : "null"}$`;
        commandCount++;
      }
    }
  }

  const totalProcessingTimeInSeconds = commandCount * 2;
  config.progressLog(
      config.COLORS.spacing,
      config.COLORS.blue,
      `${totalProcessingTimeInSeconds} seconds`,
      config.COLORS.reset
  );
  return { configPackage, commandCount };
};

//#endregion