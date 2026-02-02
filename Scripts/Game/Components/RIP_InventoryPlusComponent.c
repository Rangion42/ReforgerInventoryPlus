// Reforger Inventory Plus - Main Component
// Attaches to a character entity and coordinates all RIP subsystems
// This is the primary entry point for the mod's functionality

[ComponentEditorProps(category: "Reforger Inventory Plus", description: "Main Inventory Plus component - attach to character prefab")]
class RIP_InventoryPlusComponentClass : ScriptComponentClass
{
}

class RIP_InventoryPlusComponent : ScriptComponent
{
	// Sub-managers
	protected ref RIP_SearchManager m_SearchManager;
	protected ref RIP_CategoryTransferManager m_TransferManager;
	protected ref RIP_WeightIndicatorManager m_WeightManager;
	protected ref RIP_VicinityManager m_VicinityManager;

	// State
	protected bool m_bInitialized;
	protected bool m_bInventoryOpen;
	protected IEntity m_OwnerEntity;

	// Singleton access for UI scripts
	protected static RIP_InventoryPlusComponent s_Instance;

	static RIP_InventoryPlusComponent GetInstance()
	{
		return s_Instance;
	}

	// Lifecycle

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
	}

	override void EOnInit(IEntity owner)
	{
		m_OwnerEntity = owner;

		// Only initialize on the local player's character (client-side)
		if (!IsLocalPlayer())
			return;

		InitializeManagers();
		s_Instance = this;
		m_bInitialized = true;
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_bInitialized || !m_bInventoryOpen)
			return;

		// Update all managers each frame while inventory is open
		if (m_SearchManager)
			m_SearchManager.Update(timeSlice);

		if (m_TransferManager)
			m_TransferManager.Update(timeSlice);

		if (m_WeightManager)
			m_WeightManager.Update(timeSlice);

		if (m_VicinityManager)
			m_VicinityManager.Update(timeSlice);
	}

	override void OnDelete(IEntity owner)
	{
		if (s_Instance == this)
			s_Instance = null;

		CleanupManagers();
		super.OnDelete(owner);
	}

	// Initialize all sub-managers
	protected void InitializeManagers()
	{
		RIP_Config config = RIP_Config.GetInstance();

		if (config.m_bEnableSearch)
			m_SearchManager = new RIP_SearchManager(m_OwnerEntity);

		if (config.m_bEnableQuickTransfer)
			m_TransferManager = new RIP_CategoryTransferManager(m_OwnerEntity);

		if (config.m_bEnableCapacityBars)
		{
			m_WeightManager = new RIP_WeightIndicatorManager(m_OwnerEntity);
			m_WeightManager.RefreshStorages();
		}

		if (config.m_bEnableEnhancedVicinity)
			m_VicinityManager = new RIP_VicinityManager(m_OwnerEntity);
	}

	// Cleanup all sub-managers
	protected void CleanupManagers()
	{
		m_SearchManager = null;
		m_TransferManager = null;
		m_WeightManager = null;
		m_VicinityManager = null;
		m_bInitialized = false;
	}

	// Check if the owner entity is the local player
	protected bool IsLocalPlayer()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return false;

		return pc.GetControlledEntity() == m_OwnerEntity;
	}

	// Called when inventory UI opens
	void OnInventoryOpened()
	{
		m_bInventoryOpen = true;

		// Refresh data when inventory opens
		if (m_WeightManager)
		{
			m_WeightManager.RefreshStorages();
			m_WeightManager.ForceUpdate();
		}

		if (m_VicinityManager)
			m_VicinityManager.ForceScan();

		if (m_SearchManager)
			m_SearchManager.ForceRefresh();
	}

	// Called when inventory UI closes
	void OnInventoryClosed()
	{
		m_bInventoryOpen = false;

		// Clear search when closing
		if (m_SearchManager)
			m_SearchManager.ClearSearch();

		// Cancel any pending transfers
		if (m_TransferManager && m_TransferManager.IsTransferInProgress())
			m_TransferManager.CancelTransfer();
	}

	// Public API for UI scripts

	// Search
	RIP_SearchManager GetSearchManager()
	{
		return m_SearchManager;
	}

	void Search(string query)
	{
		if (m_SearchManager)
			m_SearchManager.SetSearchQuery(query);
	}

	void SetSearchCategory(RIP_EItemCategory category)
	{
		if (m_SearchManager)
			m_SearchManager.SetCategoryFilter(category);
	}

	void ClearSearch()
	{
		if (m_SearchManager)
			m_SearchManager.ClearSearch();
	}

	// Transfer
	RIP_CategoryTransferManager GetTransferManager()
	{
		return m_TransferManager;
	}

	bool TransferCategory(RIP_EItemCategory category, RIP_ETransferDirection direction, BaseInventoryStorageComponent storage)
	{
		if (!m_TransferManager)
			return false;

		switch (direction)
		{
			case RIP_ETransferDirection.TO_INVENTORY:
				return m_TransferManager.TransferCategoryToInventory(category, storage);

			case RIP_ETransferDirection.TO_VICINITY:
			case RIP_ETransferDirection.TO_CONTAINER:
				return m_TransferManager.TransferCategoryToStorage(category, storage);
		}

		return false;
	}

	// Weight/Capacity
	RIP_WeightIndicatorManager GetWeightManager()
	{
		return m_WeightManager;
	}

	float GetWeightRatio()
	{
		if (m_WeightManager)
			return m_WeightManager.GetOverallRatio();
		return 0;
	}

	int GetWeightColor()
	{
		if (m_WeightManager)
			return m_WeightManager.GetOverallColor();
		return RIP_Constants.COLOR_CAPACITY_LOW;
	}

	// Vicinity
	RIP_VicinityManager GetVicinityManager()
	{
		return m_VicinityManager;
	}

	// Check if inventory is currently open
	bool IsInventoryOpen()
	{
		return m_bInventoryOpen;
	}

	// Check if the component is fully initialized
	bool IsInitialized()
	{
		return m_bInitialized;
	}
}
