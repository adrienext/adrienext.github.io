# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Static bilingual (EN/FR) personal portfolio for Adrien Amberto. No build system or package manager — pure HTML/CSS/JS. Target deployment: GitHub Pages.

## Development

Serve from `www/` with any static file server:
```bash
cd www && python -m http.server
```
No build, install, or test steps.

## Architecture

### File structure
- `www/index.html` — English (default)
- `www/fr/index.html` — French (all asset paths use `../` prefix)
- `www/css/` — `style.css` (layout + responsive), `dark.css`, `light.css` (theme colors)
- `www/js/script.js` — theme, hamburger menu, scrollspy, slideshow
- `www/img/`, `www/doc/` — shared assets for both languages

### CSS theme system
Three stylesheets: `style.css` for layout, `dark.css` and `light.css` for colors via body class `.dark`/`.light`. Nord palette. Theme toggle is a hidden checkbox; JS swaps body class and persists to `localStorage`. Also detects `prefers-color-scheme` on first visit.

### JavaScript (`js/script.js`)
- **Theme toggle** — localStorage + OS preference detection
- **Hamburger menu** — toggles `.open` class on `.navbar-nav`
- **Scrollspy** — IntersectionObserver, works with any section IDs (both EN/FR)
- **Slideshow** — detects path prefix from stylesheet href to work from both root and `/fr/`

### Responsive
Media queries at 768px (hamburger nav, stacked footer, smaller images) and 480px (tighter spacing). Content grid uses `repeat(auto-fit, minmax(clamp(...), 1fr))` for fluid columns.

### Bilingual setup
Two separate HTML files sharing all CSS/JS/assets. Language switcher in navbar. French version prefixes asset paths with `../`. The JS `<html lang>` attribute can be used to detect current language if needed.
