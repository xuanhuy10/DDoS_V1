const os = require('os');
const si = require('systeminformation');
const fs = require('fs');
const config = require('../config/index');
const { isProduction } = require('../config/index');
const { getDeviceSettings, updateDeviceSettings, updateDeviceDiskSetting } = require('../models/DeviceSettings.model');
const { getFileDetails } = require('../helper/utils.helper');
const { getStorageInfo} = require('../helper/device.helper');
const { sendCommandToCProgram } = require('../services/socket.service');
const { autoDeleteActivityLogs } = require('../services/auto.service');

/**
 * Lấy thông tin sử dụng tài nguyên thiết bị
 *
 * Thu thập thông tin về thời gian hoạt động, bộ nhớ, CPU, nhiệt độ CPU và dung lượng lưu trữ của thiết bị.
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về thông tin tài nguyên thiết bị.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông tin tài nguyên.
 */
exports.getDeviceResourceUsage = async (req, res) => {
    try {
        const uptimeSeconds = os.uptime();
        const minutes = Math.floor(uptimeSeconds / 60);
        const hours = Math.floor(minutes / 60);
        const days = Math.floor(hours / 24);
        const formattedUptime = [
            days > 0 ? `${days} days` : '',
            hours % 24 > 0 ? `${hours % 24} hours` : '',
            minutes % 60 > 0 ? `${minutes % 60} minutes` : ''
        ].filter(Boolean).join(', ');
        const [memory, storageInfo, cpuInfo, cpucores, cpuTemp] = await Promise.all([
            si.mem(),
            getStorageInfo(),
            si.cpuCurrentSpeed(),
            os.cpus(),
            si.cpuTemperature(),
        ]);
        const cpuLoad = await si.currentLoad();
        const systemResourceUsage = {
            systemUptime: formattedUptime,
            performanceData: {
                memory: {
                    totalMemory: (memory.total / 1024 / 1024).toFixed(2) + ' Mb',
                    freeMemory: (memory.free / 1024 / 1024).toFixed(2) + ' Mb',
                    heapUsed: (memory.used / 1024 / 1024).toFixed(2) + ' Mb',
                    memoryPercentage: memory.percent,
                    committed: ((memory.active) / 1024 / 1024).toFixed(2) + ' Mb',
                },
                cpu: {
                    cores: cpucores.length,
                    speed: cpuInfo.avg + ' GHz',
                    utilization: cpuLoad.cpus.map((core, index) => ({
                        core: `Core ${index + 1}`,
                        usage: `${core.load.toFixed(2)}%`,
                    })),
                    overallUsage: `${cpuLoad.currentLoad.toFixed(2)}%`,
                },
                storageInfo,
                temperature: {
                    value: cpuTemp.main || 0,
                    unit: "°C"
                }
            }
        };
        return res.status(200).json({ data: systemResourceUsage });
    } catch (error) {
        console.error('Error getting device resource usage:', error);
        return res.status(500).json({ message: 'Get device resource usage failed' });
    }
};
/**
 * Lấy thông tin sử dụng dung lượng đĩa
 *
 * Thu thập thông tin về kích thước log, số lượng log và dung lượng lưu trữ của thiết bị.
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về thông tin dung lượng đĩa.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông tin dung lượng.
 */
exports.getDeviceDiskUsage = async (req, res) => {
    try {
        const logFiles = getFileDetails(config.packet.logPath);
        const logFilesNormal = getFileDetails(config.packet.logPathNormal);
        const logs = logFiles.logDetails.sort((a, b) => b.lastModified - a.lastModified);
        const logsNormal = logFilesNormal.logDetails.sort((a, b) => b.lastModified - a.lastModified);
        const data = {};
        const totallogsize = parseInt(logFiles?.logSize || 0) + parseInt(logFilesNormal?.logSize || 0);
        const storageInfo = await getStorageInfo();
        const { total, free, used } = storageInfo;
        data.totalLogSize = totallogsize;
        data.storageInfo = { total, free, used };
        data.logsCount = logs.length + logsNormal.length;
        return res.status(200).json({
            status: 'Success',
            data: data,
        });
    } catch (error) {
        console.error('Error getting disk usage: ', error);
        return res.status(500).json({
            status: 'Failed',
            message: 'Error getting system'
        });
    }
};
/**
 * Lấy cài đặt đĩa của thiết bị
 *
 * Truy vấn cơ sở dữ liệu để lấy thông tin cài đặt đĩa của thiết bị.
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về cài đặt đĩa.
 * @returns {Promise<object>} - Phản hồi JSON chứa cài đặt đĩa.
 */
