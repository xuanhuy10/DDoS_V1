import API from '@/utils/api_v2';

export const getAllDeviceInterfaces = async () => {
    try {
        const response = await API.get('defense/interfaces');
        return response.data;
    } catch (error) {
        throw error;
    }
}

export const getMirroringDeviceInterfaces = async () => {
    try {
        const response = await API.get('defense/interfaces/mirroring');
        return response.data;
    } catch (error) {
        throw error;
    }
}

export const updateDeviceInterface = async (interfaceId, data) => {
    try {
        // Nếu data đã là mảng ports thì giữ nguyên, nếu là object thì đóng gói lại
        const payload = Array.isArray(data.ports)
            ? data
            : { ports: [data] };
        return await API.patch(`defense/interfaces/${interfaceId}`, payload);
    } catch (error) {
        throw error;
    }
}

export const updateMirroringDeviceInterface = async (MirrorInterfaceId, data) => {
    try {
        return await API.patch(`defense/interfaces/mirroring/${MirrorInterfaceId}`, data);
    } catch (error) {
        throw error;
    }
}

export const createMirroringDeviceInterface = async (data) => {
    try {
        return await API.post('defense/interfaces/mirroring', data);
    } catch (error) {
        throw error;
    }
}

export const deleteMirroringDeviceInterface = async (MirrorInterfaceId) => {
    try {
        return await API.delete(`defense/interfaces/mirroring/${MirrorInterfaceId}`);
    } catch (error) {
        throw error;
    }
}

