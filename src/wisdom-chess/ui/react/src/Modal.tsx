import React from 'react'

interface ModalProps {
    children: React.ReactNode
}

const Modal = (props: ModalProps): React.JSX.Element => {
    return (
        <>
            <div className="modal-overlay"></div>
            <div className="modal">{props.children}</div>
        </>
    );
}

export default Modal;
