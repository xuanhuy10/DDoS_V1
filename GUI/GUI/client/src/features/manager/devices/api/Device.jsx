import API from "@/utils/api_v2";
export const getDataSystem = async (data) =>{
    try {
        const response = await API.get('/manager/device');
        return response.data;    
    }catch(error){
        throw error;
    }

    
}