exports.getDeviceDiskSetting = async (req, res) => {
    try {
        const deviceSettings = await getDeviceSettings();
        return res.status(200).json({ data: deviceSettings });
    } catch (error) {
        console.error('Error getting device settings:', error);
        return res.status(500).json({ message: 'Get device settings failed' });
    }
};
/**
 * Lấy cài đặt thiết bị
 *
 * Truy vấn cơ sở dữ liệu để lấy thông tin cài đặt thiết bị.
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về cài đặt thiết bị.
 * @returns {Promise<object>} - Phản hồi JSON chứa cài đặt thiết bị.
 */
exports.getDeviceSettings = async (req, res) => {
    try {
        const deviceSettings = await getDeviceSettings();
        return res.status(200).json({ data: deviceSettings });
    } catch (error) {
        console.error('Error getting device settings:', error);
        return res.status(500).json({ message: 'Get device settings failed' });
    }
};
/**
 * Cập nhật cài đặt đĩa của thiết bị
 *
 * Cập nhật cài đặt tự động xóa log, ngưỡng dung lượng và chế độ xoay log, đồng bộ với file cấu hình.
 * @param {object} req - Yêu cầu HTTP chứa thông tin cài đặt đĩa.
 * @param {object} res - Phản hồi HTTP trả về kết quả cập nhật.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo.
 */
exports.updateDeviceDiskSetting = async (req, res) => {
    try {
        const { DeviceAutoDeleteLogEnable, DeviceAutoDeleteLogThreshold, DeviceAutoRotationLogs } = req.body;
        if (
            typeof DeviceAutoDeleteLogEnable === 'undefined' ||
            typeof DeviceAutoDeleteLogThreshold === 'undefined' ||
            typeof DeviceAutoRotationLogs === 'undefined'
        ) {
            return res.status(400).json({ message: 'Missing log settings fields' });
        }
        await updateDeviceDiskSetting(DeviceAutoDeleteLogEnable, DeviceAutoDeleteLogThreshold, DeviceAutoRotationLogs);
        const thresholdFile = config.packet.diskThresholdPath;
        if (thresholdFile && DeviceAutoDeleteLogThreshold !== undefined) {
            fs.writeFileSync(thresholdFile, DeviceAutoDeleteLogThreshold.toString());
        }
        const autoManualFile = config.packet.configAutoManualPath;
        if (autoManualFile && typeof DeviceAutoDeleteLogEnable !== 'undefined') {
            let isAuto = false;
            if (
                DeviceAutoDeleteLogEnable === true ||
                DeviceAutoDeleteLogEnable === 'true' ||
                DeviceAutoDeleteLogEnable === 1 ||
                DeviceAutoDeleteLogEnable === '1'
            ) {
                isAuto = true;
            }
            fs.writeFileSync(autoManualFile, isAuto ? 'true' : 'false');
        }
        // Thêm: Truy vấn lại settings sau cập nhật
        const updatedSettings = await getDeviceSettings();
        return res.status(200).json({
            message: 'Update device disk setting success',
            data: updatedSettings  // Thêm object settings đã cập nhật
        });
    } catch (error) {
        console.error('Error updating device disk setting:', error);
        return res.status(500).json({ message: 'Update device disk setting failed' });
    }
};
/**
 * Cập nhật cài đặt thiết bị
 *
 * Cập nhật các cài đặt thiết bị như số lượng người dùng tối đa, ngưỡng xóa log, và xóa log hoạt động tự động.
 * Đồng bộ ngưỡng log với file cấu hình và kích hoạt xóa log nếu cần.
 * @param {object} req - Yêu cầu HTTP chứa thông tin cài đặt.
 * @param {object} res - Phản hồi HTTP trả về kết quả cập nhật.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo.
 */
