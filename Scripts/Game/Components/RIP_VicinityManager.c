// Reforger Inventory Plus - Enhanced Vicinity Manager
// Provides an improved vicinity view with larger, scrollable item display
// Client-side scanning with configurable range and refresh rate

class RIP_VicinityItem
{
	IEntity m_Entity;
	string m_sDisplayName;
	RIP_EItemCategory m_eCategory;
	float m_fDistance;
	BaseInventoryStorageComponent m_Storage;  // If the item is inside a storage

	void RIP_VicinityItem(IEntity entity, float distance)
	{
		m_Entity = entity;
		m_fDistance = distance;
		m_sDisplayName = RIP_InventoryHelper.GetItemDisplayName(entity);
		m_eCategory = RIP_InventoryHelper.ClassifyItem(entity);
		m_Storage = null;
	}
}

class RIP_VicinityManager
{
	protected IEntity m_OwnerEntity;
	protected ref array<ref RIP_VicinityItem> m_aVicinityItems;
	protected ref array<BaseInventoryStorageComponent> m_aVicinityStorages;
	protected float m_fScanTimer;
	protected float m_fScanRange;
	protected int m_iTotalItemCount;
	protected bool m_bEnabled;

	// Event callback when vicinity items change
	ref ScriptInvoker m_OnVicinityUpdated;

	void RIP_VicinityManager(IEntity owner)
	{
		m_OwnerEntity = owner;
		m_aVicinityItems = {};
		m_aVicinityStorages = {};
		m_fScanTimer = 0;
		m_fScanRange = RIP_Constants.VICINITY_RANGE_DEFAULT;
		m_iTotalItemCount = 0;
		m_bEnabled = true;
		m_OnVicinityUpdated = new ScriptInvoker();
	}

	void ~RIP_VicinityManager()
	{
		m_aVicinityItems = null;
		m_aVicinityStorages = null;
	}

	// Enable/disable vicinity scanning
	void SetEnabled(bool enabled)
	{
		m_bEnabled = enabled;
	}

	bool IsEnabled()
	{
		return m_bEnabled;
	}

	// Set the scan range (meters)
	void SetScanRange(float range)
	{
		m_fScanRange = Math.Clamp(range, 1.0, 20.0);
	}

	float GetScanRange()
	{
		return m_fScanRange;
	}

	// Update - called each frame, manages scan timing
	void Update(float timeSlice)
	{
		if (!m_bEnabled)
			return;

		m_fScanTimer += timeSlice;

		float refreshRate = RIP_Config.GetInstance().m_fVicinityRefreshRate;
		if (m_fScanTimer >= refreshRate)
		{
			m_fScanTimer = 0;
			ScanVicinity();
		}
	}

	// Perform the vicinity scan
	protected void ScanVicinity()
	{
		m_aVicinityItems.Clear();
		m_aVicinityStorages.Clear();
		m_iTotalItemCount = 0;

		if (!m_OwnerEntity)
			return;

		vector ownerPos = m_OwnerEntity.GetOrigin();

		// Query nearby entities within scan range
		// Use sphere query to find entities with inventory items nearby
		array<IEntity> nearbyEntities = {};
		GetGame().GetWorld().QueryEntitiesBySphere(
			ownerPos,
			m_fScanRange,
			this,
			FilterVicinityEntity,
			EQueryEntitiesFlags.ALL
		);
	}

