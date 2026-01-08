const ExcelJS = require('exceljs');
const { format } = require('date-fns');
const path = require('path');
const AdmZip = require('adm-zip');
const fs = require('fs');

const { getLogSystem,
    insertLogSystem,
    updateLogSystem,
    deleteLogSystem, getLogSystembyLogType } = require('../models/LogSystem.model');

const { getAllAnomalies } = require('../models/Anomalies.model');
const { getAllNotification } = require('../models/Notification.model');

const config = require('../config/index');
const { getFileDetails, generateProfileImage} = require('../helper/utils.helper');
const {getStorageInfo, getMemoryInfo, getCpuInfo, getCpuUsage} = require('../helper/device.helper');
const { updateDDoS } = require('../models/DDoS.model');

exports.getSystemActive = async (req, res) => {
    try {
        const data = await getLogSystem();

        return res.status(200).json({
            status: 'Success',
            data: data
        });

    } catch (error) {
        return res.status(500).json({
            status: 'Failed',
            message: 'Error getting system'
        });

    }
}

//FIXME: split by half, one handle system, one handle traffic
exports.getLogSystembyLogType = async (req, res) => {
    try {
        const logtype = req.params.logtype;

        if (logtype === 'traffic') {
            const logFilesNormal = getFileDetails(config.packet.logPathNormal);
            const logsNormal = logFilesNormal.logDetails.sort((a, b) => b.lastModified - a.lastModified);
            return res.status(200).json({
                status: 'Success',
                data: logsNormal.length ? logsNormal : null,
            });
        } else if (logtype === 'threat') {
            const logFiles = getFileDetails(config.packet.logPath);
            const logs = logFiles.logDetails.sort((a, b) => b.lastModified - a.lastModified);
            return res.status(200).json({
                status: 'Success',
                data: logs.length ? logs : null,
            });
        }
        const data = await getLogSystembyLogType(logtype);
        if (data.length === 0) {
            data = '';
        }

        return res.status(200).json({
            status: 'Success',
            data: data
        });
    }
    catch (error) {
        console.error('Error getting profile: ', error);
        return res.status(500).json({
            status: 'Failed',
            message: 'Error getting profile' + error,
        });
    }
}


exports.downloadLogNormal = async function (req, res) {

    try {
        const files = req.params.files ? req.params.files.split(',') : [];

        if (!Array.isArray(files) || files.length === 0) {
            return res.status(500).json({ success: false, message: 'No files selected' });
        }
        
        const zip = new AdmZip();
        files.forEach(file => {
            try {
                let filePath = path.join(config.packet.logPathNormal, file.replace('Normal_', ''));
                zip.addLocalFile(filePath, '', `ACS_Netdef_${file}`);
                console.log(filePath);
            } catch (err) {
                console.error(`Skipping file ${file}:`, err.message);
            }
        });

        const zipName = path.join(config.packet.logPath, 'logs.zip');
        zip.writeZip(zipName);

        res.download(zipName, 'ACS_Netdef.zip', (err) => {
            if (err) {
                console.error('Error:', err);
                return res.status(500).json({ success: false, message: `Download zip failed: ${err.message}` });
            }
            fs.unlinkSync(zipName);
        });
    } catch (error) {
        console.error('Error:', error);
        res.status(500).json({ success: false, message: `Error: ${error}` });
    }
};

exports.deleteLogNormal = async function (req, res) {
    try {
        const files = req.params.files ? req.params.files.split(',') : [];

        if (!Array.isArray(files) || files.length === 0) {
            return res.status(500).json({ success: false, message: 'No files selected' });
        }

        // const logFiles = getFileDetails(config.packet.logPathNormal);
        // const logs = logFiles.logDetails.sort((a, b) => b.lastModified - a.lastModified);

        files.forEach(file => {
            const filePath = config.packet.logPathNormal + file;
            if (fs.existsSync(filePath)) {
                fs.unlinkSync(filePath);
                console.log(`Deleted file: ${file}; `);
            } else {
                console.log(`File not found: ${file}; `);
                res.status(500).json({ success: false, message: `File not found: ${file};` });
            }
        });
        res.status(201).json({ success: true, message: 'Delete log successful' });
    }
    catch (error) {
        console.error('Error:', error);
        res.status(500).json({ success: false, message: 'Delete log failed due to an error: ' + error });
    }
};

