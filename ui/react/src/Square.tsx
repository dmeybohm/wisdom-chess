import React from "react";
import "./Board.css";

interface Props {
    square: number
    isOddRow: boolean
}

const Square = (props: Props): JSX.Element => {
    return (
        <div
            className={`square ${props.isOddRow ? "odd" : ""}`}
        >
        </div>
    );
}

export default Square;