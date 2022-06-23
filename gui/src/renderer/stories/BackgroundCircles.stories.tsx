import { ComponentStory, ComponentMeta } from "@storybook/react";

import { BackgroundCircles } from "../components/BackgroundCircles/BackgroundCircles";

export default {
  title: "BackgroundCircles/BackgroundCircles",
  component: BackgroundCircles,
  argTypes: {
    backgroundColor: { control: "color" },
  },
} as ComponentMeta<typeof BackgroundCircles>;

const Template: ComponentStory<typeof BackgroundCircles> = ({ ...args }) => (
  <BackgroundCircles {...args}></BackgroundCircles>
);

export const Primary = Template.bind({});
