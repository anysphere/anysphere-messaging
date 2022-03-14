import React from "react";
import { ComponentStory, ComponentMeta } from "@storybook/react";

import { SideBar } from "../components/SideBar/SideBar";
import { SideBarButton } from "../components/SideBar/SideBarProps";

// More on default export: https://storybook.js.org/docs/react/writing-stories/introduction#default-export
export default {
  title: "SideBar/SideBar",
  component: SideBar,

  argTypes: {
    backgroundColor: { control: "color" },
  },
} as ComponentMeta<typeof SideBar>;

// More on component templates: https://storybook.js.org/docs/react/writing-stories/introduction#using-args
const Template: ComponentStory<typeof SideBar> = ({ ...args }) => (
  <SideBar {...args}></SideBar>
);

export const Primary = Template.bind({});

const sideBarCallback = (b: SideBarButton) => {
  switch (b) {
    case SideBarButton.INBOX:
      return React.useCallback(() => {
        console.log("Inbox");
      }, []);
    case SideBarButton.OUTBOX:
      return React.useCallback(() => {
        console.log("Outbox");
      }, []);
    case SideBarButton.SENT:
      return React.useCallback(() => {
        console.log("Sent");
      }, []);
    default:
      return React.useCallback(() => {}, []);
  }
};

// More on args: https://storybook.js.org/docs/react/writing-stories/args
Primary.args = {
  title: "SideBar",
  open: true,
  setOpen: () => {},
  sideBarCallback,
};
