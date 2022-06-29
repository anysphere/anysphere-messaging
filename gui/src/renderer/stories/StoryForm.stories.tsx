import { ComponentStory, ComponentMeta } from "@storybook/react";

import {StoryForm} from "../components/AddFriend/AddFriend";

export default {
  title: "Friends/StoryForm",
  component: StoryForm,
} as ComponentMeta<typeof StoryForm>;

const Template: ComponentStory<typeof StoryForm> = () => (
  <StoryForm ></StoryForm>
);


export const Primary = Template.bind({});
