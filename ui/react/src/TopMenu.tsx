import React, { useState } from 'react'
import WhiteRook from "./assets/Chess_rlt45.svg";
import DownArrow from "./assets/bxs-down-arrow.svg";

interface TopMenuProps {
    aboutClicked: () => void;
    newGameClicked: () => void;
    settingsClicked?: () => void;
}

function Menu(props: TopMenuProps): JSX.Element {
    return (
        <ul className="menu">
            <li onClick={props.newGameClicked}>New Game</li>
            <li onClick={props.settingsClicked}>Settings</li>
            <li onClick={props.aboutClicked}>About</li>
        </ul>
    );
}

function TopMenu(props: TopMenuProps) {
    const [isMenuOpen, setIsMenuOpen] = useState(false);

    return (
        <div className="top-menu" onClick={() => setIsMenuOpen(!isMenuOpen)}>
            <img src={WhiteRook} width={32} height={32} />
            <div className="">
                Wisdom Chess
            </div>
            <img src={DownArrow} width={12} height={12}/>
            {(isMenuOpen) ? <Menu {... props} /> : null}
        </div>
    )
}

export default TopMenu
