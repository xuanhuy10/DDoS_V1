const { format } = require("date-fns");
const { vi } = require("date-fns/locale");

const fs = require("fs");
const path = require("path");
const AdmZip = require("adm-zip");
const ExcelJS = require("exceljs");

const config = require("../config");
const { getFileDetails } = require("../helper/utils.helper");
const { sendCommandToCProgram } = require("../services/socket.service");

const {
  getAllDeviceLogs,
  getDeviceLogByLogType,
  deleteDeviceLogsByIds,
  getDeviceLogsByIds,
  getDeviceLogsByTypeAndTimeRange,
  deleteDeviceLogsByTypeAndTimeRange,
} = require("../models/DeviceLogs.model");

exports.getAllDeviceLogs = async (req, res) => {
  try {
    const deviceLogs = await getAllDeviceLogs();
    return res.status(200).json({ data: deviceLogs });
  } catch (error) {
    console.log("error ", error);
    return res.status(500).json({ message: "Get all device logs failed" });
  }
};

exports.getDeviceLogByLogType = async (req, res) => {
  try {
    const deviceLogs = await getDeviceLogByLogType(req.params.logType);
    return res.status(200).json({ data: deviceLogs });
  } catch (error) {
    console.log("error ", error);
    return res
      .status(500)
      .json({ message: "Get device log by log type failed" });
  }
};

exports.getTrafficLogByLogType = async (req, res) => {
  try {
    const logType = req.params.logType;

    if (logType === "traffic") {
      const logFilesNormal = getFileDetails(config.packet.logPathNormal);
      const logsNormal = logFilesNormal.logDetails.sort(
        (a, b) => b.lastModified - a.lastModified
      );
      return res.status(200).json({
        data: logsNormal.length ? logsNormal : [],
      });
    } else if (logType === "threat") {
      const logFiles = getFileDetails(config.packet.logPath);
      const logs = logFiles.logDetails.sort(
        (a, b) => b.lastModified - a.lastModified
      );
      return res.status(200).json({
        data: logs.length ? logs : [],
      });
    } else {
      return res.status(400).json({ message: "Invalid log type" });
    }
  } catch (error) {
    console.log("error ", error);
    return res
      .status(500)
      .json({ message: "Get network log by log type failed" });
  }
};

