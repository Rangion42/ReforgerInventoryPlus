# Contributing to Reforger Inventory+

Thanks for helping improve Reforger Inventory+. This mod is intentionally small and focused: inventory search, transfer helpers, capacity visibility, and nearby-item quality of life.

## Before Opening a PR

1. Run the static checks:

   ```bash
   python3 tools/ci/check_static.py
   ```

2. Test in Workbench or in game when the change touches UI, inventory movement, replication, or storage scanning.
3. Keep new script classes prefixed with `RIP_`.
4. Do not use Bohemia-owned prefixes such as `SCR_` for new project code.

## Pull Request Notes

Please include:

- Game version tested, for example `1.6.0.119`.
- Single-player, hosted, or dedicated-server test coverage.
- Any mods used during compatibility testing.
- Screenshots or logs for UI, replication, or transfer bugs.

## Compatibility Reports

Compatibility reports are very welcome, especially with large server packs. Include the full mod list and load order if possible.
