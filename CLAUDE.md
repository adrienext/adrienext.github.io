# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Static personal portfolio website for Adrien Amberto. No build system, framework, or package manager — pure HTML/CSS/JS. Previously hosted at `kirikou.alwaysdata.net` (now inactive).

## Development

Serve from `www/` with any static file server:
```bash
cd www && python -m http.server
```
No build, install, or test steps.

## Repository Layout

- **`www/`** — current site (English, 2021-2022 version, most complete)
- **`www/fr/`** — older French translation, shares parent's CSS/JS/images via `../` paths
- **`backup site/`** — archived French snapshot (2020-2021), has a unique `pdp.mp4` video not in `www/`

## Architecture

### CSS Theme System (3 files)

`style.css` handles layout. `dark.css` and `light.css` define color schemes applied via body class (`.dark` / `.light`). Uses Nord palette (`#2e3440`, `#d8dee9`, `#5e81ac`, etc.). Theme toggle is a hidden checkbox (`#checkbox`) — JS swaps the body class and persists to `localStorage`.

### JavaScript (single file: `js/script.js`)

Four features in one file:
1. **Theme toggle** — `localStorage('theme')`, swaps body class
2. **Scrollspy** — scroll listener sets `.active` on nav links by section offset
3. **Form validation** — validates name fields (3-12 chars) and textarea (3-150 chars), highlights errors
4. **Profile slideshow** — cycles `img/pdp/pdp1-4.png` with fade transition every 7s

### Responsive Grid

Content grid uses `repeat(auto-fit, minmax(clamp(...), 1fr))` — reflows without media queries. Footer uses named CSS Grid areas.
