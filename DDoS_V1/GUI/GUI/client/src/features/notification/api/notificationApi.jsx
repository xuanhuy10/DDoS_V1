import API from '@/utils/api_v2';
import axios from 'axios';

export const getAllNotification = async () => {
  try {
    const response = await API.get('/notification');
    console.log("getAllNotificationAPI", response);
    return response.data;
  } catch (error) {
    throw error;
  }
};

