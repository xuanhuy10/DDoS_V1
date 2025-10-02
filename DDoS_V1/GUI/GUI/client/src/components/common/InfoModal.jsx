import { Modal } from 'antd';
import { Children } from 'react';

export const InfoModal = ({ title, visible, setVisible, children }) => {
    return (
        <Modal
            title={title}
            open={visible}
            onOk={() => setVisible(false)}
            onCancel={() => setVisible(false)}
            footer={null}
        >
            {children}
        </Modal>
    );
};