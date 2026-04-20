"""
Deal analyzer: compares listing prices against market prices,
applying condition adjustments to find genuine bargains.

Algorithm:
  expected_price = market_base_price × condition_factor
  discount       = 1 - (listing_price / expected_price)
  deal           = discount >= DEAL_THRESHOLD (default 25%)

Condition factors (from scraper.py CONDITION_MAP):
  neu / OVP        → 1.0 / 0.95
  neuwertig / wie neu → 0.90
  sehr gut         → 0.80
  gut / gebraucht  → 0.65
  akzeptabel       → 0.50
  defekt / bastler → 0.20–0.25
  unbekannt        → 0.70 (conservative default)
"""

from __future__ import annotations
from dataclasses import dataclass, field
from typing import Optional

from scraper import Listing
from market_prices import lookup_market_price, DevicePrice

DEAL_THRESHOLD = 0.25   # 25% below expected market price


@dataclass
class Deal:
    listing: Listing
    device: DevicePrice
    market_base_price: float       # base fair price from DB (gut condition)
    expected_price: float          # market_base_price × condition_factor
    discount_pct: float            # e.g. 0.35 → 35% below market
    savings: float                 # € saved vs expected price

    @property
    def deal_score(self) -> float:
        """0–100 score: higher = better deal."""
        return min(round(self.discount_pct * 100, 1), 99.9)

    @property
    def deal_label(self) -> str:
        pct = self.discount_pct * 100
        if pct >= 50:
            return "Hammer-Deal 🔥"
        if pct >= 40:
            return "Sehr guter Deal"
        if pct >= 30:
            return "Guter Deal"
        return "Deal"

    @property
    def condition_label(self) -> str:
        return self.listing.condition.title()


def analyze(listings: list[Listing], threshold: float = DEAL_THRESHOLD) -> list[Deal]:
    """
    Analyze a list of scraped listings and return only those that qualify as deals.
    Results are sorted by discount percentage (best deal first).
    """
    deals: list[Deal] = []

    for listing in listings:
        device, market_base = lookup_market_price(
            listing.title,
            category_hint=listing.category,
        )
        if device is None or market_base <= 0:
            continue

        # Adjust for condition: a defective device is worth much less
        expected_price = market_base * listing.condition_factor

        if expected_price <= 0:
            continue

        discount = 1.0 - (listing.price / expected_price)

        if discount >= threshold:
            savings = expected_price - listing.price
            deals.append(Deal(
                listing=listing,
                device=device,
                market_base_price=market_base,
                expected_price=round(expected_price, 2),
                discount_pct=round(discount, 4),
                savings=round(savings, 2),
            ))

    deals.sort(key=lambda d: d.discount_pct, reverse=True)
    return deals


def deals_to_dict(deals: list[Deal]) -> list[dict]:
    """Serialize deals to JSON-compatible dicts for the API / template."""
    result = []
    for d in deals:
        result.append({
            "title": d.listing.title,
            "price": d.listing.price,
            "url": d.listing.url,
            "location": d.listing.location,
            "condition": d.condition_label,
            "condition_factor": d.listing.condition_factor,
            "image_url": d.listing.image_url,
            "description": d.listing.description,
            "posted_date": d.listing.posted_date,
            "device_name": d.device.name,
            "market_base_price": d.market_base_price,
            "expected_price": d.expected_price,
            "discount_pct": round(d.discount_pct * 100, 1),
            "savings": d.savings,
            "deal_score": d.deal_score,
            "deal_label": d.deal_label,
        })
    return result
