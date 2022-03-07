import * as React from 'react';


/* Defining the properties of the component. */
export interface SideBarItemProps {
  title: string;
  // icon: string;
  onClick: () => void;
}

export function SideBarItem(props: SideBarItemProps) {
    return (
      <div className="" onClick={props.onClick}>
        <div className="">
          <i className=""></i>
        </div>
        <div className="">
          {props.title}
        </div>
      </div>
    );
}