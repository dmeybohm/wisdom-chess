import React, { useState } from 'react'

interface ModalProps {
    show: boolean;
    children: JSX.Element[] | JSX.Element;
}

const Modal = (props: ModalProps): JSX.Element=> {
    return (
        <>
            {(props.show) ?
                <div className="modal">{props.children}</div> : null }
            {(props.show) ? <div className="modal-overlay"></div> : null }
        </>
    );
}

export default Modal;
