import React from "react";
import { ComponentStory, ComponentMeta } from "@storybook/react";

import { Modal, ModalType } from "../components/Modal";

// More on default export: https://storybook.js.org/docs/react/writing-stories/introduction#default-export
export default {
  title: "Modal/Modal",
  component: Modal,

  argTypes: {
    // toggle between large and standard modal
    type: {
      control: {
        type: "select",
        options: ["standard", "large"],
      },
    },
  },
} as ComponentMeta<typeof Modal>;

const Template: ComponentStory<typeof Modal> = ({ ...args }) => (
  <Modal {...args}>
    <div className="flex h-full overflow-hidden">
      <div className="m-auto h-full w-full bg-slate-100 text-center align-middle">
        <div className="text-xl">Hello World</div>
      </div>
    </div>
  </Modal>
);

export const Primary = Template.bind({});
// More on args: https://storybook.js.org/docs/react/writing-stories/args
Primary.args = {
  onClose: () => {},
  type: ModalType.Standard,
};

export const Large = Template.bind({});
Large.args = {
  onClose: () => {},
  type: ModalType.Large,
};
