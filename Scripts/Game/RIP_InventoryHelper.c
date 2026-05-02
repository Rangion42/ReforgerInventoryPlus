// Reforger Inventory Plus - Inventory Helper Utilities
// Provides common inventory query and manipulation utilities

class RIP_InventoryHelper
{
	// Classify an entity into an item category based on its components and prefab type
	static RIP_EItemCategory ClassifyItem(IEntity item)
	{
		if (!item)
			return RIP_EItemCategory.NONE;

		// Check for weapon
		WeaponComponent weaponComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
		if (weaponComp)
			return RIP_EItemCategory.WEAPONS;

		// Check for magazine/ammo
		BaseMagazineComponent magComp = BaseMagazineComponent.Cast(item.FindComponent(BaseMagazineComponent));
		if (magComp)
			return RIP_EItemCategory.AMMO;

		// Check for medical items via SCR_ConsumableItemComponent
		SCR_ConsumableItemComponent consumableComp = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if (consumableComp)
		{
			// Check if it's a medical consumable
			SCR_ConsumableEffectHealthBase healthEffect = SCR_ConsumableEffectHealthBase.Cast(consumableComp.GetConsumableEffect());
			if (healthEffect)
				return RIP_EItemCategory.MEDICAL;
		}

		// Default to equipment for any other inventory item
		InventoryItemComponent invItemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (invItemComp)
			return RIP_EItemCategory.EQUIPMENT;

		return RIP_EItemCategory.NONE;
	}

	// Get the display name of an item
	static string GetItemDisplayName(IEntity item)
	{
		if (!item)
			return string.Empty;

		InventoryItemComponent invItem = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (invItem)
		{
			UIInfo uiInfo = invItem.GetUIInfo();
			if (uiInfo)
				return uiInfo.GetName();
		}

		return string.Empty;
	}

	// Get the description of an item
	static string GetItemDescription(IEntity item)
	{
		if (!item)
			return string.Empty;

		InventoryItemComponent invItem = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (invItem)
		{
			UIInfo uiInfo = invItem.GetUIInfo();
			if (uiInfo)
				return uiInfo.GetDescription();
		}

		return string.Empty;
	}

	// Get the weight of an item in kg
	static float GetItemWeight(IEntity item)
	{
		if (!item)
			return 0.0;

		InventoryItemComponent invItem = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (invItem)
		{
			return invItem.GetTotalWeight();
		}

		return 0.0;
	}

	// Get the inventory storage manager from a character entity
	static SCR_InventoryStorageManagerComponent GetInventoryManager(IEntity character)
	{
		if (!character)
			return null;

		return SCR_InventoryStorageManagerComponent.Cast(
			character.FindComponent(SCR_InventoryStorageManagerComponent)
		);
	}

	// Get all items in a storage, optionally filtered by category
	static void GetItemsFromStorage(BaseInventoryStorageComponent storage, RIP_EItemCategory filterCategory, out notnull array<IEntity> outItems)
	{
		if (!storage)
			return;

		int slotCount = storage.GetSlotsCount();
		for (int i = 0; i < slotCount; i++)
		{
			IEntity item = storage.Get(i);
			if (!item)
				continue;

			if (filterCategory == RIP_EItemCategory.ALL || ClassifyItem(item) == filterCategory)
			{
				outItems.Insert(item);
			}
		}
	}

	// Get all items from all storages on a character
	static void GetAllCharacterItems(IEntity character, RIP_EItemCategory filterCategory, out notnull array<IEntity> outItems)
	{
		SCR_InventoryStorageManagerComponent invManager = GetInventoryManager(character);
		if (!invManager)
			return;

		array<BaseInventoryStorageComponent> storages = {};
		invManager.GetStorages(storages);

		foreach (BaseInventoryStorageComponent storage : storages)
		{
			GetItemsFromStorage(storage, filterCategory, outItems);
		}
	}

	// Count items matching a category across all character storages
	static int CountItemsByCategory(IEntity character, RIP_EItemCategory category)
	{
		array<IEntity> items = {};
		GetAllCharacterItems(character, category, items);
		return items.Count();
	}