// Export logs theo lá»±a chá»n Ä‘á»‹nh dáº¡ng
exports.exportLogTrafficbyLogTypeAndLogName = async (req, res) => {
  try {
    const logType = req.params.logType;
    const logFiles = req.params.files.split(",");
    const formatType = req.query.format || "zip";

    if (logFiles.length === 0) {
      return res
        .status(400)
        .json({ message: "No log files provided for export" });
    }

    let logPath;
    if (logType === "traffic") logPath = config.packet.logPathNormal;
    else if (logType === "threat") logPath = config.packet.logPath;
    else return res.status(400).json({ message: "Invalid log type" });

    if (formatType === "zip-excel") {
      const AdmZip = require("adm-zip");
      const zip = new AdmZip();
      const ExcelJS = require("exceljs");
      for (const file of logFiles) {
        const filePath = path.join(logPath, file);
        if (!fs.existsSync(filePath)) {
          return res.status(404).json({ message: `File ${file} not found` });
        }
        const workbook = new ExcelJS.Workbook();
        const worksheet = workbook.addWorksheet("Log");
        worksheet.columns = [
          { header: "Time", key: "time", width: 20 },
          { header: "SRC_IP", key: "src_ip", width: 18 },
          { header: "DST_IP", key: "dst_ip", width: 18 },
          { header: "SRC_PORT", key: "src_port", width: 13 },
          { header: "DST_PORT", key: "dst_port", width: 13 },
          { header: "Protocol", key: "protocol", width: 10 },
          { header: "Attack Type", key: "attack_type", width: 15 },
          { header: "Bandwidth (byte/s)", key: "bandwidth", width: 18 },
          { header: "Packet/s", key: "packet_s", width: 10 },
          { header: "Name Port", key: "name_port", width: 10 },
        ];
        const content = fs.readFileSync(filePath, "utf8");
        const lines = content.split("\n").filter((line) => line.trim());
        for (const line of lines) {
          const parts = line.trim().split(/\s+/);
          if (parts.length >= 10) {
            const time = parts[0] + " " + parts[1];
            const src_ip = parts[2];
            const dst_ip = parts[3];
            const src_port = parts[4];
            const dst_port = parts[5];
            const protocol = parts[6];
            const attack_type = parts.slice(7, parts.length - 3).join(" ");
            const bandwidth = parts[parts.length - 3];
            const packet_s = parts[parts.length - 2];
            const name_port = parts[parts.length - 1];

            worksheet.addRow({
              time,
              src_ip,
              dst_ip,
              src_port,
              dst_port,
              protocol,
              attack_type,
              bandwidth,
              packet_s,
              name_port,
            });
          }
        }
        const buffer = await workbook.xlsx.writeBuffer();
        zip.addFile(`Sysnetdef_Logs_${logType}_${file}.xlsx`, buffer);
      }
      const zipFileName = `Sysnetdef_Logs_${logType}_${Date.now()}.zip`;
      const data = zip.toBuffer();
      res.set("Content-Type", "application/zip");
      res.set("Content-Disposition", `attachment; filename=${zipFileName}`);
      return res.send(data);
    }

    if (formatType === "zip-txt") {
      const AdmZip = require("adm-zip");
      const zip = new AdmZip();
      for (const file of logFiles) {
        const filePath = path.join(logPath, file);
        if (!fs.existsSync(filePath)) {
          return res.status(404).json({ message: `File ${file} not found` });
        }
        const content = fs.readFileSync(filePath, "utf8");
        zip.addFile(`${file}.txt`, Buffer.from(content, "utf8"));
      }
      const zipFileName = `logs_${logType}_${Date.now()}.zip`;
      const data = zip.toBuffer();
      res.set("Content-Type", "application/zip");
      res.set("Content-Disposition", `attachment; filename=${zipFileName}`);
      return res.send(data);
    }
    if (
      logFiles.length === 1 &&
      (formatType === "txt" || formatType === "xlsx")
    ) {
      const file = logFiles[0];
      const filePath = path.join(logPath, file);
      if (!fs.existsSync(filePath)) {
        return res.status(404).json({ message: `File ${file} not found` });
      }
      if (formatType === "txt") {
        const content = fs.readFileSync(filePath, "utf8");
        res.setHeader("Content-Type", "text/plain");
        res.setHeader(
          "Content-Disposition",
          `attachment; filename=${file}.txt`
        );
        return res.send(content);
      }
      if (formatType === "xlsx") {
        const ExcelJS = require("exceljs");
        const workbook = new ExcelJS.Workbook();
        const worksheet = workbook.addWorksheet("Log");
        worksheet.columns = [
          { header: "Time", key: "time", width: 20 },
          { header: "SRC_IP", key: "src_ip", width: 18 },
          { header: "DST_IP", key: "dst_ip", width: 18 },
          { header: "SRC_PORT", key: "src_port", width: 10 },
          { header: "DST_PORT", key: "dst_port", width: 10 },
          { header: "Protocol", key: "protocol", width: 10 },
          { header: "Attack Type", key: "attack_type", width: 15 },
          { header: "Bandwidth (byte/s)", key: "bandwidth", width: 12 },
          { header: "Packet/s", key: "packet_s", width: 10 },
          { header: "Name Port", key: "name_port", width: 10 },
        ];

        const file = logFiles[0];
        const filePath = path.join(logPath, file);
        if (!fs.existsSync(filePath)) {
          return res.status(404).json({ message: `File ${file} not found` });
        }
        const content = fs.readFileSync(filePath, "utf8");
        const lines = content.split("\n").filter((line) => line.trim());

        for (const line of lines) {
          const parts = line.trim().split(/\s+/);
          if (parts.length >= 10) {
            const time = parts[0] + " " + parts[1];
            const src_ip = parts[2];
            const dst_ip = parts[3];
            const src_port = parts[4];
            const dst_port = parts[5];
            const protocol = parts[6];
            const attack_type = parts.slice(7, parts.length - 3).join(" ");
            const bandwidth = parts[parts.length - 3];
            const packet_s = parts[parts.length - 2];
            const name_port = parts[parts.length - 1];

            worksheet.addRow({
              time,
              src_ip,
              dst_ip,
              src_port,
              dst_port,
              protocol,
              attack_type,
              bandwidth,
              packet_s,
              name_port,
            });
          }
        }

        res.setHeader(
          "Content-Type",
          "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"
        );
        res.setHeader(
          "Content-Disposition",
          `attachment; filename=Sysnetdef_Logs_${logType}_${file}.xlsx`
        );
        await workbook.xlsx.write(res);
        return;
      }
    }

    if (
      logFiles.length > 1 &&
      (formatType === "txt" || formatType === "xlsx")
    ) {
      return res.status(400).json({
        message: "FE nĂªn gá»i tá»«ng file riĂªng biá»‡t vá»›i format txt/xlsx",
      });
    }

    const AdmZip = require("adm-zip");
    const zip = new AdmZip();
    for (const file of logFiles) {
      const filePath = path.join(logPath, file);
      if (fs.existsSync(filePath)) {
        zip.addLocalFile(filePath);
      } else {
        return res.status(404).json({ message: `File ${file} not found` });
      }
    }
    const zipFileName = `logs_${logType}_${Date.now()}.zip`;
    const data = zip.toBuffer();
    res.set("Content-Type", "application/zip");
    res.set("Content-Disposition", `attachment; filename=${zipFileName}`);
    res.send(data);
  } catch (error) {
    console.log("error ", error);
    return res
      .status(500)
      .json({ message: "Export log traffic by log type and log name failed" });
  }
};

