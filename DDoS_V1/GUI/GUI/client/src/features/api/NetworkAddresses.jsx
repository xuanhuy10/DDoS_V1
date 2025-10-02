import API from "@/utils/api_v2";

export const getAllProtectedAddresses = async () => {
  try {
    const response = await API.get("defense/address/common/white");
    return response.data;
  } catch (error) {
    throw error;
  }
};
export const getAllBlockedAddresses = async () => {
  try {
    const response = await API.get("defense/address/common/black");
    return response.data;
  } catch (error) {
    throw error;
  }
};
export const getAllVPNAllowedAddresses = async () => {
  try {
    const response = await API.get("defense/address/vpn/white");
    return response.data;
  } catch (error) {
    throw error;
  }
};
export const getAllHTTPBlockedAddresses = async () => {
  try {
    const response = await API.get("defense/address/http/black");
    return response.data;
  } catch (error) {
    throw error;
  }
};
export const insertProtectedAddresses = async (addressData) => {
  try {
    const response = await API.post(
      "defense/address/common/white",
      addressData
    );
    return response.data;
  } catch (error) {
    throw error;
  }
};
export const insertBlockedAddresses = async (addressData) => {
  try {
    const response = await API.post(
      "defense/address/common/black",
      addressData
    );
    return response.data;
  } catch (error) {
    throw error;
  }
};
export const insertVPNAllowedAddresses = async (addressData) => {
  try {
    const response = await API.post("defense/address/vpn/white", addressData);
    return response.data;
  } catch (error) {
    throw error;
  }
};
export const insertHTTPBlockedAddresses = async (addressData) => {
  try {
    const response = await API.post("defense/address/http/black", addressData);
    return response.data;
  } catch (error) {
    throw error;
  }
};
export const deleteNetworkAddressesByIds = async (addressIds) => {
  try {
    const response = await API.post("defense/address/bulk-delete", {
      addressIds,
    });
    return response.data;
  } catch (error) {
    throw error;
  }
};
export const deleteNetworkAddressesByAddressAndVersionList = async (ipList) => {
  try {
    return await API.post("defense/address/bulk-delete-by-file", ipList);
  } catch (error) {
    throw error;
  }
};
export const importDeleteBulkHTTPBlockedAddresses = async (ipList) => {
  try {
    const response = await API.post(
      "defense/address/http/black/import-delete",
      ipList
    );
    return response.data;
  } catch (error) {
    throw error;
  }
};
export const importDeleteBulkVPNAllowedAddresses = async (ipList) => {
  try {
    return API.post("defense/address/vpn/white/import-delete", ipList);
  } catch (error) {
    throw error;
  }
};
export const importDeleteBulkBlockedAddresses = async (ipList) => {
  try {
    const response = await API.post(
      "defense/address/blocked/import-delete",
      ipList
    );
    return response.data;
  } catch (error) {
    throw error;
  }
};
export const importDeleteBulkProtectedAddresses = async (ipList) => {
  try {
    const response = await API.post(
      "defense/address/common/white/import-delete",
      ipList
    );
    return response.data;
  } catch (error) {
    throw error;
  }
};
