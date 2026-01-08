import axios from "axios";

const axiosInstance = axios.create({
  baseURL: import.meta.env.VITE_REACT_APP_API_ENDPOINT_V1,
  withCredentials: true,
  headers: {
    "Content-type": "application/json",
  },
});

//login route
export const loginUser = async (data) => {
  try {
    const response = await axiosInstance.post("/auth/login", data);
    console.log("loginUser response:", response);
    return response.data;
  } catch (error) {
    throw error;
  }
};

//refresh token route
export const refreshToken = async () => {
  try {
    const response = await axiosInstance.post("/auth/refresh");
    return response.data;
  } catch (error) {
    throw error;
  }
};
