import React, { Ref, useEffect, useRef, useState } from 'react'
import WhiteRook from "./assets/Chess_rlt45.svg";
import DownArrow from "./assets/bxs-down-arrow.svg";
import "./TopMenu.css"

interface TopMenuProps {
    aboutClicked: (event: React.SyntheticEvent) => void;
    newGameClicked: (event: React.SyntheticEvent) => void;
    settingsClicked?: (event: React.SyntheticEvent) => void;
}

function Menu(props: TopMenuProps): JSX.Element {
    return (
        <ul className="menu">
            <li><a onClick={props.newGameClicked}>New Game</a></li>
            <li><a onClick={props.settingsClicked}>Settings</a></li>
            <li><a onClick={props.aboutClicked}>About</a></li>
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
            <div className="top-menu-container">
                <div
                    className="top-menu"
                    ref={menuRef}
                    onClick={toggleOpen}
                >
                    <div className="wisdom-chess-logo">
                        <img src={WhiteRook} width={32} height={32} />
                        <div>
                            Wisdom Chess
                        </div>
                        <img
                            className="menu-arrow"
                            src={DownArrow}
                            width={12}
                            height={12}
                        />
                    </div>
                    <Menu {...props } />
                </div>
            </div>
    )
}

export default TopMenu
