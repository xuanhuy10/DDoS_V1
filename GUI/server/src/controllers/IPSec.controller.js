const path = require("path");
const fs = require("fs");
const { format } = require("date-fns");
const { vi } = require("date-fns/locale");
const config = require("../config/index");
const { insertSystemLogToDatabase } = require("../helper/dbo/logs.helper");
const { sendCommandToCProgram } = require("../services/socket.service");
const { getIpSecProfiles, getIpSecProfilesByOffset, getIpSecProfileById, insertIpSecProfile, updateIpSecProfile, deleteIpSecProfile } = require("../models/IPSecProfiles.model");

/**
 * Lấy danh sách IPSec Profiles
 *
 * Lấy tất cả hoặc một phần danh sách IPSec Profiles dựa trên offset (phân trang).
 * Chuyển đổi RemoteGateway và SubnetRemoteGateway thành mảng nếu có giá trị dạng chuỗi phân tách bởi dấu phẩy.
 * @param {object} req - Yêu cầu HTTP chứa thông tin offset (nếu có).
 * @param {object} res - Phản hồi HTTP trả về danh sách profiles.
 */
exports.getIpSecProfiles = async (req, res) => {
  config.progressLog(config.COLORS.cyan, "\n[PROGRESS] : Get IPSec Profiles ...", config.COLORS.reset);
  try {
    const offset = req.params.offset ? parseInt(req.params.offset) : 0;
    let profiles;
    if (offset === 0) {
      profiles = await getIpSecProfiles();
    } else {
      profiles = await getIpSecProfilesByOffset(offset);
    }
    profiles.forEach((profile) => {
      if (profile.RemoteGateway) {
        profile.RemoteGatewayArray = profile.RemoteGateway.split(",").map((ip) => ip.trim());
      }
      if (profile.SubnetRemoteGateway) {
        profile.SubnetRemoteGatewayArray = profile.SubnetRemoteGateway.split(",").map((subnet) => subnet.trim());
      }
    });
    return res.status(200).json({ data: profiles });
  } catch (error) {
    console.log("error ", error);
    return res.status(500).json({ message: "Get all IPSec profiles failed" });
  }
};
/**
 * Thêm mới IPSec Profile và Certificates (Tùy chọn)
 *
 * - Không bắt buộc file chứng chỉ.
 * - Chỉ xử lý file nếu có.
 * - Chỉ dọn dẹp thư mục khi có file mới (giữ nguyên file cũ nếu không upload).
 * - Không gửi lệnh cert.
 * - Vẫn gửi full config package cho profile.
 *
 * @param {object} req
 * @param {object} res
 * @returns {Promise<void>}
 */