	// Get total weight of all items on a character
	static float GetTotalCharacterWeight(IEntity character)
	{
		SCR_InventoryStorageManagerComponent invManager = GetInventoryManager(character);
		if (!invManager)
			return 0.0;

		return invManager.GetTotalWeightOfAllStorages();
	}

	// Get maximum weight capacity of a character
	static float GetMaxCharacterWeight(IEntity character)
	{
		SCR_InventoryStorageManagerComponent invManager = GetInventoryManager(character);
		if (!invManager)
			return 0.0;

		return invManager.GetMaxWeightOfAllStorages();
	}

	// Get weight ratio (0.0 - 1.0) for capacity indicator
	static float GetWeightRatio(IEntity character)
	{
		float maxWeight = GetMaxCharacterWeight(character);
		if (maxWeight <= 0)
			return 0.0;

		return Math.Clamp(GetTotalCharacterWeight(character) / maxWeight, 0.0, 1.0);
	}

	// Get capacity ratio for a specific storage
	static float GetStorageCapacityRatio(BaseInventoryStorageComponent storage)
	{
		if (!storage)
			return 0.0;

		int totalSlots = storage.GetSlotsCount();
		if (totalSlots <= 0)
			return 0.0;

		int usedSlots = 0;
		for (int i = 0; i < totalSlots; i++)
		{
			if (storage.Get(i))
				usedSlots++;
		}

		return (float)usedSlots / (float)totalSlots;
	}

	// Check if an item name or description matches a search query (case-insensitive)
	static bool MatchesSearchQuery(IEntity item, string query)
	{
		if (!item || query.IsEmpty())
			return true;

		string lowerQuery = query;
		lowerQuery.ToLower();

		string itemName = GetItemDisplayName(item);
		itemName.ToLower();

		if (itemName.Contains(lowerQuery))
			return true;

		string itemDesc = GetItemDescription(item);
		itemDesc.ToLower();

		if (itemDesc.Contains(lowerQuery))
			return true;

		return false;
	}

	// Get the ammo count for a magazine entity
	static int GetMagazineAmmoCount(IEntity item)
	{
		if (!item)
			return 0;

		BaseMagazineComponent magComp = BaseMagazineComponent.Cast(item.FindComponent(BaseMagazineComponent));
		if (!magComp)
			return 0;

		return magComp.GetAmmoCount();
	}

	// Get the max ammo count for a magazine entity
	static int GetMagazineMaxAmmoCount(IEntity item)
	{
		if (!item)
			return 0;

		BaseMagazineComponent magComp = BaseMagazineComponent.Cast(item.FindComponent(BaseMagazineComponent));
		if (!magComp)
			return 0;

		return magComp.GetMaxAmmoCount();
	}

	// Get aggregate ammo count summary string (e.g., "5.56mm: 120/150")
	static string GetAmmoSummary(IEntity character)
	{
		array<IEntity> ammoItems = {};
		GetAllCharacterItems(character, RIP_EItemCategory.AMMO, ammoItems);

		// Group magazines by type and sum ammo counts
		map<string, int> currentAmmo = new map<string, int>();
		map<string, int> maxAmmo = new map<string, int>();

		foreach (IEntity item : ammoItems)
		{
			string name = GetItemDisplayName(item);
			int current = GetMagazineAmmoCount(item);
			int max = GetMagazineMaxAmmoCount(item);

			if (currentAmmo.Contains(name))
			{
				currentAmmo.Set(name, currentAmmo.Get(name) + current);
				maxAmmo.Set(name, maxAmmo.Get(name) + max);
			}
			else
			{
				currentAmmo.Insert(name, current);
				maxAmmo.Insert(name, max);
			}
		}

		// Build summary string
		string summary = "";
		for (int i = 0; i < currentAmmo.Count(); i++)
		{
			string magName = currentAmmo.GetKey(i);
			if (summary.Length() > 0)
				summary += " | ";
			summary += string.Format("%1: %2/%3", magName, currentAmmo.Get(magName), maxAmmo.Get(magName));
		}

		return summary;
	}
}
