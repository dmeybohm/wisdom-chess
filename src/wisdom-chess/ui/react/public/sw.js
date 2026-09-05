// Service worker for Wisdom Chess (React front-end).
//
// Replaces the Workbox-generated worker from vite-plugin-pwa with a small
// hand-written one so the front-end does not depend on the Workbox
// toolchain. Strategy:
//
//   - navigations and index.html: network first, cache fallback
//   - content-hashed assets (/assets/*, *.<hash>.js, *.<hash>.wasm, ?v=):
//     cache first; these never change for a given URL
//   - everything else same-origin: stale-while-revalidate
//   - anything under /qml/ is left alone (the Qt build has its own loader)
//
// Bump CACHE_NAME to drop every cached entry on the next activation.

const CACHE_NAME = 'wisdom-chess-v1'

const HASHED_ASSET = /^\/assets\/|\.[A-Za-z0-9_-]{8,}\.(js|wasm|css)$/

self.addEventListener('install', event => {
    event.waitUntil(
        caches.open(CACHE_NAME)
            .then(cache => cache.addAll(['/']))
            .catch(() => undefined)
            .then(() => self.skipWaiting())
    )
})

self.addEventListener('activate', event => {
    event.waitUntil(
        caches.keys()
            .then(keys => Promise.all(
                keys.filter(key => key !== CACHE_NAME).map(key => caches.delete(key))
            ))
            .then(() => self.clients.claim())
    )
})

self.addEventListener('fetch', event => {
    const request = event.request
    if (request.method !== 'GET') {
        return
    }
    const url = new URL(request.url)
    if (url.origin !== self.location.origin || url.pathname.startsWith('/qml')) {
        return
    }

    if (request.mode === 'navigate') {
        event.respondWith(networkFirst(request, '/'))
        return
    }

    if (HASHED_ASSET.test(url.pathname) || url.searchParams.has('v')) {
        event.respondWith(cacheFirst(request))
        return
    }

    event.respondWith(staleWhileRevalidate(request))
})

async function networkFirst(request, fallbackUrl) {
    const cache = await caches.open(CACHE_NAME)
    try {
        const response = await fetch(request)
        if (response.ok) {
            cache.put(fallbackUrl, response.clone())
        }
        return response
    } catch (error) {
        const cached = await cache.match(fallbackUrl)
        if (cached) {
            return cached
        }
        throw error
    }
}

async function cacheFirst(request) {
    const cache = await caches.open(CACHE_NAME)
    const cached = await cache.match(request)
    if (cached) {
        return cached
    }
    const response = await fetch(request)
    if (response.ok) {
        cache.put(request, response.clone())
    }
    return response
}

async function staleWhileRevalidate(request) {
    const cache = await caches.open(CACHE_NAME)
    const cached = await cache.match(request)
    const network = fetch(request)
        .then(response => {
            if (response.ok) {
                cache.put(request, response.clone())
            }
            return response
        })
        .catch(() => undefined)
    if (cached) {
        return cached
    }
    const response = await network
    if (response) {
        return response
    }
    return Response.error()
}
