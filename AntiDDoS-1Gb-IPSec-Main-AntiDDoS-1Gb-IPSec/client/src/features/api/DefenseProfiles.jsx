import API from "@/utils/api_v2";

export const getDefenseProfiles = async (offset) => {
  try {
    const response = await API.get(`defense/profiles/all/${offset}`);
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const getAllActiveDefenseProfiles = async () => {
  try {
    const response = await API.get("defense/profiles/active");
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const getDefenseProfileAttackTypesConfig = async (attackType) => {
  try {
    const response = await API.get(
      `defense/profiles/active/config/${attackType}`
    );
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const getAllDefenseProfilesByUserId = async (userId) => {
  try {
    const response = await API.get(`defense/profiles/user/${userId}`);
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const getAllThresholdByActiveDefenseProfile = async () => {
  try {
    const response = await API.get("defense/profiles/active/attacks/rate");
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const insertDefenseProfile = async (profileData) => {
  try {
    return await API.post("defense/profiles", profileData);
  } catch (error) {
    throw error;
  }
};

export const updateDefenseProfile = async (profileId, profileData) => {
  try {
    const response = await API.patch(
      `defense/profiles/${profileId}`,
      profileData
    );
    return response.data;
  } catch (error) {
    throw error;
  }
};

// NEW APPLY

export const applyDefenseProfileToInterfaces = async (
  profileId,
  interfaces
) => {
  try {
    return await API.post(`defense/profiles/${profileId}/apply`, interfaces);
  } catch (error) {
    throw error;
  }
};

export const deleteDefenseProfile = async (profileId) => {
  try {
    const response = await API.delete(`defense/profiles/${profileId}`);
    return response.data;
  } catch (error) {
    throw error;
  }
};
