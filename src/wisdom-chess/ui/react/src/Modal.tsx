import React from 'react'

interface ModalProps {
    children: JSX.Element[] | JSX.Element;
}

const Modal = (props: ModalProps): JSX.Element => {
    return (
        <div className="modal">{props.children}</div>
    );
}

export default Modal;
