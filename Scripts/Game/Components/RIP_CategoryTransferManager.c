// Reforger Inventory Plus - Category Transfer Manager
// Handles batch transfer of items by category between storages
// Server-authoritative: all actual moves go through InventoryStorageManagerComponent

class RIP_TransferOperation
{
	ref array<IEntity> m_aItems;
	BaseInventoryStorageComponent m_TargetStorage;
	int m_iCurrentIndex;
	bool m_bComplete;

	void RIP_TransferOperation()
	{
		m_aItems = {};
		m_iCurrentIndex = 0;
		m_bComplete = false;
	}
}

class RIP_CategoryTransferManager
{
	protected IEntity m_OwnerEntity;
	protected ref RIP_TransferOperation m_CurrentOperation;
	protected bool m_bTransferInProgress;

	// Event callbacks
	ref ScriptInvoker m_OnTransferStarted;     // (RIP_EItemCategory category, int itemCount)
	ref ScriptInvoker m_OnTransferProgress;    // (int transferred, int total)
	ref ScriptInvoker m_OnTransferCompleted;   // (int totalTransferred)
	ref ScriptInvoker m_OnTransferFailed;      // (string reason)

	void RIP_CategoryTransferManager(IEntity owner)
	{
		m_OwnerEntity = owner;
		m_bTransferInProgress = false;
		m_OnTransferStarted = new ScriptInvoker();
		m_OnTransferProgress = new ScriptInvoker();
		m_OnTransferCompleted = new ScriptInvoker();
		m_OnTransferFailed = new ScriptInvoker();
	}

	void ~RIP_CategoryTransferManager()
	{
		m_CurrentOperation = null;
	}

	// Check if a transfer is currently in progress
	bool IsTransferInProgress()
	{
		return m_bTransferInProgress;
	}

	// Start a category transfer from player inventory to a target storage (vicinity/container)
	bool TransferCategoryToStorage(RIP_EItemCategory category, BaseInventoryStorageComponent targetStorage)
	{
		if (m_bTransferInProgress)
		{
			m_OnTransferFailed.Invoke("Transfer already in progress");
			return false;
		}

		if (!targetStorage)
		{
			m_OnTransferFailed.Invoke("No target storage specified");
			return false;
		}

		// Get all items of this category from character
		array<IEntity> items = {};
		RIP_InventoryHelper.GetAllCharacterItems(m_OwnerEntity, category, items);

		if (items.Count() == 0)
		{
			m_OnTransferFailed.Invoke("No items of this category found");
			return false;
		}

		return StartTransfer(items, targetStorage, category);
	}

	// Start a category transfer from vicinity/container to player inventory
	bool TransferCategoryToInventory(RIP_EItemCategory category, BaseInventoryStorageComponent sourceStorage)
	{
		if (m_bTransferInProgress)
		{
			m_OnTransferFailed.Invoke("Transfer already in progress");
			return false;
		}

		SCR_InventoryStorageManagerComponent invManager = RIP_InventoryHelper.GetInventoryManager(m_OwnerEntity);
		if (!invManager)
		{
			m_OnTransferFailed.Invoke("No inventory manager found");
			return false;
		}

		// Get items from source storage matching category
		array<IEntity> items = {};
		RIP_InventoryHelper.GetItemsFromStorage(sourceStorage, category, items);

		if (items.Count() == 0)
		{
			m_OnTransferFailed.Invoke("No items of this category in source");
			return false;
		}

		// Find the character's primary storage as target
		array<BaseInventoryStorageComponent> storages = {};
		invManager.GetStorages(storages);

		if (storages.Count() == 0)
		{
			m_OnTransferFailed.Invoke("No character storage available");
			return false;
		}

		// Use the first available storage (backpack or clothing storage)
		return StartTransfer(items, storages[0], category);
	}

	// Transfer all items of a category between two arbitrary storages
	bool TransferCategoryBetweenStorages(RIP_EItemCategory category, BaseInventoryStorageComponent sourceStorage, BaseInventoryStorageComponent targetStorage)
	{
		if (m_bTransferInProgress)
		{
			m_OnTransferFailed.Invoke("Transfer already in progress");
			return false;
		}

		if (!sourceStorage || !targetStorage)
		{
			m_OnTransferFailed.Invoke("Invalid source or target storage");
			return false;
		}

		array<IEntity> items = {};
		RIP_InventoryHelper.GetItemsFromStorage(sourceStorage, category, items);

		if (items.Count() == 0)
		{
			m_OnTransferFailed.Invoke("No items of this category in source");
			return false;
		}

		return StartTransfer(items, targetStorage, category);
	}

