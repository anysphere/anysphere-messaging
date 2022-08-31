import { ComponentStory, ComponentMeta } from "@storybook/react";

import AddFriend from "../components/AddFriend/AddFriend";
import AddFriend2 from "./AddFriendHidden";

export default {
  title: "Friends/AddFriend",
  component: AddFriend,
} as ComponentMeta<typeof AddFriend>;

const Template: ComponentStory<typeof AddFriend> = () => (
  <AddFriend></AddFriend>
);

const Template2: ComponentStory<typeof AddFriend> = () => (
  <AddFriend2></AddFriend2>
);

export const Primary = Template.bind({});
export const MoreHidden = Template2.bind({});
