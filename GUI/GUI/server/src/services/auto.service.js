const { getDeviceSettings } = require('../models/DeviceSettings.model');
const { deleteOldActivityLogs } = require('../models/DeviceLogs.model');
const cron = require('node-cron');

cron.schedule('0 0 * * *', async () => {
    console.log('Running scheduled task: AutoDeleteActivityLogs');
    try {
        await autoDeleteActivityLogs();
    } catch (error) {
        console.error('Error in scheduled task:', error);
    }
});

async function autoDeleteActivityLogs() {
    const settings = await getDeviceSettings();
    console.log('AutoDeleteActivity:', settings.DeviceAutoDeleteActivity, 'Interval:', settings.DeviceAutoDeleteActivityInterval);
    if (!settings.DeviceAutoDeleteActivity) return;
    const days = settings.DeviceAutoDeleteActivityInterval || 3;
    console.log('Deleting logs older than', days, 'days');
    const result = await deleteOldActivityLogs(days);
    console.log('Deleted rows:', result.changes);
}

module.exports = { autoDeleteActivityLogs };