exports.insertIpSecProfileAndCertificates = async (req, res) => {
  config.progressLog(config.COLORS.cyan, "\n[PROGRESS] : insert IpSec Profile (optional certs)...", config.COLORS.reset);
  let progressReport = "";
  try {
    // === 1. Validate user ===
    if (!req.user || !req.user.payload || !req.user.payload.Id) {
      return res.status(401).json({ message: "User authentication failed: Invalid or missing user data" });
    }
    const userId = req.user.payload.Id;
    const currentDate = new Date();

    // === 2. Parse profileData ===
    let profileData = {};
    try {
      profileData = JSON.parse(req.body.profileData || '{}');
    } catch (e) {
      return res.status(400).json({ message: "Invalid JSON in profileData" });
    }

    // === 3. Validate ProfileName ===
    if (!profileData.ProfileName) {
      return res.status(400).json({ message: "ProfileName is required" });
    }

    // === 4. Check duplicate ProfileName ===
    const existingProfile = await getIpSecProfiles();
    if (existingProfile.some(p => p.ProfileName === profileData.ProfileName)) {
      return res.status(400).json({ message: "ProfileName already exists" });
    }

    // === 5. Chuẩn bị thư mục & file ===
    const files = req.files || {}; // Không bắt buộc
    const hasFiles = Object.keys(files).length > 0;
    const baseUploadDir = config.packet.ipSecCertificatePath;
    const subDirs = {
      cacerts: path.join(baseUploadDir, "cacerts"),
      certs: path.join(baseUploadDir, "certs"),
      private: path.join(baseUploadDir, "private"),
    };
    const dirMap = { "ca-cert": "cacerts", cert: "certs", "private-key": "private" };
    const savedFiles = {};

    // Tạo thư mục (luôn luôn)
    await Promise.all(Object.values(subDirs).map(dir => fs.promises.mkdir(dir, { recursive: true })));

    // === 6. XỬ LÝ FILE CHỈ KHI CÓ FILE MỚI ===
    if (hasFiles) {
      const requiredFiles = ["ca-cert", "cert", "private-key"];
      const missingFiles = requiredFiles.filter(key => !files[key] || !files[key][0]);

      if (missingFiles.length > 0) {
        return res.status(400).json({
          message: `Missing required certificate files: ${missingFiles.join(", ")}`
        });
      }

      // === DỌN DẸP CHỈ KHI CÓ FILE MỚI ===
      const cleanupDirs = async () => {
        for (const [dirName, dirPath] of Object.entries(subDirs)) {
          try {
            const filesInDir = await fs.promises.readdir(dirPath);
            await Promise.all(
                filesInDir.map(async (file) => {
                  const filePath = path.join(dirPath, file);
                  const stat = await fs.promises.stat(filePath);
                  if (stat.isFile()) {
                    await fs.promises.unlink(filePath);
                  }
                })
            );
            config.progressLog(config.COLORS.green, `Cleaned ${filesInDir.length} files in ${dirName}`, config.COLORS.reset);
          } catch (err) {
            console.warn(`Warning: Failed to clean ${dirName}:`, err.message);
          }
        }
      };
      await cleanupDirs();
      progressReport += " old certs cleaned;";

      // === LƯU FILE MỚI ===
      await Promise.all(
          requiredFiles.map(async (key) => {
            const file = files[key][0];
            const fileName = file.originalname;
            const filePath = path.join(subDirs[dirMap[key]], fileName);
            await fs.promises.writeFile(filePath, file.buffer);
            savedFiles[key] = { originalName: file.originalname, savedName: fileName, path: filePath };
          })
      );
      progressReport += " new certs saved;";
    } else {
      progressReport += " no certs uploaded (kept existing);";
    }

    // === 7. Finalize profile data ===
    profileData.UserId = userId;
    profileData.ConnectionCount = profileData.ConnectionCount || 1;
    profileData.CreateTime = format(currentDate, "yyyy/MM/dd HH:mm:ss", { locale: vi });
    profileData.LastModified = format(currentDate, "yyyy/MM/dd HH:mm:ss", { locale: vi });
    profileData.Enable = profileData.Enable ? 1 : 0;

    // === 8. Build & gửi config package (chỉ profile) ===
    const { configPackage: profilePackage, commandCount } = await buildConfigPackage(profileData, {}, true);
    let configPackage = profilePackage;

    const result = await sendCommandToCProgram(configPackage);

    // === 9. Kiểm tra phản hồi từ C program ===
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
        await insertSystemLogToDatabase(userId, "IPSecProfile", "Config", "Create IPSec Profile", `Failed: ${profileData.ProfileName}`, "Failed", `C error: ${result}`);
        return res.status(500).json({
          message: "Failed to apply config",
          error: `Errors at: ${errorDetails.join(", ")} | Raw: ${result}`
        });
      } else {
        progressReport += " mainC updated;";
      }
    } else {
      if (result.toUpperCase().includes("ERROR")) {
        return res.status(500).json({ message: "Insert failed", error: result });
      } else {
        progressReport += " mainC updated;";
      }
    }

    // === 10. Lưu vào DB ===
    const profileId = await insertIpSecProfile(profileData);
    progressReport += " database updated";
    const updatedProfile = await getIpSecProfileById(profileId);

    // === 11. Ghi log ===
    const certLog = hasFiles
        ? `Certificates uploaded: ca-cert=${savedFiles["ca-cert"].originalName}, cert=${savedFiles.cert.originalName}, private-key=${savedFiles["private-key"].originalName}`
        : "No new certificates uploaded (existing kept)";

    await insertSystemLogToDatabase(
        userId,
        "IPSecProfile",
        "Config",
        "Create IPSec Profile",
        `Created: ${profileData.ProfileName}. ${certLog}`,
        "Success",
        null
    );

    // === 12. Trả kết quả ===
    return res.status(200).json({
      message: "Insert IPSec profile success",
      data: updatedProfile,
      progress: progressReport,
    });

  } catch (error) {
    console.error("error:", error);
    const isValidationError = error.message.includes("invalid") || error.message.includes("Missing") || error.message.includes("required");
    const status = isValidationError ? 400 : 500;
    const msg = isValidationError ? "Validation failed" : "Internal server error";
    await insertSystemLogToDatabase(req.user?.payload?.Id || "unknown", "IPSecProfile", "Config", "Create IPSec Profile", "Failed", "Failed", error.message);
    return res.status(status).json({ message: msg, error: error.message });
  }
};
/**
 * Cập nhật IPSec Profile
 *
 * Cập nhật thông tin của một IPSec Profile dựa trên profileId.
 * Kiểm tra tính hợp lệ của ProfileName và xử lý các trường RemoteGateway, SubnetRemoteGateway khi có nhiều kết nối.
 * Gửi cấu hình cập nhật đến chương trình C và lưu thay đổi vào cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa profileId và dữ liệu cập nhật.
 * @param {object} res - Phản hồi HTTP trả về kết quả cập nhật.
 */
