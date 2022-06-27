import React from "react";
import { ComponentStory, ComponentMeta } from "@storybook/react";

import LegacyFriendsModal from "../components/FriendsModal";

export default {
  title: "Modal/LegacyFriendsModal",
  component: LegacyFriendsModal,

  argTypes: {
    // boolean
    onClose: { control: "boolean" },
    // string
    onAddFriend: { control: "text" },
  },
} as ComponentMeta<typeof LegacyFriendsModal>;

const Template: ComponentStory<typeof LegacyFriendsModal> = ({ ...args }) => {
  const closeModal = React.useCallback(() => {
    console.log("close modal");
  }, []);

  const onAddFriend = React.useCallback(() => {
    console.log("add friend");
  }, []);

  return <LegacyFriendsModal onClose={closeModal} onAddFriend={onAddFriend} />;
};

export const Default = Template.bind({});
Default.args = {};
