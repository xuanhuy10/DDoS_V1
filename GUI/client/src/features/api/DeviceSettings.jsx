import API from '@/utils/api_v2';

export const getDeviceResourceUsage = async () => {
    try {
        const response = await API.get('/device/resource/usage');
        console.log('getDeviceResourceUsage response:', response.data);
        return response.data;
    } catch (error) {
        console.log('getDeviceResourceUsage error:', error);
        throw error;
    }
}

export const getDeviceDiskUsage = async () => {
    try {
        const response = await API.get('/device/disk/usage');
        console.log('getDeviceDiskUsage response:', response.data);
        return response.data;
    } catch (error) {
        console.log('getDeviceDiskUsage error:', error);
        throw error;
    }
}

export const getDeviceDiskSetting = async () => {
    try {
        const response = await API.get('/device/disk/setting');
        console.log('getDeviceDiskSetting response:', response.data);
        return response.data;
    } catch (error) {
        console.log('getDeviceDiskSetting error:', error);
        throw error;
    }
}

export const updateDeviceDiskSetting = async (data) => {
    try {
        console.log('updateDeviceDiskSetting gửi lên:', data); // Thêm dòng này
        const response = await API.post('/device/disk/setting', data);
        console.log('updateDeviceDiskSetting data:', data);
        console.log('updateDeviceDiskSetting response:', response.data);
        return response.data;
    } catch (error) {
        console.log('updateDeviceDiskSetting error:', error);
        throw error;
    }
}


export const resetSystem = async () => {
    try {
        const response = await API.post('/device/reset');
        console.log('resetSystem response:', response);
        return response;
    } catch (error) {
        console.log('resetSystem error:', error);
        throw error;
    }
};

export const updateDeviceSettings = async (data) => {
    try {
        const response = await API.post('/device/settings', data);
        return response.data;
    } catch (error) {
        throw error;
    }
}
