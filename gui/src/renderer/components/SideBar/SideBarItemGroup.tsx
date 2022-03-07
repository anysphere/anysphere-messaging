import * as React from 'react';


export function SideBarItemGroup(props: {
  title: string,
  children: React.ReactNode
}) {
  return (
    <div className="md-4">
      <div className="left-0">
        {props.title}
      </div>
      {props.children}
    </div>
  );
}