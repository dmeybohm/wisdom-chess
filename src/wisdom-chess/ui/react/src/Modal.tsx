import React from 'react'

interface ModalProps {
    children: React.ReactNode
}

const Modal = (props: ModalProps): JSX.Element => {
    return (
        <>
            <div className="modal-overlay"></div>
            <div className="modal">{props.children}</div>
        </>
    );
}

export default Modal;