exports.updateIpSecProfile = async (req, res) => {
  config.progressLog(config.COLORS.cyan, "\n[PROGRESS] : update IPSec Profile ...", config.COLORS.reset);
  const { profileId } = req.params;
  let updatedData = { ...req.body };
  const userId = req.user?.payload?.Id || null;
  let progressReport = "";
  console.log("profile id : ", profileId);
  console.log("updated data (raw from client) : ", updatedData);
  try {
    const currentProfile = await getIpSecProfileById(profileId);
    if (!currentProfile) {
      return res.status(404).json({
        success: false,
        message: "IPSec profile not found",
        progress: progressReport,
      });
    }
    if (updatedData.ProfileName && updatedData.ProfileName !== currentProfile.ProfileName) {
      const existingProfile = await getIpSecProfiles();
      if (existingProfile.some((profile) => profile.ProfileName === updatedData.ProfileName && profile.IPSecProfileId !== parseInt(profileId))) {
        return res.status(400).json({
          success: false,
          message: "ProfileName already exists",
          progress: progressReport,
        });
      }
    }
    const connectionCount = updatedData.ConnectionCount ?? currentProfile.ConnectionCount ?? 1;
    updatedData.ConnectionCount = connectionCount;
    updatedData.RemoteGateway = updatedData.RemoteGateway ?? updatedData.RemoteGateways ?? currentProfile.RemoteGateway ?? null;
    updatedData.SubnetRemoteGateway = updatedData.SubnetRemoteGateway ?? updatedData.SubnetRemoteGateways ?? currentProfile.SubnetRemoteGateway ?? null;
    delete updatedData.RemoteGateways;
    delete updatedData.SubnetRemoteGateways;
    const validateSemicolonCount = (fieldValue, fieldName, connectionCount) => {
      if (!fieldValue) return null;
      const count = (fieldValue.match(/;/g) || []).length;
      if (count !== connectionCount - 1) {
        return `${fieldName} invalid: expected ${connectionCount} values (need ${connectionCount - 1} ';'), got ${count} ';'`;
      }
      return null;
    };
    const remoteError = validateSemicolonCount(updatedData.RemoteGateway, "RemoteGateway", connectionCount);
    if (remoteError) {
      return res.status(400).json({ message: remoteError });
    }
    const subnetError = validateSemicolonCount(updatedData.SubnetRemoteGateway, "SubnetRemoteGateway", connectionCount);
    if (subnetError) {
      return res.status(400).json({ message: subnetError });
    }
    const { configPackage } = await buildConfigPackage(updatedData, currentProfile, false);
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
          await insertSystemLogToDatabase(userId, "IPSecProfile", "Config", "Update IPSec Profile", `Failed to update IPSec profile: ${profileId}`, "Failed", `C error: ${result}`);
          return res.status(500).json({
            success: false,
            message: "Failed to update IPSec profile",
            error: `Errors at: ${errorDetails.join(", ")} | Raw: ${result}`,
            progress: progressReport,
          });
        } else {
          progressReport += " mainC updated";
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
          progressReport += " mainC updated";
        }
      }
    }
    await updateIpSecProfile(profileId, {
      ...updatedData,
      LastModified: format(new Date(), "yyyy/MM/dd HH:mm:ss", { locale: vi }),
    });
    progressReport += " and database updated";
    const updatedProfile = await getIpSecProfileById(profileId);
    return res.status(200).json({
      success: true,
      message: "IPSec profile updated successfully",
      progress: progressReport,
      data: updatedProfile,
    });
  } catch (error) {
    console.error("Update IPSec profile error:", error);
    const isValidationError = error.message.includes("invalid") || error.message.includes("Missing");
    const status = isValidationError ? 400 : 500;
    const msg = isValidationError ? "Validation failed" : "Internal server error";
    return res.status(status).json({
      success: false,
      message: msg,
      error: error.message,
    });
  }
};
/**
 * Xóa IPSec Profile
 *
 * Xóa một IPSec Profile khỏi cơ sở dữ liệu dựa trên profileId.
 * Kiểm tra xem profile có tồn tại và không ở trạng thái Active trước khi xóa.
 * Ghi log hệ thống cho hành động xóa.
 * @param {object} req - Yêu cầu HTTP chứa profileId.
 * @param {object} res - Phản hồi HTTP trả về kết quả xóa.
 */