exports.downloadLogAttack = async function (req, res) {
    try {
        const files = req.params.files ? req.params.files.split(',') : [];
        if (!Array.isArray(files) || files.length === 0) {
            return res.status(500).json({ success: false, message: 'No files selected' });
        }

        const zip = new AdmZip();
        files.forEach(file => {
            try {
                let filePath = path.join(config.packet.logPath, file.replace('Threat_', ''));
                zip.addLocalFile(filePath, '', `ACS_Netdef_${file}`);
            } catch (err) {
                console.error(`Skipping file ${file}:`, err.message);
            }
        });

        const zipName = path.join(config.packet.logPath, 'logs.zip');
        zip.writeZip(zipName);

        res.download(zipName, 'ACS_Netdef.zip', (err) => {
            if (err) {
                console.error('Error:', err);
                return res.status(500).json({ success: false, message: `Download zip failed: ${err.message}` });
            }
            fs.unlinkSync(zipName);
        });
    } catch (error) {
        console.error('Error:', error);
        res.status(500).json({ success: false, message: `Error: ${error}` });
    }
};

exports.deleteLogAttack = async function (req, res) {
    try {
        const files = req.params.files ? req.params.files.split(',') : [];
        // const files = Array.isArray(req.body.files) ? req.body.files : [req.body.files].filter(Boolean);
        console.log('file', files);
        if (!Array.isArray(files) || files.length === 0) {
            return res.status(500).json({ success: false, message: 'No files selected' });
        }

        const logFiles = getFileDetails(config.packet.logPath);
        const logs = logFiles.logDetails.sort((a, b) => b.lastModified - a.lastModified);

        files.forEach(file => {
            const filePath = config.packet.logPath + file;
            // fs.unlinkSync(filePath);
            if (fs.existsSync(filePath)) {
                fs.unlinkSync(filePath);
                console.log(`Deleted file: ${file}; `);
            } else {
                console.log(`File not found: ${file}; `);
                res.status(500).json({ success: false, message: `File not found: ${file};` });
            }
        });
        res.status(201).json({ success: true, message: 'Delete log successful' });

    }
    catch (error) {

        console.error('Error:', error);

        res.status(500).json({ success: false, message: 'Delete log failed due to an error: ' + error });

    }
};

