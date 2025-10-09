const { getAllNetworkAnomalies } = require('../models/NetworkAnomalies.model');
const { logAnalyze } = require('../services/logAnalyze.service');

/**
 * Lấy tất cả các bất thường mạng
 *
 * Truy vấn cơ sở dữ liệu để lấy danh sách tất cả các bất thường mạng.
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về danh sách bất thường mạng.
 * @returns {Promise<object>} - Phản hồi JSON chứa danh sách bất thường.
 */
exports.getAllNetworkAnomalies = async (req, res) => {
    try {
        const anomalies = await getAllNetworkAnomalies();
        return res.status(200).json({ data: anomalies });
    } catch (error) {
        console.error('Error getting network anomalies:', error);
        return res.status(500).json({ message: 'Error getting network anomalies' });
    }
};
/**
 * Lấy phân tích mạng
 *
 * Phân tích log mạng trong khoảng thời gian được chỉ định và trả về dữ liệu biểu đồ.
 * @param {object} req - Yêu cầu HTTP chứa tham số timeStart và timeEnd.
 * @param {object} res - Phản hồi HTTP trả về dữ liệu phân tích.
 * @returns {Promise<object>} - Phản hồi JSON chứa dữ liệu biểu đồ.
 */
exports.getNetworkAnalysis = async (req, res) => {
    try {
        const { timeStart, timeEnd } = req.query;
        const chartData = await logAnalyze(timeStart, timeEnd);
        return res.status(200).json({ data: chartData });
    } catch (error) {
        console.error('Error getting network analysis:', error);
        return res.status(500).json({ message: 'Error getting network analysis' });
    }
};