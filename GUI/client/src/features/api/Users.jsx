import API from "@/utils/api_v2";

export const getAllUsers = async () => {
  try {
    const response = await API.get("users/");
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const getUserById = async (userId) => {
  try {
    const response = await API.get(`users/${userId}`);
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const getLoggedInUser = async () => {
  try {
    const response = await API.get("users/session/me");
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const insertUser = async (data) => {
  try {
    const response = await API.post("users/", data);
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const updateUser = async (userId, data) => {
  try {
    const response = await API.patch(`users/${userId}`, data);
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const deleteUser = async (userId) => {
  try {
    const response = await API.delete(`users/${userId}`);
    return response.data;
  } catch (error) {
    throw error;
  }
};