exports.deleteIpSecProfile = async (req, res) => {
  config.progressLog(config.COLORS.cyan, "\n[PROGRESS] : Delete IPSec Profiles ...", config.COLORS.reset);
  let progressReport = "";
  try {
    const profileId = req.params.profileId;
    const ipSecProfileData = await getIpSecProfileById(profileId);
    if (!ipSecProfileData) {
      await insertSystemLogToDatabase(req.user.payload.Id, "IPSecProfile", "Config", "Delete IPSec Profile", `Delete IPSec profile: ${ipSecProfileData?.ProfileName || "unknown"}`, "Failed", "IPSec profile not found");
      return res.status(404).json({ message: "IPSec profile not found" });
    }
    if (ipSecProfileData.Enable === 1) {
      await insertSystemLogToDatabase(req.user.payload.Id, "IPSecProfile", "Config", "Delete IPSec Profile", `Delete IPSec profile: ${ipSecProfileData.ProfileName}`, "Failed", "IPSec profile is currently active. Please deactivate it before deleting.");
      return res.status(400).json({
        message: "IPSec profile is currently active. Please deactivate it before deleting.",
      });
    }
    await deleteIpSecProfile(profileId);
    progressReport = "database updated";
    const deletedProfile = { ...ipSecProfileData, status: "Deleted" };
    await insertSystemLogToDatabase(req.user.payload.Id, "IPSecProfile", "Config", "Delete IPSec Profile", `Delete IPSec profile: ${ipSecProfileData.ProfileName}`, "Success", null);
    return res.status(200).json({
      message: "Delete IPSec profile success",
      data: deletedProfile,
      progress: progressReport,
    });
  } catch (error) {
    console.log("error ", error);
    const isValidationError = error.message.includes("invalid") || error.message.includes("Missing");
    const status = isValidationError ? 400 : 500;
    const msg = isValidationError ? "Validation failed" : "Internal server error";
    await insertSystemLogToDatabase(req.user.payload.Id, "IPSecProfile", "Config", "Delete IPSec Profile", `Delete IPSec profile`, "Failed", error.message);
    return res.status(status).json({ message: msg });
  }
};
/**
 * Xây dựng gói cấu hình
 *
 * Tạo gói cấu hình (configPackage) để gửi đến chương trình C dựa trên dữ liệu IPSec Profile mới và hiện tại.
 * So sánh các trường cấu hình để chỉ gửi những trường thay đổi (trừ khi là thêm mới).
 * Hỗ trợ xử lý đặc biệt cho RemoteGateway và SubnetRemoteGateway khi có nhiều giá trị.
 * @param {object} newConfig - Dữ liệu cấu hình mới.
 * @param {object} currentConfig - Dữ liệu cấu hình hiện tại (nếu có).
 * @param {boolean} isInsert - Xác định xem có phải thao tác thêm mới hay không.
 * @returns {object} - Đối tượng chứa configPackage và commandCount.
 */
