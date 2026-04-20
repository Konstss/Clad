"""Kleinanzeigen.de scraper — fetches listings for a given search term and region."""

import re
import time
import random
import logging
import requests
from bs4 import BeautifulSoup
from dataclasses import dataclass, field
from typing import Optional

logger = logging.getLogger(__name__)

BASE_URL = "https://www.kleinanzeigen.de"

HEADERS = {
    "User-Agent": (
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/124.0.0.0 Safari/537.36"
    ),
    "Accept-Language": "de-DE,de;q=0.9,en;q=0.8",
    "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
    "Referer": "https://www.kleinanzeigen.de/",
}

CONDITION_MAP = {
    "neu": 1.0,
    "neuwertig": 0.90,
    "sehr gut": 0.80,
    "gut": 0.65,
    "akzeptabel": 0.50,
    "defekt": 0.25,
    "bastlergerät": 0.20,
    "bastler": 0.20,
    "für bastler": 0.20,
    "wie neu": 0.90,
    "ovp": 0.95,
    "originalverpackung": 0.95,
    "gebraucht": 0.65,
}


@dataclass
class Listing:
    title: str
    price: float
    url: str
    location: str
    condition: str
    condition_factor: float
    image_url: str = ""
    description: str = ""
    ad_id: str = ""
    posted_date: str = ""
    category: str = ""
    raw_price_text: str = ""


def _extract_condition(text: str) -> tuple[str, float]:
    """Detect condition keywords in title/description, return label and factor."""
    lowered = text.lower()
    for keyword, factor in sorted(CONDITION_MAP.items(), key=lambda x: -len(x[0])):
        if keyword in lowered:
            return keyword, factor
    return "unbekannt", 0.70  # default assumption: used / good


def _parse_price(price_text: str) -> Optional[float]:
    """Parse German price string like '1.299 €' → 1299.0."""
    cleaned = re.sub(r"[^\d,.]", "", price_text.replace(".", "").replace(",", "."))
    try:
        return float(cleaned)
    except ValueError:
        return None


def _build_search_url(query: str, region_id: str, page: int = 1) -> str:
    """Build Kleinanzeigen search URL with optional region."""
    slug = query.lower().replace(" ", "-")
    if region_id:
        # Kleinanzeigen URL format: /s-{region}/{query}/k0
        base = f"{BASE_URL}/s-{region_id}/{slug}/k0"
    else:
        base = f"{BASE_URL}/s-{slug}/k0"
    if page > 1:
        base = base.replace("/k0", f"/seite:{page}/k0")
    return base


def _fetch_page(url: str, session: requests.Session) -> Optional[BeautifulSoup]:
    try:
        resp = session.get(url, headers=HEADERS, timeout=15)
        resp.raise_for_status()
        return BeautifulSoup(resp.text, "lxml")
    except requests.RequestException as e:
        logger.warning("Failed to fetch %s: %s", url, e)
        return None


def _parse_listings(soup: BeautifulSoup, category: str) -> list[Listing]:
    listings = []
    articles = soup.select("article.aditem")

    for article in articles:
        try:
            # Title
            title_tag = article.select_one("a.ellipsis")
            if not title_tag:
                continue
            title = title_tag.get_text(strip=True)

            # URL
            href = article.select_one("a[href*='/s-anzeige/']")
            if not href:
                continue
            url = BASE_URL + href["href"]

            # Price
            price_tag = article.select_one("p.aditem-main--middle--price-shipping--price")
            if not price_tag:
                continue
            raw_price = price_tag.get_text(strip=True)
            if "VB" in raw_price or "Zu verschenken" in raw_price or "Tausch" in raw_price.lower():
                continue
            price = _parse_price(raw_price)
            if price is None or price <= 0:
                continue

            # Location
            location_tag = article.select_one("div.aditem-main--top--left")
            location = location_tag.get_text(strip=True) if location_tag else ""

            # Description snippet
            desc_tag = article.select_one("p.aditem-main--middle--description")
            description = desc_tag.get_text(strip=True) if desc_tag else ""

            # Image
            img_tag = article.select_one("img.imagebox-image")
            image_url = img_tag.get("src", "") if img_tag else ""

            # Ad ID
            ad_id = article.get("data-adid", "")

            # Date
            date_tag = article.select_one("div.aditem-main--top--right")
            posted_date = date_tag.get_text(strip=True) if date_tag else ""

            # Condition detection from title + description
            full_text = f"{title} {description}"
            condition, condition_factor = _extract_condition(full_text)

            listings.append(Listing(
                title=title,
                price=price,
                url=url,
                location=location,
                condition=condition,
                condition_factor=condition_factor,
                image_url=image_url,
                description=description,
                ad_id=ad_id,
                posted_date=posted_date,
                category=category,
                raw_price_text=raw_price,
            ))
        except Exception as e:
            logger.debug("Error parsing article: %s", e)
            continue

    return listings


def scrape(
    query: str,
    region_id: str = "",
    max_pages: int = 3,
    delay: float = 1.5,
    category: str = "",
) -> list[Listing]:
    """
    Scrape Kleinanzeigen listings for the given query and region.
    Returns a list of Listing objects.
    """
    session = requests.Session()
    all_listings: list[Listing] = []

    for page in range(1, max_pages + 1):
        url = _build_search_url(query, region_id, page)
        logger.info("Fetching page %d: %s", page, url)

        soup = _fetch_page(url, session)
        if soup is None:
            break

        page_listings = _parse_listings(soup, category or query)
        if not page_listings:
            break

        all_listings.extend(page_listings)
        time.sleep(delay + random.uniform(0.3, 0.8))

    # Deduplicate by ad_id
    seen = set()
    unique = []
    for l in all_listings:
        key = l.ad_id or l.url
        if key not in seen:
            seen.add(key)
            unique.append(l)

    return unique
