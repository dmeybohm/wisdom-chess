import React, { Ref, useEffect, useRef, useState } from 'react'
import WhiteRook from "./assets/Chess_rlt45.svg";
import DownArrow from "./assets/bxs-down-arrow.svg";

interface TopMenuProps {
    aboutClicked: (event: React.SyntheticEvent) => void;
    newGameClicked: (event: React.SyntheticEvent) => void;
    settingsClicked?: (event: React.SyntheticEvent) => void;
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
    const menuRef : Ref<HTMLDivElement>|null = useRef(null)

    const toggleOpen = (e: React.SyntheticEvent): void => {
        e.preventDefault()
        setIsMenuOpen(!isMenuOpen)
    }

    useEffect(() => {
        const listener = (ev: MouseEvent): void => {
            if (!menuRef.current?.contains(ev.target as Node)) {
                setIsMenuOpen(false)
            }
        }

        document.addEventListener('click', listener)
        return () => document.removeEventListener('click', listener)
    }, [isMenuOpen])

    return (
        <div
            ref={menuRef}
            className="top-menu"
            onClick={toggleOpen}
        >
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
