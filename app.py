"""Flask web application for Kleinanzeigen deal finder."""

import logging
from flask import Flask, render_template, request, jsonify

from scraper import scrape
from analyzer import analyze, deals_to_dict
from regions import REGIONS
from market_prices import SMARTPHONE_PRICES, LAPTOP_PRICES, ALL_DEVICES

logging.basicConfig(level=logging.INFO, format="%(levelname)s %(name)s: %(message)s")
logger = logging.getLogger(__name__)

app = Flask(__name__)

DB_STATS = {
    "smartphones": len(SMARTPHONE_PRICES),
    "laptops": len(LAPTOP_PRICES),
    "total": len(ALL_DEVICES),
}

SEARCH_CATEGORIES = [
    {"label": "Smartphones", "queries": ["smartphone", "iphone", "samsung galaxy", "pixel"], "hint": "smartphone"},
    {"label": "Laptops / Notebooks", "queries": ["laptop", "notebook", "macbook", "thinkpad"], "hint": "laptop"},
    {"label": "Tablets", "queries": ["tablet", "ipad", "samsung tab"], "hint": "tablet"},
]

PRESET_SEARCHES = [
    {"label": "iPhone (alle Modelle)", "query": "iphone", "hint": "smartphone"},
    {"label": "Samsung Galaxy", "query": "samsung galaxy", "hint": "smartphone"},
    {"label": "Google Pixel", "query": "google pixel", "hint": "smartphone"},
    {"label": "MacBook", "query": "macbook", "hint": "laptop"},
    {"label": "ThinkPad", "query": "thinkpad", "hint": "laptop"},
    {"label": "Dell XPS", "query": "dell xps", "hint": "laptop"},
    {"label": "Gaming Laptop", "query": "gaming laptop", "hint": "laptop"},
    {"label": "Laptop allgemein", "query": "laptop", "hint": "laptop"},
]


@app.route("/")
def index():
    return render_template(
        "index.html",
        regions=REGIONS,
        presets=PRESET_SEARCHES,
        stats=DB_STATS,
    )


@app.route("/api/search", methods=["POST"])
def api_search():
    data = request.get_json(force=True)
    query: str = (data.get("query") or "").strip()
    region_slug: str = (data.get("region") or "").strip()
    hint: str = (data.get("hint") or "").strip()
    max_pages: int = min(int(data.get("max_pages", 3)), 5)
    threshold: float = float(data.get("threshold", 0.25))

    if not query:
        return jsonify({"error": "Kein Suchbegriff angegeben."}), 400

    logger.info("Search: query=%r region=%r hint=%r pages=%d", query, region_slug, hint, max_pages)

    try:
        listings = scrape(
            query=query,
            region_id=region_slug,
            max_pages=max_pages,
            category=hint,
        )
        deals = analyze(listings, threshold=threshold)
        result = deals_to_dict(deals)

        return jsonify({
            "listings_found": len(listings),
            "deals_found": len(deals),
            "deals": result,
        })
    except Exception as e:
        logger.exception("Search failed")
        return jsonify({"error": str(e)}), 500


@app.route("/api/stats")
def api_stats():
    """Return some stats about the market price database."""
    return jsonify({
        "smartphones_in_db": len(SMARTPHONE_PRICES),
        "laptops_in_db": len(LAPTOP_PRICES),
        "total_devices": len(SMARTPHONE_PRICES) + len(LAPTOP_PRICES),
    })


if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0", port=5000)
