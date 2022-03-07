import React from 'react';
import { ComponentStory, ComponentMeta } from '@storybook/react';

import { SideBar } from '../components/SideBar/SideBar';


// More on default export: https://storybook.js.org/docs/react/writing-stories/introduction#default-export
export default {
  title: 'SideBar/SideBar',
  component: SideBar,
  
  argTypes: {
    backgroundColor: { control: 'color' },
  },
} as ComponentMeta<typeof SideBar>;

// More on component templates: https://storybook.js.org/docs/react/writing-stories/introduction#using-args
const Template: ComponentStory<typeof SideBar> = ({...args}) => (
  <SideBar {...args}>
  </SideBar>
);

export const Primary = Template.bind({});

const [open, setOpen] = React.useState(true);
// More on args: https://storybook.js.org/docs/react/writing-stories/args
Primary.args = {
  title: 'SideBar',
  open: open,
  setOpen: (x: boolean) => {}
  // setOpen: (o) => setOpen(o),
};