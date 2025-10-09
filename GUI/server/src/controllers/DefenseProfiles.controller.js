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
  getDefenseProfilesByOffset,
  getActiveDefenseProfile,
  getDefenseProfileByDefenseProfileId,
  getAttackTypeOfDefenseProfile,
  getAllThresholdByActiveDefenseProfile,
  insertDefenseProfile,
  updateDefenseProfile,
  deleteDefenseProfile,
} = require("../models/DefenseProfiles.model");

/**
 * Kiểm tra khóa có nên được bao gồm
 *
 * Kiểm tra xem một khóa có nên được sử dụng trong gói cấu hình hay không, loại trừ các khóa liên quan đến metadata của profile.
 * @param {string} key - Khóa cần kiểm tra.
 * @returns {boolean} - True nếu khóa nên được bao gồm, False nếu không.
 */
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
/**
 * Cập nhật thời gian sử dụng
 *
 * Thêm một bản ghi thời gian sử dụng vào DefenseProfileUsingTime của profile, bao gồm thời gian bắt đầu và kết thúc (nếu không phải profile hiện tại).
 * @param {object} profile - Đối tượng profile cần cập nhật thời gian sử dụng.
 * @param {boolean} isCurrent - Xác định xem profile có đang được sử dụng hiện tại hay không.
 */
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
/**
 * Xây dựng gói cấu hình
 *
 * Tạo gói cấu hình để gửi đến chương trình C dựa trên dữ liệu profile mới và hiện tại, chỉ bao gồm các trường thay đổi (trừ profile DEFAULT).
 * @param {object} config - Dữ liệu cấu hình mới.
 * @param {object} currentConfig - Dữ liệu cấu hình hiện tại.
 * @param {string} portName - Tên cổng (mặc định là PORT1).
 * @returns {object} - Đối tượng chứa configPackage và differentFieldsCount.
 */
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
/**
 * Xây dựng gói cấu hình đầy đủ
 *
 * Tạo gói cấu hình đầy đủ để gửi đến chương trình C, bao gồm tất cả các trường hợp lệ của profile.
 * @param {object} config - Dữ liệu cấu hình của profile.
 * @param {string} portName - Tên cổng (mặc định là PORT1).
 * @returns {object} - Đối tượng chứa configPackage và fieldsCount.
 */
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
/**
 * Kiểm tra tên profile trùng lặp
 *
 * Kiểm tra xem tên profile đã tồn tại trong cơ sở dữ liệu hay chưa (không phân biệt hoa thường).
 * @param {string} name - Tên profile cần kiểm tra.
 * @returns {Promise<boolean>} - True nếu tên đã tồn tại, False nếu không.
 */
const checkDuplicateName = async (name) => {
  const profiles = await getAllDefenseProfiles();
  return profiles.some(
      (profile) => profile.DefenseProfileName.toLowerCase() === name.toLowerCase()
  );
};
/**
 * Xây dựng gói cấu hình từ thiết bị
 *
 * Tạo gói cấu hình cho một giao diện thiết bị dựa trên profile mới và profile hiện tại (nếu có).
 * Hỗ trợ bao gồm hoặc loại bỏ chế độ bảo vệ của giao diện.
 * @param {object} interfaceObj - Đối tượng giao diện thiết bị.
 * @param {object} newProfile - Profile mới cần áp dụng.
 * @param {boolean} includeProtect - Có bao gồm cấu hình bảo vệ giao diện hay không.
 * @param {object} currentProfile - Profile hiện tại (nếu có).
 * @returns {object} - Đối tượng chứa config và differentFieldsCount.
 */
const buildConfigPackageFromDevice = (interfaceObj, newProfile, includeProtect = true, currentProfile = null) => {
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
};
/**
 * Lấy danh sách Defense Profiles
 *
 * Lấy tất cả hoặc một phần danh sách Defense Profiles dựa trên offset (phân trang).
 * @param {object} req - Yêu cầu HTTP chứa thông tin offset (nếu có).
 * @param {object} res - Phản hồi HTTP trả về danh sách profiles.
 * @returns {Promise<object>} - Phản hồi JSON chứa danh sách profiles.
 */
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
      const defenseProfiles = await getDefenseProfilesByOffset(offset);
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
/**
 * Lấy danh sách Defense Profiles đang hoạt động
 *
 * Lấy tất cả Defense Profiles có trạng thái Active.
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về danh sách profiles đang hoạt động.
 * @returns {Promise<object>} - Phản hồi JSON chứa danh sách profiles.
 */
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
/**
 * Lấy cấu hình loại tấn công của Defense Profile
 *
 * Lấy cấu hình cho một loại tấn công cụ thể từ Defense Profile đang hoạt động.
 * @param {object} req - Yêu cầu HTTP chứa attackType.
 * @param {object} res - Phản hồi HTTP trả về cấu hình loại tấn công.
 * @returns {Promise<object>} - Phản hồi JSON chứa cấu hình loại tấn công.
 */
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
/**
 * Lấy tất cả ngưỡng của Defense Profile đang hoạt động
 *
 * Lấy các giá trị ngưỡng và tính toán tỷ lệ cho các loại tấn công từ Defense Profile đang hoạt động.
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về các giá trị ngưỡng.
 * @returns {Promise<object>} - Phản hồi JSON chứa các giá trị ngưỡng.
 */
