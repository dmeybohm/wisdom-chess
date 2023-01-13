import React, { useState } from 'react'
import WhiteRook from "./assets/Chess_rlt45.svg";
import DownArrow from "./assets/bxs-down-arrow.svg";

function TopMenu() {
    return (
        <div className="top-menu">
            <img src={WhiteRook} width={32} height={32} />
            <div className="">
                Wisdom Chess
            </div>
            <img src={DownArrow} width={12} height={12}/>
        </div>
    )
}

export default TopMenu