	// Internal: Start the batch transfer operation
	protected bool StartTransfer(array<IEntity> items, BaseInventoryStorageComponent targetStorage, RIP_EItemCategory category)
	{
		m_CurrentOperation = new RIP_TransferOperation();
		m_CurrentOperation.m_aItems.Copy(items);
		m_CurrentOperation.m_TargetStorage = targetStorage;
		m_CurrentOperation.m_iCurrentIndex = 0;
		m_CurrentOperation.m_bComplete = false;

		m_bTransferInProgress = true;
		m_OnTransferStarted.Invoke(category, items.Count());

		return true;
	}

	// Process transfer batch - call this from the frame update
	// Processes a limited number of items per frame to avoid hitching
	void Update(float timeSlice)
	{
		if (!m_bTransferInProgress || !m_CurrentOperation)
			return;

		if (m_CurrentOperation.m_bComplete)
		{
			CompleteTransfer();
			return;
		}

		SCR_InventoryStorageManagerComponent invManager = RIP_InventoryHelper.GetInventoryManager(m_OwnerEntity);
		if (!invManager)
		{
			FailTransfer("Inventory manager lost during transfer");
			return;
		}

		int batchSize = RIP_Config.GetInstance().m_iTransferBatchSize;
		int processed = 0;
		int total = m_CurrentOperation.m_aItems.Count();

		while (m_CurrentOperation.m_iCurrentIndex < total && processed < batchSize)
		{
			IEntity item = m_CurrentOperation.m_aItems[m_CurrentOperation.m_iCurrentIndex];

			if (item)
			{
				// Use TryMoveItemToStorage for server-authoritative movement
				bool moved = invManager.TryMoveItemToStorage(item, m_CurrentOperation.m_TargetStorage);
				if (!moved)
				{
					// Try insert if move fails (item may be from vicinity)
					invManager.TryInsertItem(item);
				}
			}

			m_CurrentOperation.m_iCurrentIndex++;
			processed++;
		}

		// Report progress
		m_OnTransferProgress.Invoke(m_CurrentOperation.m_iCurrentIndex, total);

		// Check if complete
		if (m_CurrentOperation.m_iCurrentIndex >= total)
		{
			m_CurrentOperation.m_bComplete = true;
		}
	}

	// Complete the transfer operation
	protected void CompleteTransfer()
	{
		int totalTransferred = 0;
		if (m_CurrentOperation)
			totalTransferred = m_CurrentOperation.m_aItems.Count();

		m_bTransferInProgress = false;
		m_CurrentOperation = null;

		m_OnTransferCompleted.Invoke(totalTransferred);
	}

	// Fail the transfer operation
	protected void FailTransfer(string reason)
	{
		m_bTransferInProgress = false;
		m_CurrentOperation = null;

		m_OnTransferFailed.Invoke(reason);
	}

	// Cancel an in-progress transfer
	void CancelTransfer()
	{
		if (!m_bTransferInProgress)
			return;

		m_bTransferInProgress = false;
		m_CurrentOperation = null;
	}

	// Quick convenience methods for each category

	bool TransferAllWeapons(BaseInventoryStorageComponent target)
	{
		return TransferCategoryToStorage(RIP_EItemCategory.WEAPONS, target);
	}

	bool TransferAllAmmo(BaseInventoryStorageComponent target)
	{
		return TransferCategoryToStorage(RIP_EItemCategory.AMMO, target);
	}

	bool TransferAllMedical(BaseInventoryStorageComponent target)
	{
		return TransferCategoryToStorage(RIP_EItemCategory.MEDICAL, target);
	}

	bool TransferAllEquipment(BaseInventoryStorageComponent target)
	{
		return TransferCategoryToStorage(RIP_EItemCategory.EQUIPMENT, target);
	}

	bool TransferAll(BaseInventoryStorageComponent target)
	{
		return TransferCategoryToStorage(RIP_EItemCategory.ALL, target);
	}

	// Pick up all items of a category from a source
	bool PickUpAllWeapons(BaseInventoryStorageComponent source)
	{
		return TransferCategoryToInventory(RIP_EItemCategory.WEAPONS, source);
	}

	bool PickUpAllAmmo(BaseInventoryStorageComponent source)
	{
		return TransferCategoryToInventory(RIP_EItemCategory.AMMO, source);
	}

	bool PickUpAllMedical(BaseInventoryStorageComponent source)
	{
		return TransferCategoryToInventory(RIP_EItemCategory.MEDICAL, source);
	}

	bool PickUpAllEquipment(BaseInventoryStorageComponent source)
	{
		return TransferCategoryToInventory(RIP_EItemCategory.EQUIPMENT, source);
	}

	bool PickUpAll(BaseInventoryStorageComponent source)
	{
		return TransferCategoryToInventory(RIP_EItemCategory.ALL, source);
	}
}
