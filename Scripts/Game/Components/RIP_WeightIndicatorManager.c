// Reforger Inventory Plus - Weight/Capacity Indicator Manager
// Manages real-time weight and capacity tracking with color-coded visual feedback
// Client-side only - uses GetTotalWeightOfAllStorages() with caching

class RIP_StorageInfo
{
	BaseInventoryStorageComponent m_Storage;
	string m_sStorageName;
	float m_fCurrentWeight;
	float m_fMaxWeight;
	int m_iUsedSlots;
	int m_iTotalSlots;
	float m_fWeightRatio;
	float m_fSlotRatio;
	RIP_ECapacityLevel m_eCapacityLevel;
	int m_iBarColor;

	void RIP_StorageInfo(BaseInventoryStorageComponent storage)
	{
		m_Storage = storage;
		m_sStorageName = "";
		m_fCurrentWeight = 0;
		m_fMaxWeight = 0;
		m_iUsedSlots = 0;
		m_iTotalSlots = 0;
		m_fWeightRatio = 0;
		m_fSlotRatio = 0;
		m_eCapacityLevel = RIP_ECapacityLevel.LOW;
		m_iBarColor = RIP_Constants.COLOR_CAPACITY_LOW;
	}

	void Update()
	{
		if (!m_Storage)
			return;

		// Count used slots
		m_iTotalSlots = m_Storage.GetSlotsCount();
		m_iUsedSlots = 0;
		m_fCurrentWeight = 0;

		for (int i = 0; i < m_iTotalSlots; i++)
		{
			IEntity item = m_Storage.Get(i);
			if (item)
			{
				m_iUsedSlots++;
				m_fCurrentWeight += RIP_InventoryHelper.GetItemWeight(item);
			}
		}

		// Calculate ratios
		if (m_iTotalSlots > 0)
			m_fSlotRatio = m_iUsedSlots / m_iTotalSlots;
		else
			m_fSlotRatio = 0;

		if (m_fMaxWeight > 0)
			m_fWeightRatio = Math.Clamp(m_fCurrentWeight / m_fMaxWeight, 0.0, 1.0);
		else
			m_fWeightRatio = m_fSlotRatio;

		// Use the higher ratio for capacity level
		float effectiveRatio = Math.Max(m_fWeightRatio, m_fSlotRatio);
		m_eCapacityLevel = RIP_Constants.GetCapacityLevel(effectiveRatio);
		m_iBarColor = RIP_Constants.GetCapacityColor(effectiveRatio);
	}

	// Get display string for the indicator
	string GetDisplayString()
	{
		if (m_fMaxWeight > 0)
		{
			return string.Format("%1: %2/%3 %4 (%5/%6 slots)",
				m_sStorageName,
				RIP_WeightIndicatorManager.FormatWeight(m_fCurrentWeight),
				RIP_WeightIndicatorManager.FormatWeight(m_fMaxWeight),
				RIP_Constants.WEIGHT_UNIT,
				m_iUsedSlots,
				m_iTotalSlots);
		}
		else
		{
			return string.Format("%1: %2/%3 slots",
				m_sStorageName,
				m_iUsedSlots,
				m_iTotalSlots);
		}
	}
}

class RIP_WeightIndicatorManager
{
	protected IEntity m_OwnerEntity;
	protected ref array<ref RIP_StorageInfo> m_aStorageInfos;
	protected float m_fTotalWeight;
	protected float m_fTotalMaxWeight;
	protected float m_fOverallRatio;
	protected RIP_ECapacityLevel m_eOverallLevel;
	protected int m_iOverallColor;
	protected float m_fUpdateTimer;
	protected static const float UPDATE_INTERVAL = 0.25;  // Update 4 times per second

	// Event callback when indicators change
	ref ScriptInvoker m_OnIndicatorsUpdated;

