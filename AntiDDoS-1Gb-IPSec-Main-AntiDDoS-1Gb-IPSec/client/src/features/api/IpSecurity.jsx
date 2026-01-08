import API from "@/utils/api_v2";

export const getIpSecurity = async (offset = 0) => {
  try {
    const response = await API.get(`defense/ipsec/all/${offset}`);
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const updateIpSecProfile = async (profileId, profileData) => {
  try {
    const mappedData = {
      ProfileName: profileData.IpSecProfileName || profileData.ProfileName,
      ProfileDescription:
        profileData.IpSecProfileDescription || profileData.ProfileDescription,
      LocalGateway: profileData.LocalGateway,
      SubnetLocalGateway: profileData.SubnetLocalGateway,
      RemoteGateway: profileData.RemoteGateway,
      SubnetRemoteGateway: profileData.SubnetRemoteGateway,
      ConnectionCount: profileData.ConnectionCount || 1,
      IKEVersion: profileData.IKEVersion,
      Mode: profileData.IKEMode || profileData.Mode,
      ESPAHProtocol: profileData.ESPAHProtocol,
      IKEReauthTime: profileData.IKEReauthTime
        ? parseInt(profileData.IKEReauthTime, 10)
        : null,
      EncryptionAlgorithm: profileData.EncryptionAlgorithm,
      HashAlgorithm: profileData.HashAlgorithm,
      ReKeyTime: profileData.ReKeyTime
        ? parseInt(profileData.ReKeyTime, 10)
        : null,
      Enable: profileData.Enable,
    };
    return await API.patch(`defense/ipsec/${profileId}`, mappedData);
  } catch (error) {
    throw error;
  }
};

export const deleteIpSecProfile = async (profileId) => {
  try {
    const response = await API.delete(`defense/ipsec/${profileId}`);
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const insertIpSecProfileAndCertificates = async (
  profileData,
  uploadedFiles
) => {
  try {
    const formData = new FormData();

    // Add certificate files to FormData
    const requiredFiles = ["ca-cert", "cert", "private-key"];
    for (const key of requiredFiles) {
      if (
        uploadedFiles[key] &&
        uploadedFiles[key].content &&
        uploadedFiles[key].name
      ) {
        const blob = new Blob([uploadedFiles[key].content], {
          type: "application/pkix-cert",
        });
        formData.append(key, blob, uploadedFiles[key].name);
      } else {
        throw new Error(`Missing or invalid file: ${key}`);
      }
    }

    // Add profile data to FormData
    const mappedData = {
      ProfileName: profileData.ProfileName,
      ProfileDescription: profileData.ProfileDescription,
      LocalGateway: profileData.LocalGateway,
      SubnetLocalGateway: profileData.SubnetLocalGateway,
      ConnectionCount: profileData.ConnectionCount || 1,
      RemoteGateway: profileData.RemoteGateway,
      SubnetRemoteGateway: profileData.SubnetRemoteGateway,
      IKEVersion: profileData.IKEVersion,
      Mode: profileData.Mode,
      ESPAHProtocol: profileData.ESPAHProtocol,
      IKEReauthTime: profileData.IKEReauthTime
        ? parseInt(profileData.IKEReauthTime)
        : null,
      EncryptionAlgorithm: profileData.EncryptionAlgorithm,
      HashAlgorithm: profileData.HashAlgorithm,
      ReKeyTime: profileData.ReKeyTime ? parseInt(profileData.ReKeyTime) : null,
      Enable: profileData.Enable || false,
    };
    formData.append("profileData", JSON.stringify(mappedData));

    return await API.post("defense/ipsec/combined", formData, {
      headers: { "Content-Type": "multipart/form-data" },
    });
  } catch (error) {
    throw error;
  }
};
