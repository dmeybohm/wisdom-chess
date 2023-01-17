import React from "react";
import "./Board.css";

interface Props {
    position: string
    isOddRow: boolean
    onClick: (position: string) => void;
}

const Square = (props: Props): JSX.Element => {
    return (
        <div
            className={`square ${props.isOddRow ? "odd" : ""} `}
            onClick={() => {
                props.onClick(props.position)
            }}
        >
        </div>
    );
}

export default Square;