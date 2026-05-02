# GNT Group — Design System

## Color Palette
- Background: #040a06 (near-black, green-tinted)
- Surface: rgba(255,255,255,0.04) glassmorphism
- Primary: #22c55e (natural green)
- Primary dark: #16a34a
- Primary light: #4ade80
- Spectrum accent: full rainbow (red → orange → yellow → green → teal → blue → purple → pink)
- Text: #edf7ee
- Muted: #9db09e

## Typography
- Font: System UI / Segoe UI (no external fonts needed)
- Hero: 900 weight, tight letter-spacing (-0.025em)
- Section titles: 800 weight
- Body: 400 weight, 1.6 line-height

## Design Language
- Dark theme with deep green-black background
- Glassmorphism panels (backdrop-filter blur)
- Gradient accents (green spectrum for brand, rainbow spectrum for EXBERRY product range)
- Subtle particle/orb animations
- Card hover: translateY(-4px) + border glow
- Scroll-triggered reveal animations

## Elevation
- Cards: 1px border rgba(255,255,255,0.08) + backdrop blur
- Hover state: border-color rgba(34,197,94,0.35) + box-shadow
- Buttons: gradient background + glow shadow

## Motion Principles
- Ease: cubic-bezier(0.16, 1, 0.3, 1) — fast out, smooth settle
- Reveal: translateY(40px) → 0, opacity 0 → 1, duration 0.8s
- Counters: count up animation on scroll into view
- Bars: width 0 → target on reveal
- Particles: slow, organic floating movement