	void RIP_WeightIndicatorManager(IEntity owner)
	{
		m_OwnerEntity = owner;
		m_aStorageInfos = {};
		m_fTotalWeight = 0;
		m_fTotalMaxWeight = 0;
		m_fOverallRatio = 0;
		m_eOverallLevel = RIP_ECapacityLevel.LOW;
		m_iOverallColor = RIP_Constants.COLOR_CAPACITY_LOW;
		m_fUpdateTimer = 0;
		m_OnIndicatorsUpdated = new ScriptInvoker();
	}

	void ~RIP_WeightIndicatorManager()
	{
		m_aStorageInfos = null;
	}

	// Refresh the list of tracked storages
	void RefreshStorages()
	{
		m_aStorageInfos.Clear();

		SCR_InventoryStorageManagerComponent invManager = RIP_InventoryHelper.GetInventoryManager(m_OwnerEntity);
		if (!invManager)
			return;

		array<BaseInventoryStorageComponent> storages = {};
		invManager.GetStorages(storages);

		foreach (BaseInventoryStorageComponent storage : storages)
		{
			RIP_StorageInfo info = new RIP_StorageInfo(storage);

			// Try to get a readable name for the storage
			IEntity storageEntity = storage.GetOwner();
			if (storageEntity)
			{
				InventoryItemComponent invItem = InventoryItemComponent.Cast(
					storageEntity.FindComponent(InventoryItemComponent)
				);
				if (invItem)
				{
					UIInfo uiInfo = invItem.GetUIInfo();
					if (uiInfo)
						info.m_sStorageName = uiInfo.GetName();
				}
			}

			if (info.m_sStorageName.IsEmpty())
				info.m_sStorageName = "Storage";

			m_aStorageInfos.Insert(info);
		}
	}

	// Update all indicators
	void Update(float timeSlice)
	{
		m_fUpdateTimer += timeSlice;
		if (m_fUpdateTimer < UPDATE_INTERVAL)
			return;

		m_fUpdateTimer = 0;
		UpdateIndicators();
	}

	// Perform the actual indicator update
	protected void UpdateIndicators()
	{
		// Update overall weight from the manager (uses caching internally)
		m_fTotalWeight = RIP_InventoryHelper.GetTotalCharacterWeight(m_OwnerEntity);
		m_fTotalMaxWeight = RIP_InventoryHelper.GetMaxCharacterWeight(m_OwnerEntity);

		if (m_fTotalMaxWeight > 0)
			m_fOverallRatio = Math.Clamp(m_fTotalWeight / m_fTotalMaxWeight, 0.0, 1.0);
		else
			m_fOverallRatio = 0;

		m_eOverallLevel = RIP_Constants.GetCapacityLevel(m_fOverallRatio);
		m_iOverallColor = RIP_Constants.GetCapacityColor(m_fOverallRatio);

		// Update per-storage info
		foreach (RIP_StorageInfo info : m_aStorageInfos)
		{
			info.Update();
		}

		m_OnIndicatorsUpdated.Invoke();
	}

	// Force an immediate update
	void ForceUpdate()
	{
		m_fUpdateTimer = UPDATE_INTERVAL;
		UpdateIndicators();
	}

	// Getters for overall stats
	float GetTotalWeight() { return m_fTotalWeight; }
	float GetMaxWeight() { return m_fTotalMaxWeight; }
	float GetOverallRatio() { return m_fOverallRatio; }
	RIP_ECapacityLevel GetOverallLevel() { return m_eOverallLevel; }
	int GetOverallColor() { return m_iOverallColor; }

	// Get per-storage info array
	array<ref RIP_StorageInfo> GetStorageInfos()
	{
		return m_aStorageInfos;
	}

	// Get storage info count
	int GetStorageCount()
	{
		return m_aStorageInfos.Count();
	}

	// Get overall display string
	string GetOverallDisplayString()
	{
		return string.Format("Total: %1/%2 %3",
			FormatWeight(m_fTotalWeight),
			FormatWeight(m_fTotalMaxWeight),
			RIP_Constants.WEIGHT_UNIT);
	}

	// Format weight for display (e.g., "3.2" from 3.245)
	static string FormatWeight(float weight)
	{
		// Round to one decimal place
		float rounded = Math.Round(weight * 10) / 10;
		return rounded.ToString();
	}
}
