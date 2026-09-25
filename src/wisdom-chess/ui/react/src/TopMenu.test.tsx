import { describe, it, expect, vi } from 'vitest'
import { render } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import TopMenu from './TopMenu'

describe('TopMenu', () => {
    const renderMenu = () => render(
        <TopMenu newGameClicked={vi.fn()} settingsClicked={vi.fn()} aboutClicked={vi.fn()} />
    )
    const isOpen = () => document.querySelector('.menu.is-open') !== null
    const toggle = () => document.querySelector('.wisdom-chess-logo.is-mobile')!

    it('opens and closes the mobile menu from the logo', async () => {
        const user = userEvent.setup()
        renderMenu()

        await user.click(toggle())
        expect(isOpen()).toBe(true)
        await user.click(toggle())
        expect(isOpen()).toBe(false)
    })

    it('closes the mobile menu on a click elsewhere', async () => {
        const user = userEvent.setup()
        renderMenu()

        await user.click(toggle())
        await user.click(document.body)

        expect(isOpen()).toBe(false)
    })
})
