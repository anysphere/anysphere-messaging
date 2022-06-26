import React from "react";
import { ComponentStory, ComponentMeta } from "@storybook/react";

import FriendsModal from "../components/FriendsModal";

export default {
  title: "Modals/FriendsModal",
  component: FriendsModal,

  argTypes: {
    backgroundColor: { control: "color" },
  },
} as ComponentMeta<typeof FriendsModal>;

const Template: ComponentStory<typeof FriendsModal> = ({ ...args }) => {
  const [modal, setModal] = React.useState<JSX.Element | null>(null);

  const closeModal = React.useCallback(() => {
    setModal(null);
  }, [setModal]);

  return <FriendsModal {...args}></FriendsModal>;
};
