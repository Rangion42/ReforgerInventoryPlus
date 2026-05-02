# Compatibility Notes

Last reviewed: 2026-05-02

## Game Versions

| Track | Version | Status | Notes |
| --- | --- | --- | --- |
| Stable/Main | 1.6.0.119 | Target | Official January 29, 2026 update does not list direct breaking changes to the inventory APIs used by this mod. |
| Experimental | 1.7.0.30 | Watch | Official April 2026 experimental builds include Enfusion changes, persistence changes, and inventory-adjacent fixes. Treat as unverified until 1.7 reaches Main or a dedicated experimental smoke test is complete. |

## Popular Mod Watchlist

These are not declared hard dependencies. They are high-value compatibility targets because they are commonly present in public server stacks or modify loadouts, gear, storage, or inventory-adjacent behavior.

| Mod / ecosystem | Current signal | Risk area for Inventory+ |
| --- | --- | --- |
| Bacon Loadout Editor | Workshop changelog shows 1.6.3 for game version 1.6.0.119, published April 13, 2026. | Arsenal/loadout flows and clothing storage edge cases. |
| RHS: Status Quo | RHS changelog includes 2026 updates with gear/loadout fixes, including equipment in pouches and attached clothing items being saved with loadouts. | Modded items, nested storage, and display-name coverage. |
| WCS-style packs | WCS Armaments has frequent 1.6.0.119 updates, including May 1, 2026. | Large item catalogs, custom magazines, search result volume, and category classification. |
| Battle Belts & Bags | Workshop changelog notes an April 19, 2026 inventory UI display update for belts. | Storage slot ratio, UI overlap, and wearable storage naming. |
| ACE Anvil | Popular realism ecosystem; monitor for medical/inventory behavior changes. | Medical classification and loadout/storage side effects. |

## Release Claim

Use this wording for the first public release:

> Built for Arma Reforger stable `1.6.0.119`. Experimental `1.7` compatibility is being monitored and needs runtime validation before it is claimed as supported.

Avoid claiming full compatibility with specific mod packs until a tester has provided game version, load order, repro notes, and logs or screenshots.

## Source Links

- Arma Reforger 1.6.0.119 Update: https://reforger.armaplatform.com/news/update-january-29-2026
- Arma Reforger 1.7.0.13 Experimental Update: https://reforger.armaplatform.com/news/experimental-april-2-2026
- Arma Reforger 1.7.0.30 Experimental Update: https://reforger.armaplatform.com/news/experimental-april-28-2026
- Bacon Loadout Editor changelog: https://reforger.armaplatform.com/workshop/606B100247F5C709/changelog
- RHS: Status Quo changelog: https://docs.rhsmods.org/rhs-status-quo-user-documentation/arma-reforger/rhs-status-quo/changelog
- Battle Belts & Bags changelog: https://reforger.armaplatform.com/workshop/6227608C3B33B383/changelog
