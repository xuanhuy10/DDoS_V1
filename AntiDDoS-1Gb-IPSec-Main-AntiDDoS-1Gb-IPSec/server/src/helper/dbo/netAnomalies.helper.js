const { format } = require('date-fns');
const { vi } = require('date-fns/locale');

const { getNetworkAnomaliesByAnomaliesId, 
        getNetworkAnomaliesByAnomaliesStatus, 
        insertNetworkAnomalies, 
        updateNetworkAnomalies 
    } = require('../../models/NetworkAnomalies.model.js');

const getActiveAnomaliesFromDatabase = async () => {
    try {
        const anomalies = await getNetworkAnomaliesByAnomaliesStatus('active');
        return anomalies.map(anomaly => ({  
            ...anomaly,
            AnomaliesLastUpdateTime: Date.now(),
        }));
    } catch (error) {
        console.error("Failed to get active anomalies from database: ", error);
        throw error;
    }
}

// // CREATE TABLE NetworkAnomalies (
// //     AnomaliesId INTEGER PRIMARY KEY,
// //     Anomalies TEXT NOT NULL,
// //     AnomaliesTargetIp TEXT NOT NULL,
// //     AnomaliesTargetPort INTEGER NOT NULL,
// //     AnomaliesStats TEXT NOT NULL,
// //     AnomaliesStart DATETIME NOT NULL,
// //     AnomaliesEnd DATETIME,
// //     AnomaliesStatus BOOLEAN NOT NULL DEFAULT 1
// // );

const addNewAnomalyToDatabase = async (Anomalies, TargetIP, TargetPort, Stats) => {
    try {
        const newAnomaly = {
            Anomalies: Anomalies,
            AnomaliesTargetIp: TargetIP,
            AnomaliesTargetPort: TargetPort,
            AnomaliesStats: Stats,
            AnomaliesStart: format(new Date(), 'yyyy/MM/dd HH:mm:ss', { locale: vi}),
            AnomaliesEnd: null,
            AnomaliesStatus: 'active',
        };
        const newAnomalyId = await insertNetworkAnomalies(newAnomaly);
        return newAnomalyId;
    } catch (error) {
        console.error("Failed to add new anomaly to database: ", error);
        throw error;
    }
}

const updateEndTimeForAnomaliesToDatabase = async (anomalyId) => {
    try {
        const updatingAnomaly = await getNetworkAnomaliesByAnomaliesId(anomalyId);
        updatingAnomaly.AnomaliesEnd = format(new Date(), 'yyyy/MM/dd HH:mm:ss', { locale: vi});
        updatingAnomaly.AnomaliesStatus = 'inactive';

        const result = await updateNetworkAnomalies(anomalyId, updatingAnomaly);
        return result;
    } catch (error) {
        console.error("Failed to update end time for anomalies: ", error);
        throw error;
    }
}

module.exports = {
    getActiveAnomaliesFromDatabase,
    addNewAnomalyToDatabase,
    updateEndTimeForAnomaliesToDatabase,
};