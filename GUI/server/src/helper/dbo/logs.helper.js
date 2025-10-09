const { format } = require('date-fns');
const { vi } = require('date-fns/locale');

const { insertDeviceLog } = require('../../models/DeviceLogs.model');

const insertSystemLogToDatabase = async (UserId, LogSource, LogType, LogActionContent, LogActionDetail, LogActionResult, LogActionResultDetail) => {
    try {
        const deviceLog = {};
        deviceLog.UserId = UserId;
        deviceLog.LogSource = LogSource;
        deviceLog.LogType = LogType;
        deviceLog.LogActionTime = format(new Date(), 'yyyy/MM/dd HH:mm:ss', { locale: vi});
        deviceLog.LogActionContent = LogActionContent;
        deviceLog.LogActionDetail = LogActionDetail;
        deviceLog.LogActionResult = LogActionResult;
        deviceLog.LogActionResultDetail = LogActionResultDetail;
        await insertDeviceLog(deviceLog);
    } catch (error) {
        console.error("Failed to insert system log into device");
    }
};

module.exports = {
    insertSystemLogToDatabase,
}