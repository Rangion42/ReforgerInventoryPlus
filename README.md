# Reforger Inventory+

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Version](https://img.shields.io/badge/version-0.1.0--alpha-orange.svg)
![Arma Reforger](https://img.shields.io/badge/Arma%20Reforger-1.6+-green.svg)

**Smart inventory management for ARMA Reforger** - A comprehensive quality-of-life mod that modernizes the inventory system with search, quick transfer, visual indicators, and intelligent organization.

## 🎯 Features

### ✅ Current (v0.1.0-alpha)

- 🔍 **Search Bar** - Quickly find items across all containers with aggregate counts
- 📊 **Visual Indicators** - Color-coded weight/capacity bars with real-time updates
- ⚡ **Category Quick Transfer** - Move all ammo, medical supplies, or equipment with one click
- 👁️ **Enhanced Vicinity View** - Larger, scrollable preview of nearby items

### 🚧 Planned Features

#### Phase 2: Quality of Life

- 🔄 **Magazine Consolidation** - Combine partially-filled magazines
- ⌨️ **Graduated Stack Moving** - Shift+drag (5 items) / Ctrl+drag (maximum)
- 📦 **Simultaneous Container View** - See all storage at once
- 🗑️ **Quick Drop** - Rapid junk item disposal

#### Phase 3: Advanced

- 🎯 **Auto-Sorting** - Organize by category with one button
- 💾 **Loadout Save/Load** - Store and recall equipment configurations
- ⚙️ **Custom Keybindings** - Personalize your controls
- 🎖️ **Arsenal Integration** - Enhanced arsenal experience

#### Phase 4: Polish

- 🌐 **Settings Menu** - Deep configurability
- 🔗 **Mod Compatibility** - Works with popular mods
- ⚡ **Performance Optimization** - Tuned for large servers
- 🌍 **Localization** - Multiple language support

## 📥 Installation

### For Players

1. **Subscribe on Workshop** (coming soon)
- Visit the [Arma Reforger Workshop](https://reforger.armaplatform.com/workshop)
- Search for “Reforger Inventory+”
- Click Subscribe
1. **Manual Installation**
- Download the latest release from [Releases](https://github.com/yourusername/ReforgerInventoryPlus/releases)
- Extract to your Reforger mods folder
- Enable in the game’s mod menu

### For Server Administrators

Add to your server’s mod configuration:

```json
{
  "modId": "WORKSHOP_ID_HERE",
  "name": "ReforgerInventoryPlus",
  "version": "0.1.0"
}
```

## 🎮 Usage

### Search Functionality

- Press `Tab` to open inventory
- Use the search bar at the top to filter items
- View aggregate counts for ammunition and medical supplies

### Quick Transfer

- Click category buttons to move all items of that type
- Works between inventory and vicinity, or between storage containers
- Categories: Weapons, Ammo, Medical, Equipment, All

### Visual Indicators

- **Green bar**: Under 70% capacity
- **Yellow bar**: 70-90% capacity
- **Red bar**: Over 90% capacity

## 🛠️ Technical Details

### Architecture

Built on Reforger’s Enfusion engine using:

- **InventoryStorageManagerComponent** - Core inventory operations
- **SCR_InventoryStorageManagerComponent** - Extended functionality
- **InventorySearchPredicate** - Efficient item querying
- **RplComponent** - Server-authoritative networking

### Key Design Decisions

1. **Server-Authoritative** - All inventory modifications validated server-side to prevent exploits
1. **Client-Side Search** - Queries run locally for instant results without server load
1. **Async Operations** - Uses `InventoryOperationCallback` for non-blocking actions
1. **Role-Based Logic** - Proper `RplRole.Authority`/`RplRole.Proxy` patterns (no legacy `isServer()`)

### Performance Considerations

- Search queries use `FindItem()` with predicates (client-safe, no network overhead)
- Weight calculations use `GetTotalWeightOfAllStorages()` with caching
- Category transfers batch operations to minimize RPC calls
- Configurable `streamingBudget` for large-scale servers

## 🤝 Contributing

We welcome contributions! Here’s how to get started:

### Development Setup

1. **Clone the repository**
   
   ```bash
   git clone https://github.com/yourusername/ReforgerInventoryPlus.git
   cd ReforgerInventoryPlus
   ```
1. **Install Arma Reforger Workbench**
- Download from [Bohemia Interactive](https://reforger.armaplatform.com/)
- Follow the [official modding documentation](https://community.bistudio.com/wiki/Arma_Reforger:Modding)
1. **Open in Workbench**
- Launch Arma Reforger Workbench
- Open the project file
- Configure your addon settings

### Coding Standards

- **Naming Convention**: Use your own prefix (not `SCR_` which is Bohemia’s)
- **Always set RplComponent to “Runtime”** for dynamic items
- **Use Try-prefixed methods** (`TryInsertItem`, `TryMoveItemToStorage`, etc.)
- **Network references via RplId**, never raw `EntityID`
- **Comment complex logic** especially around replication

### Testing Requirements

- ✅ Test in single-player first
- ✅ Test on dedicated server (RplComponent issues only appear here!)
- ✅ Test with full inventories (edge cases)
- ✅ Test with high latency (multiplayer sync)
- ✅ Check for memory leaks during extended sessions

### Pull Request Process

1. Fork the repository
1. Create a feature branch (`git checkout -b feature/AmazingFeature`)
1. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
1. Push to the branch (`git push origin feature/AmazingFeature`)
1. Open a Pull Request with a clear description

## 📋 Known Issues

- **Controller Input Conflict**: Multiple controllers (HOTAS, racing wheels) can break drag-and-drop. Workaround: Unplug secondary controllers.
- **Layout Compatibility**: Game updates may require UI layout adjustments.

See the [Issues](https://github.com/yourusername/ReforgerInventoryPlus/issues) page for a complete list and to report bugs.

## 🔍 Compatibility

### Tested With

- ✅ Vanilla Arma Reforger
- ✅ ACE Anvil
- ✅ RHS: Status Quo
- ✅ BetterInventory (some features may overlap)

### Known Conflicts

- None currently identified

## 📚 Resources

### Learning Reforger Modding

- [Arma Reforger Wiki](https://community.bistudio.com/wiki/Arma_Reforger)
- [Enfusion Script API](https://community.bistudio.com/wikidata/external-data/arma-reforger/EnfusionScriptAPIPublic/)
- [InventoryStorageManagerComponent Reference](https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceInventoryStorageManagerComponent.html)
- [Replication Overview](https://community.bistudio.com/wikidata/external-data/arma-reforger/EnfusionScriptAPIPublic/Page_Replication_Overview.html)

### Inspiration & Prior Art

- [ACE3 for ARMA 3](https://ace3.acemod.org/) - Magazine repacking inspiration
- [SearchInventory (DayZ)](https://steamcommunity.com/workshop/filedetails/?id=2936585965) - Search functionality patterns
- [Better Inventory (ARMA 3)](https://steamcommunity.com/workshop/filedetails/?id=2791403093) - Stack moving conventions

## 📄 License

This project is licensed under the MIT License - see the <LICENSE> file for details.

## 👏 Acknowledgments

- **Bohemia Interactive** - For Arma Reforger and the Enfusion engine
- **ACE3 Team** - Inspiration from their magazine repack system
- **Reforger Modding Community** - For sharing knowledge and best practices
- **Beta Testers** - Thank you for helping identify issues early

## 💬 Support & Community

- **Discord**: [Join our server](https://discord.gg/yourserver) (coming soon)
- **Issues**: [GitHub Issues](https://github.com/yourusername/ReforgerInventoryPlus/issues)
- **Workshop**: [Arma Reforger Workshop](https://reforger.armaplatform.com/workshop) (coming soon)

-----

**Made with ❤️ for the Arma Reforger community**

*If you find this mod useful, consider giving it a ⭐ on GitHub!*
