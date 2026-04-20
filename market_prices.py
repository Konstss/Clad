"""
Market price database for popular laptops and smartphones.
Prices represent the typical used-market price for a device in 'gut' (good) condition.
The analyzer applies condition multipliers on top of these base prices.
"""

from __future__ import annotations
import re
from dataclasses import dataclass


@dataclass
class DevicePrice:
    name: str
    base_price: float          # fair used-market price (gut condition)
    keywords: list[str]        # keywords to match in listing title
    category: str              # "laptop" | "smartphone" | "tablet"


# ---------------------------------------------------------------------------
# Smartphones
# ---------------------------------------------------------------------------
SMARTPHONE_PRICES: list[DevicePrice] = [
    # iPhone
    DevicePrice("iPhone 16 Pro Max", 1050, ["iphone 16 pro max"], "smartphone"),
    DevicePrice("iPhone 16 Pro", 900, ["iphone 16 pro"], "smartphone"),
    DevicePrice("iPhone 16 Plus", 780, ["iphone 16 plus"], "smartphone"),
    DevicePrice("iPhone 16", 700, ["iphone 16"], "smartphone"),
    DevicePrice("iPhone 15 Pro Max", 900, ["iphone 15 pro max"], "smartphone"),
    DevicePrice("iPhone 15 Pro", 780, ["iphone 15 pro"], "smartphone"),
    DevicePrice("iPhone 15 Plus", 650, ["iphone 15 plus"], "smartphone"),
    DevicePrice("iPhone 15", 580, ["iphone 15"], "smartphone"),
    DevicePrice("iPhone 14 Pro Max", 750, ["iphone 14 pro max"], "smartphone"),
    DevicePrice("iPhone 14 Pro", 640, ["iphone 14 pro"], "smartphone"),
    DevicePrice("iPhone 14 Plus", 520, ["iphone 14 plus"], "smartphone"),
    DevicePrice("iPhone 14", 460, ["iphone 14"], "smartphone"),
    DevicePrice("iPhone 13 Pro Max", 580, ["iphone 13 pro max"], "smartphone"),
    DevicePrice("iPhone 13 Pro", 500, ["iphone 13 pro"], "smartphone"),
    DevicePrice("iPhone 13 Mini", 320, ["iphone 13 mini"], "smartphone"),
    DevicePrice("iPhone 13", 400, ["iphone 13"], "smartphone"),
    DevicePrice("iPhone 12 Pro Max", 400, ["iphone 12 pro max"], "smartphone"),
    DevicePrice("iPhone 12 Pro", 340, ["iphone 12 pro"], "smartphone"),
    DevicePrice("iPhone 12 Mini", 210, ["iphone 12 mini"], "smartphone"),
    DevicePrice("iPhone 12", 280, ["iphone 12"], "smartphone"),
    DevicePrice("iPhone 11 Pro Max", 290, ["iphone 11 pro max"], "smartphone"),
    DevicePrice("iPhone 11 Pro", 240, ["iphone 11 pro"], "smartphone"),
    DevicePrice("iPhone 11", 200, ["iphone 11"], "smartphone"),
    DevicePrice("iPhone SE 3", 220, ["iphone se 3", "iphone se (3"], "smartphone"),
    DevicePrice("iPhone SE 2", 150, ["iphone se 2", "iphone se (2"], "smartphone"),
    DevicePrice("iPhone XS Max", 180, ["iphone xs max"], "smartphone"),
    DevicePrice("iPhone XS", 150, ["iphone xs"], "smartphone"),
    DevicePrice("iPhone XR", 130, ["iphone xr"], "smartphone"),
    DevicePrice("iPhone X", 120, ["iphone x "], "smartphone"),

    # Samsung Galaxy S
    DevicePrice("Samsung Galaxy S25 Ultra", 1050, ["galaxy s25 ultra", "s25 ultra"], "smartphone"),
    DevicePrice("Samsung Galaxy S25+", 850, ["galaxy s25+", "s25+", "galaxy s25 plus", "s25 plus"], "smartphone"),
    DevicePrice("Samsung Galaxy S25", 700, ["galaxy s25", "s25 5g"], "smartphone"),
    DevicePrice("Samsung Galaxy S24 Ultra", 900, ["galaxy s24 ultra", "s24 ultra"], "smartphone"),
    DevicePrice("Samsung Galaxy S24+", 720, ["galaxy s24+", "s24+", "galaxy s24 plus"], "smartphone"),
    DevicePrice("Samsung Galaxy S24", 580, ["galaxy s24", "s24 5g"], "smartphone"),
    DevicePrice("Samsung Galaxy S23 Ultra", 680, ["galaxy s23 ultra", "s23 ultra"], "smartphone"),
    DevicePrice("Samsung Galaxy S23+", 530, ["galaxy s23+", "s23+", "galaxy s23 plus"], "smartphone"),
    DevicePrice("Samsung Galaxy S23", 420, ["galaxy s23", "s23 5g"], "smartphone"),
    DevicePrice("Samsung Galaxy S22 Ultra", 520, ["galaxy s22 ultra", "s22 ultra"], "smartphone"),
    DevicePrice("Samsung Galaxy S22+", 380, ["galaxy s22+", "galaxy s22 plus"], "smartphone"),
    DevicePrice("Samsung Galaxy S22", 300, ["galaxy s22", "s22 5g"], "smartphone"),
    DevicePrice("Samsung Galaxy S21 Ultra", 380, ["galaxy s21 ultra", "s21 ultra"], "smartphone"),
    DevicePrice("Samsung Galaxy S21+", 290, ["galaxy s21+", "galaxy s21 plus"], "smartphone"),
    DevicePrice("Samsung Galaxy S21", 220, ["galaxy s21", "s21 5g"], "smartphone"),
    DevicePrice("Samsung Galaxy S20 Ultra", 270, ["galaxy s20 ultra", "s20 ultra"], "smartphone"),
    DevicePrice("Samsung Galaxy S20+", 210, ["galaxy s20+", "galaxy s20 plus"], "smartphone"),
    DevicePrice("Samsung Galaxy S20", 170, ["galaxy s20", "s20 5g"], "smartphone"),

    # Samsung Galaxy A/Z
    DevicePrice("Samsung Galaxy Z Fold 6", 1400, ["galaxy z fold 6", "z fold6"], "smartphone"),
    DevicePrice("Samsung Galaxy Z Fold 5", 1050, ["galaxy z fold 5", "z fold5"], "smartphone"),
    DevicePrice("Samsung Galaxy Z Flip 6", 680, ["galaxy z flip 6", "z flip6"], "smartphone"),
    DevicePrice("Samsung Galaxy Z Flip 5", 500, ["galaxy z flip 5", "z flip5"], "smartphone"),
    DevicePrice("Samsung Galaxy A55", 280, ["galaxy a55"], "smartphone"),
    DevicePrice("Samsung Galaxy A54", 230, ["galaxy a54"], "smartphone"),
    DevicePrice("Samsung Galaxy A53", 180, ["galaxy a53"], "smartphone"),
    DevicePrice("Samsung Galaxy A35", 220, ["galaxy a35"], "smartphone"),
    DevicePrice("Samsung Galaxy A34", 180, ["galaxy a34"], "smartphone"),

    # Google Pixel
    DevicePrice("Google Pixel 9 Pro XL", 950, ["pixel 9 pro xl"], "smartphone"),
    DevicePrice("Google Pixel 9 Pro", 800, ["pixel 9 pro"], "smartphone"),
    DevicePrice("Google Pixel 9", 650, ["pixel 9"], "smartphone"),
    DevicePrice("Google Pixel 8 Pro", 650, ["pixel 8 pro"], "smartphone"),
    DevicePrice("Google Pixel 8", 490, ["pixel 8"], "smartphone"),
    DevicePrice("Google Pixel 7 Pro", 470, ["pixel 7 pro"], "smartphone"),
    DevicePrice("Google Pixel 7", 340, ["pixel 7"], "smartphone"),
    DevicePrice("Google Pixel 6 Pro", 320, ["pixel 6 pro"], "smartphone"),
    DevicePrice("Google Pixel 6", 230, ["pixel 6"], "smartphone"),

    # OnePlus
    DevicePrice("OnePlus 12", 550, ["oneplus 12"], "smartphone"),
    DevicePrice("OnePlus 11", 420, ["oneplus 11"], "smartphone"),
    DevicePrice("OnePlus 10 Pro", 320, ["oneplus 10 pro"], "smartphone"),

    # Xiaomi
    DevicePrice("Xiaomi 14 Pro", 650, ["xiaomi 14 pro"], "smartphone"),
    DevicePrice("Xiaomi 14", 520, ["xiaomi 14"], "smartphone"),
    DevicePrice("Xiaomi 13 Pro", 500, ["xiaomi 13 pro"], "smartphone"),
    DevicePrice("Xiaomi 13", 380, ["xiaomi 13"], "smartphone"),
    DevicePrice("Xiaomi Redmi Note 13 Pro", 240, ["redmi note 13 pro"], "smartphone"),
    DevicePrice("Xiaomi Redmi Note 12 Pro", 190, ["redmi note 12 pro"], "smartphone"),
]