exports.getLogSetting = async function (req, res) {
    try {
        console.time('start');
        console.time('file');
        const logFiles = getFileDetails(config.packet.logPath);
        const logFilesNormal = getFileDetails(config.packet.logPathNormal);

        const logs = logFiles.logDetails.sort((a, b) => b.lastModified - a.lastModified);
        const logsNormal = logFilesNormal.logDetails.sort((a, b) => b.lastModified - a.lastModified);
        console.timeEnd('file');

        let data = {};  // Khởi tạo data với giá trị mặc định

        const totallogsize = parseInt(logFiles?.logSize) + parseInt(logFilesNormal?.logSize);
        console.time('storage');
        const storageInfo = await getStorageInfo();
        const { total, free, used } = storageInfo;
        console.timeEnd('storage');

        // Cập nhật data với giá trị đã định dạng
        data.totalLogSize = totallogsize;
        data.storageInfo = { total, free, used };
        data.logsCount = logs.length + logsNormal.length;

        let sentdata = data;
        console.timeEnd('start');
        return res.status(201).json({
            status: 'Success',
            data: sentdata,
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

exports.getDiskSetting = async function (req, res) {
}


exports.setAutodelete = async function (req, res) {
    try {
        const { TrafficAutoDeleteThreshold, TrafficAutoDelete } = req.body;
        console.log(req.body);

        const threshold = parseInt(TrafficAutoDeleteThreshold, 10);
        
        if (isNaN(threshold) || threshold < 20 || threshold > 100) {
            return res.status(500).json({ success: false, message: 'Auto delete threshold must be a number between 50 and 100' });
        }
        let data = {};
        data.TrafficAutoDelete = TrafficAutoDelete;
        data.TrafficAutoDeleteThreshold = TrafficAutoDeleteThreshold;
        await updateDDoS(data);

        const diskThresholdPath = config.packet.diskThresholdPath;
        fs.writeFileSync(diskThresholdPath, threshold.toString());
        return res.status(201).json({ success: true, message: 'Auto delete settings updated successfully', data: data });
    }
    catch (error) {
        console.error('Error:', error);
        return res.status(500).json({ success: false, message: 'Failed to update auto delete settings: ' + error });
    }
}

exports.getExcel = async (req, res) => {
    try {

        const logtype = req.params.logtype;
        console.log(logtype);

        const data = await getLogSystembyLogType(logtype);
        const now = format(new Date(), 'yyyy/MM/dd_HH-mm-ss');

        if (data.length === 0) {
            return res.status(500).json({
                status: 'Failed',
                message: 'No logs found for the specified log type',
            });
        }
        // 2. Tạo workbook và worksheet
        const workbook = new ExcelJS.Workbook();
        const worksheet = workbook.addWorksheet('Logs');

        // 3. Định nghĩa cột trong Excel
        worksheet.columns = [
            { header: 'UserId', key: 'UserId', width: 10 },
            { header: 'Username', key: 'Username', width: 20 },
            { header: 'LogSource', key: 'LogSource', width: 20 },
            { header: 'LogType', key: 'LogType', width: 15 },
            { header: 'ActionTime', key: 'ActionTime', width: 40 },
            { header: 'ActionContent', key: 'ActionContent', width: 40 },
            { header: 'ActionDetail', key: 'ActionDetail', width: 100 },
            { header: 'ActionResult', key: 'ActionResult', width: 15 },
            { header: 'ResultReason', key: 'ResultReason', width: 30 },
        ];

        // 4. Thêm dữ liệu vào worksheet
        data.forEach((data) => {
            worksheet.addRow({
                UserId: data.UserId,
                Username: data.Username,
                LogSource: data.LogSource,
                LogType: data.LogType,
                ActionTime: data.ActionTime,
                ActionContent: data.ActionContent,
                ActionDetail: data.ActionDetail,
                ActionResult: data.ActionResult,
                ResultReason: data.ResultReason,
            });
        });

        // 5. Xuất file Excel ra response
        res.setHeader('Content-Type', 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet');
        res.setHeader('Content-Disposition', `attachment; filename=logs_${logtype}_${now}.xlsx`);

        await workbook.xlsx.write(res);
        res.end(); // Kết thúc response

        // return res.status(200).json({
        //     status: 'Success',
        //     data: data
        // });
    }
    catch (error) {
        console.error('Error exporting logs to Excel:', error);

        return res.status(500).json({
            status: 'Failed',
            message: 'Error exporting logs to Excel:' + error,
        });

    }
}

exports.setNotification = async (req, res) => {
    try {
        const anomalies = await getAllAnomalies();
        const config = await getLogSystembyLogType('config');
        const system = await getLogSystembyLogType('system');
        const UserId = req.user.payload.Id;

        const NoticeAnomaly = await getAllNotification(UserId);
        const config_user = config.filter((item) => item.UserId === UserId);
        const system_user = system.filter((item) => item.UserId === UserId);


        return res.status(200).json({
            status: 'Success',
            // config: config_user,
            // system: system_user,
            // anomalies: anomalies,
            notification: {
                noti_Info: NoticeAnomaly,
                system: system_user,
                config: config_user,
                anomalies: anomalies
            }
        });
    }
    catch (error) {
        console.error('Error getting profile: ', error);

        return res.status(500).json({
            status: 'Failed',
            message: 'Error getting profile' + error,
        });
    }
}