exports.updateDeviceSettings = async (req, res) => {
    try {
        const {
            DeviceMaxUser,
            DeviceAutoDeleteLogThreshold,
            DeviceAutoDeleteLogEnable,
            DeviceAutoRotationLogs,
            DeviceAutoDeleteActivityInterval,
            DeviceAutoDeleteActivity
        } = req.body;
        const deviceSetting = await getDeviceSettings();
        const deviceSettings = {
            DeviceMaxUser: DeviceMaxUser !== undefined ? DeviceMaxUser : deviceSetting.DeviceMaxUser,
            DeviceAutoDeleteLogThreshold: DeviceAutoDeleteLogThreshold !== undefined ? DeviceAutoDeleteLogThreshold : deviceSetting.DeviceAutoDeleteLogThreshold,
            DeviceAutoDeleteLogEnable: DeviceAutoDeleteLogEnable !== undefined ? DeviceAutoDeleteLogEnable : deviceSetting.DeviceAutoDeleteLogEnable,
            DeviceAutoRotationLogs: DeviceAutoRotationLogs !== undefined ? DeviceAutoRotationLogs : deviceSetting.DeviceAutoRotationLogs,
            DeviceAutoDeleteActivityInterval: DeviceAutoDeleteActivityInterval !== undefined ? DeviceAutoDeleteActivityInterval : deviceSetting.DeviceAutoDeleteActivityInterval,
            DeviceAutoDeleteActivity: DeviceAutoDeleteActivity !== undefined ? DeviceAutoDeleteActivity : deviceSetting.DeviceAutoDeleteActivity
        };
        await updateDeviceSettings(deviceSettings);
        if (DeviceAutoDeleteLogThreshold !== undefined) {
            const thresholdFile = config.packet.diskThresholdPath;
            if (thresholdFile) {
                fs.writeFileSync(thresholdFile, DeviceAutoDeleteLogThreshold.toString());
            }
        }
        if (DeviceAutoDeleteActivity === 1 || DeviceAutoDeleteActivity === true) {
            await autoDeleteActivityLogs();
        }
        const updatedSettings = await getDeviceSettings();
        return res.status(200).json({
            message: 'Update device settings success',
            data: updatedSettings  // Thêm object settings đã cập nhật
        });
    } catch (error) {
        console.error('Error updating device settings:', error);
        return res.status(500).json({ message: 'Update device settings failed' });
    }
};
/**
 * Xóa cài đặt thiết bị
 *
 * Trả về thông báo thành công khi xóa cài đặt thiết bị (hiện chưa triển khai logic cụ thể).
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về thông báo.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo.
 */
exports.deleteDeviceSettings = async (req, res) => {
    try {
        const settingsBeforeDelete = await getDeviceSettings();
        // TODO: await deleteDeviceSettings(1); // Nếu model có delete
        // Thêm: Trả data là settings cũ với status 'Deleted'
        const deletedSettings = { ...settingsBeforeDelete, status: 'Deleted' };
        return res.status(200).json({
            message: 'Delete device settings success',
            data: deletedSettings  // Thêm object settings đã xóa (info cũ)
        });
    } catch (error) {
        console.error('Error deleting device settings:', error);
        return res.status(500).json({ message: 'Delete device settings failed' });
    }
};
/**
 * Reset hệ thống
 *
 * Gửi lệnh reset hệ thống đến chương trình C và xử lý phản hồi.
 * Kiểm tra trạng thái phản hồi khác nhau trong môi trường production và non-production.
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về kết quả reset.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo.
 */
exports.resetSystem = async (req, res) => {
    try {
        const command = "RS_TEMP$1$";
        const result = await sendCommandToCProgram(command);
        if (isProduction) {
            const resultParts = result.split('$');
            const commandIndex = resultParts.indexOf('RS_TEMP');
            if (commandIndex === -1 || resultParts[commandIndex + 2] !== 'OK') {
                return res.status(500).json({ message: 'Reset system failed from lower layer' });
            }
            return res.status(200).json({ message: 'System reset successfully triggered' });
        } else {
            if (result.toString().toUpperCase() === 'OK') {
                return res.status(200).json({ message: 'System reset successfully triggered' });
            } else {
                return res.status(500).json({ message: 'Reset system failed from lower layer' });
            }
        }
    } catch (error) {
        console.error('Reset system error:', error);
        return res.status(500).json({ message: 'Reset system failed' });
    }
};