import React from 'react';
import { ComponentStory, ComponentMeta } from '@storybook/react';

import Modal from '../components/Modal';

// More on default export: https://storybook.js.org/docs/react/writing-stories/introduction#default-export
export default {
  title: 'Modal/Modal',
  component: Modal,
  
  argTypes: {
    backgroundColor: { control: 'color' },
  },
} as ComponentMeta<typeof Modal>;

// More on component templates: https://storybook.js.org/docs/react/writing-stories/introduction#using-args
const Template: ComponentStory<typeof Modal> = (args) => <Modal {...args} />;

const Template2: ComponentStory<typeof Modal> = ({...args}) => (
  <Modal {...args}>
    <div className='justify-center text-center'>Hello World</div>
  </Modal>
);


export const Primary = Template2.bind({});
// More on args: https://storybook.js.org/docs/react/writing-stories/args
Primary.args = {
  onClose: () => {},
};