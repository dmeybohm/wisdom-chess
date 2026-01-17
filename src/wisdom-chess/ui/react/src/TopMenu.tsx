import React, { Ref, useEffect, useRef, useState } from 'react'
import WhiteRook from "./assets/Chess_rlt45.svg";
import DownArrow from "./assets/bxs-down-arrow.svg";
import "./TopMenu.css"

interface TopMenuProps {
    aboutClicked: (event: React.SyntheticEvent) => void;
    newGameClicked: (event: React.SyntheticEvent) => void;
    settingsClicked?: (event: React.SyntheticEvent) => void;
}

interface ListMenuProps extends TopMenuProps {
    isMenuOpen: boolean
}

type LogoAndBrandProps = {
    isMobile: boolean
    toggleOpen?: (e: React.SyntheticEvent) => void
    menuRef?: Ref<HTMLDivElement>
}

function LogoAndBrand(props: LogoAndBrandProps) {
    return (
        <div
            ref={props.menuRef}
            onClick={props.toggleOpen}
            className={`wisdom-chess-logo ${props.isMobile ? 'is-mobile' : 'is-desktop'}`}>
            <img src={WhiteRook} width={32} height={32} />
            <div>
                Wisdom Chess
            </div>
            {props.isMobile &&
                <img
                    className="menu-arrow"
                    src={DownArrow}
                    width={12}
                    height={12}
                />
            }
        </div>
    )
}

function Menu(props: ListMenuProps): JSX.Element {
    return (
        <ul className={`menu ${props.isMenuOpen ? 'is-open' : ''}`}>
            <li><a onClick={props.newGameClicked}>New Game</a></li>
            <li><a onClick={props.settingsClicked}>Settings</a></li>
            <li><a onClick={props.aboutClicked}>About</a></li>
            <li><a href="/qml/" target="_blank" rel="noopener">QML Version</a></li>
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

    const listMenuProps = {
        ...props,
        isMenuOpen
    }

    return (
            <div className="top-menu-container">
                <div
                    className="top-menu"
                >
                    <LogoAndBrand isMobile={false} />
                    <LogoAndBrand isMobile={true} toggleOpen={toggleOpen} menuRef={menuRef} />
                    <Menu { ...listMenuProps } />
                </div>
            </div>
    )
}

export default TopMenu