exports.deleteLogTrafficbyLogTypeAndLogName = async (req, res) => {
  try {
    const logType = req.params.logType;
    const logFiles = req.params.files.split(",");

    if (logFiles.length === 0) {
      return res
        .status(400)
        .json({ message: "No log files provided for deletion" });
    }

    let deletedLatest = false;
    let latestFile = null;

    let logPath;
    if (logType === "traffic") logPath = config.packet.logPathNormal;
    else if (logType === "threat") logPath = config.packet.logPath;
    else return res.status(400).json({ message: "Invalid log type" });

    const files = fs
      .readdirSync(logPath)
      .map((file) => {
        const stat = fs.statSync(path.join(logPath, file));
        return { file, mtime: stat.mtime };
      })
      .sort((a, b) => b.mtime - a.mtime);

    if (files.length > 0) {
      latestFile = files[0].file;
      if (logFiles.includes(latestFile)) {
        deletedLatest = true;
      }
    }
    for (const file of logFiles) {
      const filePath = path.join(logPath, file);
      if (fs.existsSync(filePath)) {
        fs.unlinkSync(filePath);
      } else {
        return res.status(404).json({ message: `File ${file} not found` });
      }
    }
    if (deletedLatest) {
      await sendCommandToCProgram("CREATE_NEW_LOG$1$");
    }

    return res.status(200).json({ message: "Logs deleted successfully" });
  } catch (error) {
    console.log("error ", error);
    return res
      .status(500)
      .json({ message: "Delete log traffic by log name failed" });
  }
};

exports.deleteDeviceLogByTypeAndTimeRange = async (req, res) => {
  try {
    const { logType } = req.params;
    const { from, to } = req.query;
    const result = await deleteDeviceLogsByTypeAndTimeRange(logType, from, to);
    if (result.changes === 0) {
      return res
        .status(404)
        .json({ message: "No logs deleted (check time range and logType)" });
    }
    return res
      .status(200)
      .json({ message: "Logs deleted successfully", deleted: result.changes });
  } catch (error) {
    console.error("Error in deleteDeviceLogByTypeAndTimeRange:", error);
    res.status(500).json({ message: "Delete logs by time range failed" });
  }
};

exports.exportDeviceLogByLogTypeAndIds = async (req, res) => {
  try {
    const logType = req.params.logType;
    const ids = req.params.ids.split(",").map(Number);
    const logs = await getDeviceLogsByIds(logType, ids);
    const workbook = new ExcelJS.Workbook();
    const worksheet = workbook.addWorksheet("Logs");

    worksheet.columns = [
      { header: "LogId", key: "LogId", width: 10 },
      { header: "Username", key: "Username", width: 20 },
      { header: "LogActionTime", key: "LogActionTime", width: 20 },
      { header: "LogActionContent", key: "LogActionContent", width: 30 },
      { header: "LogActionDetail", key: "LogActionDetail", width: 30 },
      { header: "LogActionResult", key: "LogActionResult", width: 15 },
      {
        header: "LogActionResultDetail",
        key: "LogActionResultDetail",
        width: 30,
      },
    ];

    logs.forEach((log) => {
      worksheet.addRow({
        LogId: log.LogId,
        Username: log.Username || "",
        LogActionTime: log.LogActionTime || "",
        LogActionContent: log.LogActionContent || "",
        LogActionDetail: log.LogActionDetail || "",
        LogActionResult: log.LogActionResult || "",
        LogActionResultDetail: log.LogActionResultDetail || "",
      });
    });
    res.setHeader(
      "Content-Type",
      "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"
    );
    res.setHeader(
      "Content-Disposition",
      `attachment; filename=activity_logs_${logType}_${new Date()
        .toISOString()
        .slice(0, 10)}.xlsx`
    );
    await workbook.xlsx.write(res);
  } catch (error) {
    console.log("error ", error);
    return res.status(500).json({ message: "Export logs failed" });
  }
};

