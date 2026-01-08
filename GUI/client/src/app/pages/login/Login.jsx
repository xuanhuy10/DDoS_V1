import React, { useState, useEffect } from "react";
import { LockOutlined, UserOutlined } from "@ant-design/icons";
import { Button, Form, Input, message } from "antd";
import { useNavigate } from "react-router-dom";

import { useAuth } from "@/hooks/useAuth";
import { loginUser } from "@/features/api/Auth";

import logo from "@/assets/ddos_logo.svg";
import "@/features/login/main.css";

const Login = () => {
  const [loading, setLoading] = useState(false);
  const navigate = useNavigate();

  const { user, login } = useAuth();

  useEffect(() => {
    if (user) {
      navigate("/dashboard");
    }
  }, [user, navigate]);

  const onFinish = async (values) => {
    const { username, password } = values;
    try {
      setLoading(true);
      const response = await loginUser({ username, password });
      if (response) {
        login(response);
        navigate("/dashboard");
      }
    } catch (error) {
      console.error("Failed to login:", error);
      message.error(
        error.response?.data?.message || "Login failed. Please try again."
      );
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="login-container">
      <Form
        name="login"
        initialValues={{
          remember: true,
        }}
        onFinish={onFinish}
        className="login-form"
      >
        <img src={logo} alt="logo" />
        <Form.Item
          name="username"
          rules={[{ required: true, message: "Please input your Username!" }]}
        >
          <Input
            prefix={<UserOutlined />}
            placeholder="Username"
            styles={{ activeBg: "red" }}
          />
        </Form.Item>
        <Form.Item
          name="password"
          rules={[{ required: true, message: "Please input your Password!" }]}
        >
          <Input.Password prefix={<LockOutlined />} placeholder="Password" />
        </Form.Item>
        <Form.Item>
          <Button
            type="primary"
            htmlType="submit"
            block
            className="login-form-btn"
            loading={loading}
          >
            LOG IN
          </Button>
        </Form.Item>
      </Form>
    </div>
  );
};

export default Login;
