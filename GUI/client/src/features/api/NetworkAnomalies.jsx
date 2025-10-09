import API from '@/utils/api_v2';

export const getAllNetworkAnomalies = async () => {
    try {
        const response = await API.get('network/anomalies');
        return response.data;
    } catch (error) {
        throw error;
    }
}

export const getNetworkAnalysis = async () => {
    try {
        const response = await API.get('network/analysis');
        return response.data;
    } catch (error) {
        throw error;
    }
}