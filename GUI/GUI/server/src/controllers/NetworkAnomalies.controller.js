const { getAllNetworkAnomalies } = require('../models/NetworkAnomalies.model');
const { logAnalyze } = require('../services/logAnalyze.service');

exports.getAllNetworkAnomalies = async (req, res) => {
    try {
        const anomalies = await getAllNetworkAnomalies();
        return res.status(200).json({
            data: anomalies
        });
    } catch (error) {
        console.error('Error getting network anomalies:', error);
        return res.status(500).json({
            message: 'Error getting network anomalies'
        });
    }
}

exports.getNetworkAnalysis = async (req, res) => {
    try {
        const { timeStart, timeEnd } = req.query;
        const chartData = await logAnalyze(timeStart, timeEnd);
        return res.status(200).json({
            data: chartData
        });
    } catch (error) {
        console.error('Error getting network analysis:', error);
        return res.status(500).json({
            message: 'Error getting network analysis'
        });
    }
}