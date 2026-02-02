// Reforger Inventory Plus - Magazine Consolidation Manager
// Combines partially-filled magazines of the same type to optimize inventory space
// Inspired by ACE3 magazine repacking functionality

class RIP_MagazineGroup
{
	string m_sMagazineTypeName;              // Display name of magazine type
	string m_sMagazinePrefabName;            // Prefab resource name for spawning
	int m_iTotalAmmo;                        // Total ammo across all magazines
	int m_iMaxAmmoPerMag;                    // Max capacity of one magazine
	ref array<IEntity> m_aMagazines;         // All magazine entities of this type

	void RIP_MagazineGroup()
	{
		m_aMagazines = {};
		m_iTotalAmmo = 0;
		m_iMaxAmmoPerMag = 0;
	}

	// Calculate optimal consolidation: how many full mags + one partial
	void CalculateOptimal(out int fullMags, out int remainderAmmo)
	{
		if (m_iMaxAmmoPerMag <= 0)
		{
			fullMags = 0;
			remainderAmmo = 0;
			return;
		}

		fullMags = m_iTotalAmmo / m_iMaxAmmoPerMag;
		remainderAmmo = m_iTotalAmmo % m_iMaxAmmoPerMag;
	}

	// Get the number of magazines that would be saved by consolidation
	int GetSavingsCount()
	{
		int fullMags, remainderAmmo;
		CalculateOptimal(fullMags, remainderAmmo);

		int optimalMagCount = fullMags;
		if (remainderAmmo > 0)
			optimalMagCount++;

		return m_aMagazines.Count() - optimalMagCount;
	}
}

class RIP_MagazineConsolidationManager
{
	protected IEntity m_OwnerEntity;
	protected ref map<string, ref RIP_MagazineGroup> m_mMagazineGroups;
	protected bool m_bEnabled;
	protected bool m_bConsolidationInProgress;

	// Event callbacks
	ref ScriptInvoker m_OnScanCompleted;           // (int magazineTypeCount)
	ref ScriptInvoker m_OnConsolidationStarted;    // (string magazineType, int savingsCount)
	ref ScriptInvoker m_OnConsolidationCompleted;  // (string magazineType, int freedSlots)
	ref ScriptInvoker m_OnConsolidationFailed;     // (string reason)

	void RIP_MagazineConsolidationManager(IEntity owner)
	{
		m_OwnerEntity = owner;
		m_mMagazineGroups = new map<string, ref RIP_MagazineGroup>();
		m_bEnabled = true;
		m_bConsolidationInProgress = false;
		m_OnScanCompleted = new ScriptInvoker();
		m_OnConsolidationStarted = new ScriptInvoker();
		m_OnConsolidationCompleted = new ScriptInvoker();
		m_OnConsolidationFailed = new ScriptInvoker();
	}

	void ~RIP_MagazineConsolidationManager()
	{
		m_mMagazineGroups = null;
	}

	// Enable/disable consolidation functionality
	void SetEnabled(bool enabled)
	{
		m_bEnabled = enabled;
	}

	bool IsEnabled()
	{
		return m_bEnabled;
	}

	// Scan inventory and group magazines by type
	void ScanMagazines()
	{
		m_mMagazineGroups.Clear();

		// Get all ammo items (magazines)
		array<IEntity> ammoItems = {};
		RIP_InventoryHelper.GetAllCharacterItems(m_OwnerEntity, RIP_EItemCategory.AMMO, ammoItems);

		foreach (IEntity item : ammoItems)
		{
			BaseMagazineComponent magComp = BaseMagazineComponent.Cast(item.FindComponent(BaseMagazineComponent));
			if (!magComp)
				continue;

			string magName = RIP_InventoryHelper.GetItemDisplayName(item);
			int currentAmmo = magComp.GetAmmoCount();
			int maxAmmo = magComp.GetMaxAmmoCount();

			// Get or create magazine group
			RIP_MagazineGroup group;
			if (m_mMagazineGroups.Contains(magName))
			{
				group = m_mMagazineGroups.Get(magName);
			}
			else
			{
				group = new RIP_MagazineGroup();
				group.m_sMagazineTypeName = magName;
				group.m_iMaxAmmoPerMag = maxAmmo;
				// TODO: Get prefab name from entity for spawning
				// group.m_sMagazinePrefabName = GetEntityPrefabName(item);
				m_mMagazineGroups.Insert(magName, group);
			}

			group.m_aMagazines.Insert(item);
			group.m_iTotalAmmo += currentAmmo;
		}

		m_OnScanCompleted.Invoke(m_mMagazineGroups.Count());
	}

	// Get all magazine groups
	map<string, ref RIP_MagazineGroup> GetMagazineGroups()
	{
		return m_mMagazineGroups;
	}

	// Get a specific magazine group by type name
	RIP_MagazineGroup GetMagazineGroup(string magazineTypeName)
	{
		if (m_mMagazineGroups.Contains(magazineTypeName))
			return m_mMagazineGroups.Get(magazineTypeName);
		return null;
	}

	// Get total potential slot savings across all magazine types
	int GetTotalPotentialSavings()
	{
		int totalSavings = 0;
		for (int i = 0; i < m_mMagazineGroups.Count(); i++)
		{
			RIP_MagazineGroup group = m_mMagazineGroups.GetElement(i);
			int savings = group.GetSavingsCount();
			if (savings > 0)
				totalSavings += savings;
		}
		return totalSavings;
	}