exports.getAllThresholdByActiveDefenseProfile = async (req, res) => {
  progressLog(
      config.COLORS.cyan,
      "\n[PROGRESS] : Get All Threshold By Active Defense Profile ...",
      config.COLORS.reset
  );
  try {
    const profileAllThreshold = await getAllThresholdByActiveDefenseProfile();
    if (!profileAllThreshold) {
      return res
          .status(404)
          .json({ message: "No active defense profile found" });
    }
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
/**
 * Thêm mới Defense Profile
 *
 * Thêm một Defense Profile mới vào cơ sở dữ liệu và gửi cấu hình đến chương trình C.
 * Kiểm tra tên profile trùng lặp, chuẩn bị dữ liệu và ghi log hệ thống.
 * @param {object} req - Yêu cầu HTTP chứa dữ liệu profile.
 * @param {object} res - Phản hồi HTTP trả về kết quả thêm mới.
 * @returns {Promise<object>} - Phản hồi JSON chứa profileId và thông báo.
 */
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
    for (let key in defenseProfileData) {
      if (
          defenseProfileData[key] == null ||
          defenseProfileData[key] === undefined
      ) {
        return res.status(400).json({ message: `Missing field: ${key}` });
      }
    }
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
    const timeout = Math.max(fieldsCount * 2 * 1000, 5000);
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
    const profileId = await insertDefenseProfile(defenseProfileData);
    progressReport += " database updated";
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
 * Cập nhật Defense Profile
 *
 * Cập nhật thông tin Defense Profile dựa trên profileId, kiểm tra tên trùng lặp và áp dụng cấu hình cho các giao diện liên quan.
 * Gửi cấu hình đến chương trình C và cập nhật cơ sở dữ liệu, ghi log hệ thống.
 * @param {object} req - Yêu cầu HTTP chứa profileId và dữ liệu cập nhật.
 * @param {object} res - Phản hồi HTTP trả về kết quả cập nhật.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông tin cập nhật.
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
    const currentProfile = await getDefenseProfileByDefenseProfileId(profileId);
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
    const configPackages = [];
    for (const iface of usingInterfaces) {
      const { config, differentFieldsCount } = buildConfigPackageFromDevice(
          iface,
          mergedData,
          false,
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
/**
 * Áp dụng Defense Profile cho giao diện
 *
 * Áp dụng Defense Profile cho các giao diện được chỉ định, gửi cấu hình đến chương trình C và cập nhật cơ sở dữ liệu.
 * Cập nhật thời gian sử dụng và trạng thái profile, ghi log hệ thống.
 * @param {object} req - Yêu cầu HTTP chứa profileId và danh sách giao diện.
 * @param {object} res - Phản hồi HTTP trả về kết quả áp dụng.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo thành công hoặc lỗi.
 */
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
    const newProfile = await getDefenseProfileByDefenseProfileId(profileId);
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
          ? await getDefenseProfileByDefenseProfileId(iface.DefenseProfileId)
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
    const totalProcessingTimeInSeconds = totalDifferentFields * 2;
    config.progressLog(
        config.COLORS.spacing,
        `Total time to process for all ports: `,
        config.COLORS.blue,
        `${totalProcessingTimeInSeconds} seconds`,
        config.COLORS.reset
    );
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
    progressReport += `mainC updated`;
    for (const { configPackage, differentFieldsCount } of configPackages) {
      config.progressLog(
          config.COLORS.spacing,
          "Command sent to C program: ",
          config.COLORS.magenta,
          configPackage,
          config.COLORS.reset
      );
      const timeout = Math.max(differentFieldsCount * 2 * 1000, 5000);
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
/**
 * Xóa Defense Profile
 *
 * Xóa một Defense Profile khỏi cơ sở dữ liệu dựa trên profileId, kiểm tra xem profile có đang hoạt động hay không.
 * Ghi log hệ thống cho hành động xóa.
 * @param {object} req - Yêu cầu HTTP chứa profileId.
 * @param {object} res - Phản hồi HTTP trả về kết quả xóa.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo thành công hoặc lỗi.
 */
exports.deleteDefenseProfile = async (req, res) => {
  config.progressLog(
      config.COLORS.cyan,
      "\n[PROGRESS] : delete Defense Profile ...",
      config.COLORS.reset
  );
  let progressReport = "";
  try {
    const profileId = req.params.profileId;
    const defenseProfileData = await getDefenseProfileByDefenseProfileId(
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