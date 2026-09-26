// Whether the primary pointer is a mouse or a trackpad rather than a
// finger. Dragging pieces is for those; a finger taps the two squares.
// A browser without the media query is taken to have a mouse.
export function hasFinePointer(): boolean {
    if (typeof window === 'undefined' || typeof window.matchMedia !== 'function') {
        return true
    }
    return window.matchMedia('(pointer: fine)').matches
}