# ---------------------------------------------------------------------------
# Laptops
# ---------------------------------------------------------------------------
LAPTOP_PRICES: list[DevicePrice] = [
    # Apple MacBook
    DevicePrice("MacBook Pro 16 M4 Pro/Max", 2800, ["macbook pro 16", "macbook pro 16\""], "laptop"),
    DevicePrice("MacBook Pro 14 M4 Pro/Max", 2300, ["macbook pro 14", "macbook pro 14\""], "laptop"),
    DevicePrice("MacBook Pro 13 M3", 1200, ["macbook pro 13 m3", "macbook pro m3 13"], "laptop"),
    DevicePrice("MacBook Pro M3", 1400, ["macbook pro m3"], "laptop"),
    DevicePrice("MacBook Pro M2 Pro", 1600, ["macbook pro m2 pro"], "laptop"),
    DevicePrice("MacBook Pro M2 Max", 2100, ["macbook pro m2 max"], "laptop"),
    DevicePrice("MacBook Pro M2", 1100, ["macbook pro m2"], "laptop"),
    DevicePrice("MacBook Pro M1 Pro", 1300, ["macbook pro m1 pro"], "laptop"),
    DevicePrice("MacBook Pro M1 Max", 1700, ["macbook pro m1 max"], "laptop"),
    DevicePrice("MacBook Pro M1", 900, ["macbook pro m1"], "laptop"),
    DevicePrice("MacBook Pro 2020", 750, ["macbook pro 2020"], "laptop"),
    DevicePrice("MacBook Pro 2019", 600, ["macbook pro 2019"], "laptop"),
    DevicePrice("MacBook Air M3", 1050, ["macbook air m3"], "laptop"),
    DevicePrice("MacBook Air M2", 850, ["macbook air m2"], "laptop"),
    DevicePrice("MacBook Air M1", 650, ["macbook air m1"], "laptop"),
    DevicePrice("MacBook Air 2020", 550, ["macbook air 2020"], "laptop"),
    DevicePrice("MacBook Air 2019", 420, ["macbook air 2019"], "laptop"),

    # Dell
    DevicePrice("Dell XPS 15", 1200, ["dell xps 15", "xps 15"], "laptop"),
    DevicePrice("Dell XPS 13", 850, ["dell xps 13", "xps 13"], "laptop"),
    DevicePrice("Dell XPS 17", 1500, ["dell xps 17", "xps 17"], "laptop"),
    DevicePrice("Dell Latitude 7490", 350, ["latitude 7490"], "laptop"),
    DevicePrice("Dell Latitude 7480", 280, ["latitude 7480"], "laptop"),
    DevicePrice("Dell Latitude 7440", 650, ["latitude 7440"], "laptop"),
    DevicePrice("Dell Inspiron 15", 450, ["dell inspiron 15", "inspiron 15"], "laptop"),
    DevicePrice("Dell Alienware m15", 1400, ["alienware m15"], "laptop"),
    DevicePrice("Dell Alienware m16", 1600, ["alienware m16"], "laptop"),

    # Lenovo ThinkPad
    DevicePrice("ThinkPad X1 Carbon Gen 11", 1100, ["thinkpad x1 carbon gen 11", "x1 carbon 11"], "laptop"),
    DevicePrice("ThinkPad X1 Carbon Gen 10", 850, ["thinkpad x1 carbon gen 10", "x1 carbon 10"], "laptop"),
    DevicePrice("ThinkPad X1 Carbon Gen 9", 650, ["thinkpad x1 carbon gen 9", "x1 carbon gen 9"], "laptop"),
    DevicePrice("ThinkPad X1 Carbon Gen 8", 500, ["thinkpad x1 carbon gen 8", "x1 carbon gen 8"], "laptop"),
    DevicePrice("ThinkPad X1 Carbon", 400, ["thinkpad x1 carbon"], "laptop"),
    DevicePrice("ThinkPad T14 Gen 4", 700, ["thinkpad t14 gen 4", "t14 gen 4"], "laptop"),
    DevicePrice("ThinkPad T14 Gen 3", 550, ["thinkpad t14 gen 3", "t14 gen 3"], "laptop"),
    DevicePrice("ThinkPad T14s", 450, ["thinkpad t14s", "t14s"], "laptop"),
    DevicePrice("ThinkPad T490", 300, ["thinkpad t490", "t490"], "laptop"),
    DevicePrice("ThinkPad T480", 260, ["thinkpad t480", "t480"], "laptop"),
    DevicePrice("ThinkPad T470", 200, ["thinkpad t470", "t470"], "laptop"),
    DevicePrice("ThinkPad E14 Gen 5", 550, ["thinkpad e14 gen 5", "e14 gen 5"], "laptop"),
    DevicePrice("ThinkPad E15 Gen 4", 500, ["thinkpad e15 gen 4", "e15 gen 4"], "laptop"),
    DevicePrice("Lenovo IdeaPad 5", 500, ["ideapad 5"], "laptop"),
    DevicePrice("Lenovo Yoga 9i", 950, ["yoga 9i"], "laptop"),
    DevicePrice("Lenovo Legion 5", 800, ["legion 5"], "laptop"),
    DevicePrice("Lenovo Legion 7", 1100, ["legion 7"], "laptop"),

    # HP
    DevicePrice("HP EliteBook 840 G10", 800, ["elitebook 840 g10"], "laptop"),
    DevicePrice("HP EliteBook 840 G9", 650, ["elitebook 840 g9"], "laptop"),
    DevicePrice("HP EliteBook 840 G8", 500, ["elitebook 840 g8"], "laptop"),
    DevicePrice("HP EliteBook 840 G7", 380, ["elitebook 840 g7"], "laptop"),
    DevicePrice("HP EliteBook 840 G6", 290, ["elitebook 840 g6"], "laptop"),
    DevicePrice("HP Spectre x360 14", 1100, ["spectre x360 14", "hp spectre 14"], "laptop"),
    DevicePrice("HP Spectre x360 13", 900, ["spectre x360 13", "hp spectre 13"], "laptop"),
    DevicePrice("HP Envy 15", 700, ["hp envy 15", "envy 15"], "laptop"),
    DevicePrice("HP ProBook 450 G9", 550, ["probook 450 g9"], "laptop"),
    DevicePrice("HP ProBook 450 G8", 420, ["probook 450 g8"], "laptop"),
    DevicePrice("HP Omen 16", 1000, ["hp omen 16", "omen 16"], "laptop"),

    # ASUS
    DevicePrice("ASUS ROG Zephyrus G14", 1100, ["zephyrus g14", "rog g14"], "laptop"),
    DevicePrice("ASUS ROG Zephyrus G15", 1200, ["zephyrus g15", "rog g15"], "laptop"),
    DevicePrice("ASUS ROG Strix G15", 1000, ["rog strix g15", "strix g15"], "laptop"),
    DevicePrice("ASUS ROG Strix G16", 1100, ["rog strix g16", "strix g16"], "laptop"),
    DevicePrice("ASUS VivoBook 15", 450, ["vivobook 15"], "laptop"),
    DevicePrice("ASUS ZenBook 14", 650, ["zenbook 14"], "laptop"),
    DevicePrice("ASUS TUF Gaming A15", 750, ["tuf gaming a15", "tuf a15"], "laptop"),
    DevicePrice("ASUS TUF Gaming F15", 700, ["tuf gaming f15", "tuf f15"], "laptop"),

    # Acer
    DevicePrice("Acer Swift 3", 500, ["acer swift 3", "swift 3"], "laptop"),
    DevicePrice("Acer Aspire 5", 420, ["acer aspire 5", "aspire 5"], "laptop"),
    DevicePrice("Acer Predator Helios 300", 950, ["predator helios 300", "helios 300"], "laptop"),
    DevicePrice("Acer Nitro 5", 700, ["acer nitro 5", "nitro 5"], "laptop"),

    # Microsoft
    DevicePrice("Surface Laptop 5", 950, ["surface laptop 5"], "laptop"),
    DevicePrice("Surface Laptop 4", 700, ["surface laptop 4"], "laptop"),
    DevicePrice("Surface Laptop 3", 500, ["surface laptop 3"], "laptop"),
    DevicePrice("Surface Pro 9", 1000, ["surface pro 9"], "laptop"),
    DevicePrice("Surface Pro 8", 750, ["surface pro 8"], "laptop"),
    DevicePrice("Surface Book 3", 900, ["surface book 3"], "laptop"),
]

# Combined catalog
ALL_DEVICES: list[DevicePrice] = SMARTPHONE_PRICES + LAPTOP_PRICES


def lookup_market_price(title: str, category_hint: str = "") -> tuple[Optional[DevicePrice], float]:
    """
    Find the best matching device for a listing title.
    Returns (device, market_price) or (None, 0.0) if no match.
    The returned market_price is already adjusted for fair used-market value.
    """
    title_lower = title.lower()

    candidates = ALL_DEVICES
    if category_hint == "smartphone":
        candidates = SMARTPHONE_PRICES
    elif category_hint in ("laptop", "notebook"):
        candidates = LAPTOP_PRICES

    best_match: Optional[DevicePrice] = None
    best_score = 0

    for device in candidates:
        for keyword in device.keywords:
            if keyword in title_lower:
                score = len(keyword)  # longer match = more specific = better
                if score > best_score:
                    best_score = score
                    best_match = device
                break

    if best_match is None:
        return None, 0.0

    return best_match, best_match.base_price


# Make Optional importable from here for type hints used across modules
from typing import Optional  # noqa: E402 (re-export)