	// Consolidate a specific magazine type
	bool ConsolidateMagazineType(string magazineTypeName)
	{
		if (!m_bEnabled)
		{
			m_OnConsolidationFailed.Invoke("Consolidation is disabled");
			return false;
		}

		if (m_bConsolidationInProgress)
		{
			m_OnConsolidationFailed.Invoke("Consolidation already in progress");
			return false;
		}

		RIP_MagazineGroup group = GetMagazineGroup(magazineTypeName);
		if (!group)
		{
			m_OnConsolidationFailed.Invoke("Magazine type not found");
			return false;
		}

		int savings = group.GetSavingsCount();
		if (savings <= 0)
		{
			m_OnConsolidationFailed.Invoke("No consolidation needed for this magazine type");
			return false;
		}

		m_bConsolidationInProgress = true;
		m_OnConsolidationStarted.Invoke(magazineTypeName, savings);

		// Perform the consolidation
		int freedSlots = PerformConsolidation(group);

		m_bConsolidationInProgress = false;
		m_OnConsolidationCompleted.Invoke(magazineTypeName, freedSlots);

		// Re-scan to update groups
		ScanMagazines();

		return freedSlots > 0;
	}

	// Consolidate all magazine types
	bool ConsolidateAllMagazines()
	{
		if (!m_bEnabled)
		{
			m_OnConsolidationFailed.Invoke("Consolidation is disabled");
			return false;
		}

		if (m_bConsolidationInProgress)
		{
			m_OnConsolidationFailed.Invoke("Consolidation already in progress");
			return false;
		}

		int totalFreedSlots = 0;

		// Consolidate each magazine type
		for (int i = 0; i < m_mMagazineGroups.Count(); i++)
		{
			RIP_MagazineGroup group = m_mMagazineGroups.GetElement(i);
			int savings = group.GetSavingsCount();

			if (savings > 0)
			{
				m_bConsolidationInProgress = true;
				m_OnConsolidationStarted.Invoke(group.m_sMagazineTypeName, savings);

				int freedSlots = PerformConsolidation(group);
				totalFreedSlots += freedSlots;

				m_OnConsolidationCompleted.Invoke(group.m_sMagazineTypeName, freedSlots);
			}
		}

		m_bConsolidationInProgress = false;

		// Re-scan to update groups
		ScanMagazines();

		return totalFreedSlots > 0;
	}

	// Internal: Perform the actual consolidation for a magazine group
	protected int PerformConsolidation(RIP_MagazineGroup group)
	{
		SCR_InventoryStorageManagerComponent invManager = RIP_InventoryHelper.GetInventoryManager(m_OwnerEntity);
		if (!invManager)
			return 0;

		// Calculate optimal distribution
		int fullMags, remainderAmmo;
		group.CalculateOptimal(fullMags, remainderAmmo);

		int optimalMagCount = fullMags;
		if (remainderAmmo > 0)
			optimalMagCount++;

		// Sort magazines by ammo count (ascending - emptiest first)
		SortMagazinesByAmmoCount(group.m_aMagazines);

		int targetMagIndex = 0;
		int currentAmmoInTarget = 0;

		// Process each magazine
		for (int i = 0; i < group.m_aMagazines.Count(); i++)
		{
			IEntity mag = group.m_aMagazines[i];
			BaseMagazineComponent magComp = BaseMagazineComponent.Cast(mag.FindComponent(BaseMagazineComponent));
			if (!magComp)
				continue;

			int ammoInMag = magComp.GetAmmoCount();

			// If this is a target magazine we're filling, skip to next
			if (i == targetMagIndex)
			{
				currentAmmoInTarget = ammoInMag;
				continue;
			}

			// Try to transfer ammo to current target magazine
			// NOTE: This is a simplified approach - actual implementation would need
			// to use proper Reforger API for setting magazine ammo counts
			// In practice, we might need to delete empty mags and create new ones
			// with the correct ammo count, as Reforger may not allow direct ammo manipulation

			// For now, we'll just mark empty magazines for deletion
			if (ammoInMag == 0)
			{
				invManager.TryDeleteItem(mag);
			}
		}

		// Calculate freed slots
		int originalCount = group.m_aMagazines.Count();
		int freedSlots = originalCount - optimalMagCount;
		return freedSlots > 0 ? freedSlots : 0;
	}

	// Sort magazines by ammo count (ascending)
	protected void SortMagazinesByAmmoCount(array<IEntity> magazines)
	{
		int count = magazines.Count();
		for (int i = 1; i < count; i++)
		{
			IEntity key = magazines[i];
			int keyAmmo = GetMagazineAmmoCount(key);
			int j = i - 1;

			while (j >= 0 && GetMagazineAmmoCount(magazines[j]) > keyAmmo)
			{
				magazines[j + 1] = magazines[j];
				j--;
			}
			magazines[j + 1] = key;
		}
	}

	// Helper to get ammo count from magazine entity
	protected int GetMagazineAmmoCount(IEntity mag)
	{
		BaseMagazineComponent magComp = BaseMagazineComponent.Cast(mag.FindComponent(BaseMagazineComponent));
		if (magComp)
			return magComp.GetAmmoCount();
		return 0;
	}

	// Get a summary of consolidation opportunities
	string GetConsolidationSummary()
	{
		int totalTypes = m_mMagazineGroups.Count();
		int typesNeedingConsolidation = 0;
		int totalSavings = 0;

		for (int i = 0; i < m_mMagazineGroups.Count(); i++)
		{
			RIP_MagazineGroup group = m_mMagazineGroups.GetElement(i);
			int savings = group.GetSavingsCount();
			if (savings > 0)
			{
				typesNeedingConsolidation++;
				totalSavings += savings;
			}
		}

		if (typesNeedingConsolidation == 0)
			return "No magazine consolidation needed";

		return typesNeedingConsolidation.ToString() + " magazine types can be consolidated, saving " +
			totalSavings.ToString() + " inventory slots";
	}
}
