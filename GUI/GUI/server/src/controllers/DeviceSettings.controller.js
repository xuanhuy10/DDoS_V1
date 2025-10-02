const os = require('os');
const si = require('systeminformation');
const fs = require('fs');

const config = require('../config/index');
const { isProduction } = require('../config/index');

const { getDeviceSettings,
    updateDeviceSettings,
    updateDeviceDiskSetting } = require('../models/DeviceSettings.model');

const { getFileDetails } = require('../helper/utils.helper');
const { getStorageInfo, getCpuInfo, getCpuUsage, getMemoryInfo } = require('../helper/device.helper');
const { sendCommandToCProgram } = require('../services/socket.service');
const { autoDeleteActivityLogs } = require('../services/auto.service');
const {progressLog} = require("../config");

exports.getDeviceResourceUsage = async (req, res) => {
    try {
        // ADD CACHING HERE TO AVOID REPEATED CALLS TO SYSTEM INFORMATION

        const uptimeSeconds = os.uptime();
        const minutes = Math.floor(uptimeSeconds / 60);
        const hours = Math.floor(minutes / 60);
        const days = Math.floor(hours / 24);

        const formattedUptime = [
            days > 0 ? `${days} days` : '',
            hours % 24 > 0 ? `${hours % 24} hours` : '',
            minutes % 60 > 0 ? `${minutes % 60} minutes` : ''
        ].filter(Boolean).join(', ');

        // Lấy toàn bộ thông tin hệ thống cùng lúc
        const [memory, storageInfo, cpuInfo, cpucores, cpuTemp] = await Promise.all([
            si.mem(),
            getStorageInfo(),
            si.cpuCurrentSpeed(),
            os.cpus(),
            si.cpuTemperature(), // <-- Lấy nhiệt độ CPU
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
                    cores: 4,
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
}
exports.getDeviceDiskUsage = async (req, res) => {
    try {
        const logFiles = getFileDetails(config.packet.logPath);
        const logFilesNormal = getFileDetails(config.packet.logPathNormal);

        const logs = logFiles.logDetails.sort((a, b) => b.lastModified - a.lastModified);
        const logsNormal = logFilesNormal.logDetails.sort((a, b) => b.lastModified - a.lastModified);

        const data = {};

        const totallogsize = parseInt(logFiles?.logSize) + parseInt(logFilesNormal?.logSize);
        const storageInfo = await getStorageInfo();
        const { total, free, used } = storageInfo;

        data.totalLogSize = totallogsize;
        data.storageInfo = { total, free, used };
        data.logsCount = logs.length + logsNormal.length;

        return res.status(201).json({
            status: 'Success',
            data: data,
        });
    }
    catch (error) {
        console.error('Error getting address: ', error);
        return res.status(500).json({
            status: 'Failed',
            message: 'Error getting system'
        });
    }
}
// DEVICE SETTINGS GOES BELOW
exports.getDeviceDiskSetting = async (req, res) => {
    try {
        const deviceSettings = await getDeviceSettings();
        return res.status(200).json({ data: deviceSettings });
    } catch (error) {
        console.error('Error getting device settings:', error);
        return res.status(500).json({ message: 'Get device settings failed' });
    }
}
exports.getDeviceSettings = async (req, res) => {
    try {
        return res.status(200).json({ message: 'Get device settings success' });
    } catch (error) {
        console.error('Error getting device settings:', error);
        return res.status(500).json({ message: 'Get device settings failed' });
    }
}
exports.updateDeviceDiskSetting = async (req, res) => {
    try {
        const { DeviceAutoDeleteLogEnable, DeviceAutoDeleteLogThreshold, DeviceAutoRotationLogs } = req.body;

        // Nếu thiếu trường log, trả về lỗi rõ ràng
        if (
            typeof DeviceAutoDeleteLogEnable === 'undefined' ||
            typeof DeviceAutoDeleteLogThreshold === 'undefined' ||
            typeof DeviceAutoRotationLogs === 'undefined'
        ) {
            return res.status(400).json({ message: 'Missing log settings fields' });
        }

        await updateDeviceDiskSetting(DeviceAutoDeleteLogEnable, DeviceAutoDeleteLogThreshold, DeviceAutoRotationLogs);

        // Ghi threshold
        const thresholdFile = config.packet.diskThresholdPath;
        if (thresholdFile && DeviceAutoDeleteLogThreshold !== undefined) {
            fs.writeFileSync(thresholdFile, DeviceAutoDeleteLogThreshold.toString());
        }

        // Ghi trạng thái Auto clean logs
        const autoManualFile = config.packet.configAutoManualPath;
        if (autoManualFile && typeof DeviceAutoDeleteLogEnable !== 'undefined') {
            // Xử lý mọi kiểu dữ liệu: boolean, số, chuỗi
            let isAuto = false;
            if (
                DeviceAutoDeleteLogEnable === true ||
                DeviceAutoDeleteLogEnable === 'true' ||
                DeviceAutoDeleteLogEnable === 1 ||
                DeviceAutoDeleteLogEnable === '1'
            ) {
                isAuto = true;
            }
            if (
                DeviceAutoDeleteLogEnable === false ||
                DeviceAutoDeleteLogEnable === 'false' ||
                DeviceAutoDeleteLogEnable === 0 ||
                DeviceAutoDeleteLogEnable === '0'
            ) {
                isAuto = false;
            }
            fs.writeFileSync(autoManualFile, isAuto ? 'true' : 'false');
        }

        return res.status(200).json({ message: 'Update device disk setting success' });
    } catch (error) {
        console.error('Error updating device disk setting:', error);
        return res.status(500).json({ message: 'Update device disk setting failed' });
    }
};
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

        // Lấy setting hiện tại để giữ nguyên giá trị không thay đổi
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

        // Nếu có thay đổi ngưỡng log, đồng bộ file threshold
        if (DeviceAutoDeleteLogThreshold !== undefined) {
            const thresholdFile = config.packet.diskThresholdPath;
            if (thresholdFile) {
                fs.writeFileSync(thresholdFile, DeviceAutoDeleteLogThreshold.toString());
            }
        }

        // Nếu bật auto activity, gọi xóa log ngay
        if (DeviceAutoDeleteActivity == 1 || DeviceAutoDeleteActivity === true) {
            await autoDeleteActivityLogs();
        }

        return res.status(200).json({ message: 'Update device settings success' });
    } catch (error) {
        console.error('Error updating device settings:', error);
        return res.status(500).json({ message: 'Update device settings failed' });
    }
}
exports.deleteDeviceSettings = async (req, res) => {
    try {
        return res.status(200).json({ message: 'Delete device settings success' });
    } catch (error) {
        console.error('Error deleting device settings:', error);
        return res.status(500).json({ message: 'Delete device settings failed' });
    }
}
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
                res.status(200).json({ message: 'System reset successfully triggered' });
            } else {
                res.status(500).json({ message: 'Reset system failed from lower layer' });
            }
            return res;
        }
    } catch (error) {
        console.error('Reset system error:', error);
        return res.status(500).json({ message: 'Reset system failed' });
    }
};