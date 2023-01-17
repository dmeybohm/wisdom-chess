import React from "react";
import "./Board.css";

interface Props {
    position: string
    isOddRow: boolean
    onClick: () => void;
}

const Square = (props: Props): JSX.Element => {
    return (
        <div
            className={`square ${props.isOddRow ? "odd" : ""} `}
            onClick={props.onClick}
        >
        </div>
    );
}

export default Square;