const buildConfigPackage = async (newConfig, currentConfig = {}, isInsert = false) => {
  let configPackage = "";
  let commandCount = 0;

  const ipSecFields = {
    LocalGateway: "IPSEC_LOCAL_IP",
    SubnetLocalGateway: "IPSEC_LOCAL_SUBNET",
    RemoteGateway: "IPSEC_REMOTE_IP",
    SubnetRemoteGateway: "IPSEC_REMOTE_SUBNET",
    IKEVersion: "IPSEC_IKE_VERSION",
    Mode: "IPSEC_MODE",
    ESPAHProtocol: "IPSEC_PROTOCOL",
    IKEReauthTime: "IPSEC_IKE_TIME",
    EncryptionAlgorithm: "IPSEC_ENCRY",
    HashAlgorithm: "IPSEC_HASH",
    ReKeyTime: "IPSEC_KEY",
    ConnectionCount: "IPSEC_CONNECTION_COUNT",
    // Enable sẽ được xử lý SAU CÙNG
  };

  const normalizedCurrentConfig = {};
  Object.keys(newConfig).forEach((key) => {
    if (key in ipSecFields || key === "Enable") {
      normalizedCurrentConfig[key] = currentConfig[key] !== undefined ? currentConfig[key] : null;
    }
  });

  // === 1. Xử lý tất cả field TRỪ Enable ===
  for (const [field, command] of Object.entries(ipSecFields)) {
    if (newConfig[field] !== undefined) {
      let newValue = newConfig[field];
      let currentValue = normalizedCurrentConfig[field];

      if (field === "RemoteGateway" || field === "SubnetRemoteGateway") {
        const valueToSend = newValue && typeof newValue === "string" && newValue.includes(";")
            ? newValue
            : newValue !== null ? newValue : "null";

        if (isInsert || valueToSend !== currentValue) {
          configPackage += `${command}$${valueToSend}$`;
          commandCount++;
        }
        continue;
      }

      if (isInsert || newValue !== currentValue) {
        const finalValue = newValue !== null ? newValue : "null";
        configPackage += `${command}$${finalValue}$`;
        commandCount++;
      }
    }
  }

  // === 2. Xử lý Enable CUỐI CÙNG ===
  if (newConfig.Enable !== undefined) {
    const enableValue = newConfig.Enable ? 1 : 0;
    const currentEnable = normalizedCurrentConfig.Enable !== undefined
        ? (normalizedCurrentConfig.Enable ? 1 : 0)
        : 0;

    if (isInsert || enableValue !== currentEnable) {
      configPackage += `IPSEC_EN_DIS$${enableValue}$`;
      commandCount++;
    }
  }

  const totalProcessingTimeInSeconds = commandCount * 2;
  config.progressLog(config.COLORS.spacing, `Total processing time: `, config.COLORS.blue, `${totalProcessingTimeInSeconds} seconds`, config.COLORS.reset);

  return { configPackage, commandCount };
};