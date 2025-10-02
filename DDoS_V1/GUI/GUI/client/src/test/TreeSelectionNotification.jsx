import React, { useState } from 'react';
import { TreeSelect, Button, message } from 'antd';

const TreeSelectExample = () => {
  // Define the items as a tree structure
  const items = [
    { title: 'Item 1', value: '1' },
    { title: 'Item 2', value: '2' },
    { title: 'Item 3', value: '3' },
    { title: 'Item 4', value: '4' },
    { title: 'Item 5', value: '5' },
    { title: 'Item 6', value: '6' },
    { title: 'Item 7', value: '7' },
    { title: 'Item 8', value: '8' },
    { title: 'Item 9', value: '9' },
  ];

  // State management for selected items and the TreeSelect visibility
  const [selectedItems, setSelectedItems] = useState([]);
  const [treeOpen, setTreeOpen] = useState(false);

  // Function to handle selection change
  const onChange = (value) => {
    if (value.length <= 9) {
      setSelectedItems(value);
    } else {
      // message.warning('You can only select up to 5 items!');
    }
  };

  // Convert the items into the format that TreeSelect expects
  const treeData = items.map(item => ({
    title: item.title,
    value: item.value,
  }));

  return (
    <div style={{ padding: '20px' }}>
      <Button onClick={() => setTreeOpen(true)}>Open TreeSelect</Button>

      {treeOpen && (
        <div
          style={{
            marginTop: '10px',
            padding: '10px',
            width:'200px',
            border: '1px solid #ddd',
            backgroundColor: '#fafafa',
          }}
        >
          <h4>Select up items:</h4>
          <TreeSelect
            treeData={treeData}
            value={selectedItems}
            onChange={onChange}
            treeCheckable
            showCheckedStrategy={TreeSelect.SHOW_PARENT}
            maxTagCount={5}
            placeholder="Please select"
            style={{ width: '100%' }}
          />

          <Button
            style={{ marginTop: '10px' }}
            onClick={() => setTreeOpen(false)}
          >
            Done
          </Button>
        </div>
      )}

      <div style={{ marginTop: '20px' }}>
        <h4>Selected Items:</h4>
        {selectedItems.length > 0 ? (
          <ul>
            {selectedItems.map((itemId) => {
              const item = items.find((i) => i.value === itemId);
              return <li key={itemId}>{item.title}</li>;
            })}
          </ul>
        ) : (
          <p>No items selected.</p>
        )}
      </div>
    </div>
  );
};

export default TreeSelectExample;