exports.exportDeviceLogByTypeAndTimeRange = async (req, res) => {
  try {
    let { logType } = req.params;
    const { from, to } = req.query;
    logType = logType.charAt(0).toUpperCase() + logType.slice(1).toLowerCase();
    const logs = await getDeviceLogsByTypeAndTimeRange(logType, from, to);
    if (!logs || logs.length === 0) {
      return res
        .status(404)
        .json({ message: "No logs found for the specified time range" });
    }

    const ExcelJS = require("exceljs");
    const workbook = new ExcelJS.Workbook();
    const worksheet = workbook.addWorksheet("Activity Logs");
    worksheet.columns = [
      { header: "LogId", key: "LogId", width: 10 },
      { header: "Username", key: "Username", width: 20 },
      { header: "LogActionTime", key: "LogActionTime", width: 20 },
      { header: "LogActionContent", key: "LogActionContent", width: 30 },
      { header: "LogActionDetail", key: "LogActionDetail", width: 30 },
      { header: "LogActionResult", key: "LogActionResult", width: 15 },
      {
        header: "LogActionResultDetail",
        key: "LogActionResultDetail",
        width: 30,
      },
    ];
    logs.forEach((log) => worksheet.addRow(log));

    res.setHeader(
      "Content-Type",
      "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"
    );
    res.setHeader(
      "Content-Disposition",
      `attachment; filename=device_logs_${logType}_${from}_to_${to}.xlsx`
    );
    await workbook.xlsx.write(res);
    res.end();
  } catch (error) {
    console.error("Error in exportDeviceLogByTypeAndTimeRange:", error);
    return res
      .status(500)
      .json({ message: "Export logs by time range failed" });
  }
};

exports.deleteDeviceLogByLogTypeAndIds = async (req, res) => {
  try {
    let logType = req.params.logType;
    logType = logType.charAt(0).toUpperCase() + logType.slice(1).toLowerCase();
    const ids = req.params.ids.split(",").map(Number);
    const result = await deleteDeviceLogsByIds(logType, ids);
    return res
      .status(200)
      .json({ message: "Logs deleted successfully", deleted: result.changes });
  } catch (error) {
    console.log("error ", error);
    return res.status(500).json({ message: "Delete logs by ids failed" });
  }
};

exports.exportLogTrafficByTypeAndTimeRange = async (req, res) => {
  try {
    const { logType } = req.params;
    const { from, to } = req.query;
    const logPath =
      logType === "traffic"
        ? config.packet.logPathNormal
        : config.packet.logPath;
    const files = fs.readdirSync(logPath).filter((file) => {
      const stat = fs.statSync(path.join(logPath, file));
      const mtime = stat.mtime;
      return mtime >= new Date(from) && mtime <= new Date(to);
    });

    if (!files.length)
      return res.status(404).json({ message: "No logs found in time range" });

    const zip = new AdmZip();
    files.forEach((file) => zip.addLocalFile(path.join(logPath, file)));

    const zipName = `${logType}_logs_${from}_to_${to}.zip`;
    const data = zip.toBuffer();
    res.set("Content-Type", "application/zip");
    res.set("Content-Disposition", `attachment; filename=${zipName}`);
    res.send(data);
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: "Export logs by time range failed" });
  }
};

exports.deleteLogTrafficByTypeAndTimeRange = async (req, res) => {
  try {
    const { logType } = req.params;
    const { from, to } = req.query;
    const logPath =
      logType === "traffic"
        ? config.packet.logPathNormal
        : config.packet.logPath;
    const files = fs.readdirSync(logPath).filter((file) => {
      const stat = fs.statSync(path.join(logPath, file));
      const mtime = stat.mtime;
      return mtime >= new Date(from) && mtime <= new Date(to);
    });

    files.forEach((file) => fs.unlinkSync(path.join(logPath, file)));
    res
      .status(200)
      .json({ message: "Logs deleted successfully", deleted: files.length });
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: "Delete logs by time range failed" });
  }
};