	// Callback for entity query - process each found entity
	protected bool FilterVicinityEntity(IEntity entity)
	{
		if (!entity || entity == m_OwnerEntity)
			return true;  // Continue searching

		// Check if this entity has an inventory item component (is a pickup-able item)
		InventoryItemComponent invItem = InventoryItemComponent.Cast(
			entity.FindComponent(InventoryItemComponent)
		);

		if (invItem)
		{
			float distance = vector.Distance(m_OwnerEntity.GetOrigin(), entity.GetOrigin());
			RIP_VicinityItem vicinityItem = new RIP_VicinityItem(entity, distance);
			m_aVicinityItems.Insert(vicinityItem);
			m_iTotalItemCount++;
		}

		// Check if this entity has storages (is a container like a box, vehicle, body)
		BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(
			entity.FindComponent(BaseInventoryStorageComponent)
		);

		if (storage)
		{
			m_aVicinityStorages.Insert(storage);

			// Also list the items inside this storage
			int slotCount = storage.GetSlotsCount();
			for (int i = 0; i < slotCount; i++)
			{
				IEntity containedItem = storage.Get(i);
				if (containedItem)
				{
					float containerDist = vector.Distance(m_OwnerEntity.GetOrigin(), entity.GetOrigin());
					RIP_VicinityItem containerItem = new RIP_VicinityItem(containedItem, containerDist);
					containerItem.m_Storage = storage;
					m_aVicinityItems.Insert(containerItem);
					m_iTotalItemCount++;
				}
			}
		}

		// Enforce max item limit
		if (m_iTotalItemCount >= RIP_Constants.VICINITY_MAX_ITEMS)
			return false;  // Stop searching

		return true;  // Continue
	}

	// Sort vicinity items by distance
	void SortByDistance()
	{
		int count = m_aVicinityItems.Count();
		for (int i = 1; i < count; i++)
		{
			RIP_VicinityItem key = m_aVicinityItems[i];
			int j = i - 1;

			while (j >= 0 && m_aVicinityItems[j].m_fDistance > key.m_fDistance)
			{
				m_aVicinityItems[j + 1] = m_aVicinityItems[j];
				j--;
			}
			m_aVicinityItems[j + 1] = key;
		}
	}

	// Sort vicinity items by category then name
	void SortByCategory()
	{
		int count = m_aVicinityItems.Count();
		for (int i = 1; i < count; i++)
		{
			RIP_VicinityItem key = m_aVicinityItems[i];
			int j = i - 1;

			while (j >= 0 && CompareByCategory(m_aVicinityItems[j], key) > 0)
			{
				m_aVicinityItems[j + 1] = m_aVicinityItems[j];
				j--;
			}
			m_aVicinityItems[j + 1] = key;
		}
	}

	protected int CompareByCategory(RIP_VicinityItem a, RIP_VicinityItem b)
	{
		if (a.m_eCategory != b.m_eCategory)
			return a.m_eCategory - b.m_eCategory;
		return a.m_sDisplayName.Compare(b.m_sDisplayName);
	}

	// Filter vicinity items by search query
	void GetFilteredItems(string query, RIP_EItemCategory category, out notnull array<ref RIP_VicinityItem> outItems)
	{
		string lowerQuery = query;
		lowerQuery.ToLower();

		foreach (RIP_VicinityItem item : m_aVicinityItems)
		{
			// Category filter
			if (category != RIP_EItemCategory.ALL && item.m_eCategory != category)
				continue;

			// Text search filter
			if (!lowerQuery.IsEmpty())
			{
				string lowerName = item.m_sDisplayName;
				lowerName.ToLower();
				if (!lowerName.Contains(lowerQuery))
					continue;
			}

			outItems.Insert(item);
		}
	}

	// Force an immediate scan
	void ForceScan()
	{
		m_fScanTimer = 0;
		ScanVicinity();
	}

	// Getters
	array<ref RIP_VicinityItem> GetVicinityItems()
	{
		return m_aVicinityItems;
	}

	array<BaseInventoryStorageComponent> GetVicinityStorages()
	{
		return m_aVicinityStorages;
	}

	int GetItemCount()
	{
		return m_iTotalItemCount;
	}

	// Get count of items by category in vicinity
	int GetCategoryCount(RIP_EItemCategory category)
	{
		if (category == RIP_EItemCategory.ALL)
			return m_iTotalItemCount;

		int count = 0;
		foreach (RIP_VicinityItem item : m_aVicinityItems)
		{
			if (item.m_eCategory == category)
				count++;
		}
		return count;
	}

	// Get summary string for vicinity
	string GetVicinitySummary()
	{
		return string.Format("%1 items nearby (%2m range)",
			m_iTotalItemCount,
			m_fScanRange);
	}
}
