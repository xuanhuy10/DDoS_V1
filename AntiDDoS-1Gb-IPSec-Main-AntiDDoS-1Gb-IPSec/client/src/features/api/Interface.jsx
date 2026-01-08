import API from '@/utils/api';
import axios from 'axios';

export const getInterfaces = async () => {
    try {
        const response = await API.get('/interfaces');
        return response.data;
    } catch (error) {
      throw error;
    }
  };
