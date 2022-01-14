import * as React from 'react';
import { Message } from '../types';

function Read(props: { message: Message }) {
  return (
    <>
      <h1>message!</h1>
      <div>
        {props.message.from}: {props.message.message}
      </div>
    </>
  );
}

export default Read;
