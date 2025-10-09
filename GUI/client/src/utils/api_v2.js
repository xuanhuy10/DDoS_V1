import axios from "axios";

const axiosInstance = axios.create({
    baseURL: import.meta.env.VITE_REACT_APP_API_ENDPOINT_V1,
    withCredentials: true,
    headers: {
        "Content-type": "application/json",
    },
});

// Add a request interceptor to attach the Authorization header
axiosInstance.interceptors.request.use(
    (request) => {
        const accessToken = localStorage.getItem("access_token");
        if (accessToken) {
            request.headers["Authorization"] = `Bearer ${accessToken}`; // Standard "Bearer" format
        }
        return request;
    },
    (error) => {
        return Promise.reject(error);
    }
);

// Add a response interceptor to handle token refreshing
axiosInstance.interceptors.response.use(
    (response) => response, // Pass through successful responses
    async (error) => {
        const originalRequest = error.config;

        if (error.response?.status === 401 && !originalRequest._retry && !originalRequest.url.includes("/auth/refresh")) {
            originalRequest._retry = true; 
            try {
                const response = await axiosInstance.post("/auth/refresh");

                const { access_token } = response.data;

                if (access_token) {
                    localStorage.setItem("access_token", access_token);
                    originalRequest.headers["Authorization"] = `Bearer ${access_token}`;
                    return axiosInstance(originalRequest); // Retry the original request
                }
            } catch (refreshError) {
                localStorage.removeItem("access_token");
                window.location.href = "/auth/login";
                return Promise.reject(refreshError);
            }
        }
        return Promise.reject(error);
    }
);

export default axiosInstance;