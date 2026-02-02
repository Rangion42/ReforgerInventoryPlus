// Reforger Inventory Plus - Quick Drop Manager
// Handles rapid item dropping/deletion from inventory
// Server-authoritative: all drop operations go through InventoryStorageManagerComponent

class RIP_QuickDropManager
{
	protected IEntity m_OwnerEntity;
	protected ref array<IEntity> m_aMarkedItems;
	protected bool m_bEnabled;

	// Event callbacks
	ref ScriptInvoker m_OnItemMarkedForDrop;    // (IEntity item)
	ref ScriptInvoker m_OnItemUnmarked;         // (IEntity item)
	ref ScriptInvoker m_OnDropStarted;          // (int itemCount)
	ref ScriptInvoker m_OnDropCompleted;        // (int droppedCount)
	ref ScriptInvoker m_OnDropFailed;           // (string reason)

	void RIP_QuickDropManager(IEntity owner)
	{
		m_OwnerEntity = owner;
		m_aMarkedItems = {};
		m_bEnabled = true;
		m_OnItemMarkedForDrop = new ScriptInvoker();
		m_OnItemUnmarked = new ScriptInvoker();
		m_OnDropStarted = new ScriptInvoker();
		m_OnDropCompleted = new ScriptInvoker();
		m_OnDropFailed = new ScriptInvoker();
	}

	void ~RIP_QuickDropManager()
	{
		m_aMarkedItems = null;
	}

	// Enable/disable quick drop functionality
	void SetEnabled(bool enabled)
	{
		m_bEnabled = enabled;
	}

	bool IsEnabled()
	{
		return m_bEnabled;
	}

	// Mark an item for quick drop
	bool MarkItemForDrop(IEntity item)
	{
		if (!item || !m_bEnabled)
			return false;

		// Check if already marked
		if (m_aMarkedItems.Contains(item))
			return false;

		m_aMarkedItems.Insert(item);
		m_OnItemMarkedForDrop.Invoke(item);
		return true;
	}

	// Unmark an item
	bool UnmarkItem(IEntity item)
	{
		if (!item)
			return false;

		int index = m_aMarkedItems.Find(item);
		if (index == -1)
			return false;

		m_aMarkedItems.Remove(index);
		m_OnItemUnmarked.Invoke(item);
		return true;
	}

	// Toggle mark status for an item
	bool ToggleItemMark(IEntity item)
	{
		if (IsItemMarked(item))
			return UnmarkItem(item);
		else
			return MarkItemForDrop(item);
	}

	// Check if an item is marked for drop
	bool IsItemMarked(IEntity item)
	{
		return m_aMarkedItems.Contains(item);
	}

	// Clear all marks
	void ClearMarkedItems()
	{
		m_aMarkedItems.Clear();
	}

	// Get count of marked items
	int GetMarkedItemCount()
	{
		return m_aMarkedItems.Count();
	}

	// Get all marked items
	array<IEntity> GetMarkedItems()
	{
		return m_aMarkedItems;
	}

	// Drop all marked items
	bool DropMarkedItems()
	{
		if (!m_bEnabled)
		{
			m_OnDropFailed.Invoke("Quick drop is disabled");
			return false;
		}

		if (m_aMarkedItems.Count() == 0)
		{
			m_OnDropFailed.Invoke("No items marked for drop");
			return false;
		}

		SCR_InventoryStorageManagerComponent invManager = RIP_InventoryHelper.GetInventoryManager(m_OwnerEntity);
		if (!invManager)
		{
			m_OnDropFailed.Invoke("Inventory manager not found");
			return false;
		}

		int totalItems = m_aMarkedItems.Count();
		m_OnDropStarted.Invoke(totalItems);

		int droppedCount = 0;
		// Create a copy of the array to avoid modification during iteration
		array<IEntity> itemsToDrop = {};
		itemsToDrop.Copy(m_aMarkedItems);

		foreach (IEntity item : itemsToDrop)
		{
			if (item)
			{
				// Try to remove the item from inventory (drops it on ground)
				bool dropped = invManager.TryRemoveItemFromInventory(item);
				if (dropped)
					droppedCount++;
			}
		}

		// Clear the marked items list
		m_aMarkedItems.Clear();

		m_OnDropCompleted.Invoke(droppedCount);
		return true;
	}

	// Quick drop items by category
	bool DropItemsByCategory(RIP_EItemCategory category)
	{
		if (!m_bEnabled)
		{
			m_OnDropFailed.Invoke("Quick drop is disabled");
			return false;
		}

		// Get all items of this category
		array<IEntity> items = {};
		RIP_InventoryHelper.GetAllCharacterItems(m_OwnerEntity, category, items);

		if (items.Count() == 0)
		{
			m_OnDropFailed.Invoke("No items of this category found");
			return false;
		}

		// Mark all items
		foreach (IEntity item : items)
		{
			MarkItemForDrop(item);
		}

		return true;
	}

	// Drop a single item immediately (bypass marking)
	bool DropItemImmediate(IEntity item)
	{
		if (!item || !m_bEnabled)
			return false;

		SCR_InventoryStorageManagerComponent invManager = RIP_InventoryHelper.GetInventoryManager(m_OwnerEntity);
		if (!invManager)
			return false;

		return invManager.TryRemoveItemFromInventory(item);
	}

	// Delete all marked items (remove from game, not drop)
	bool DeleteMarkedItems()
	{
		if (!m_bEnabled)
		{
			m_OnDropFailed.Invoke("Quick drop is disabled");
			return false;
		}

		if (m_aMarkedItems.Count() == 0)
		{
			m_OnDropFailed.Invoke("No items marked for deletion");
			return false;
		}

		SCR_InventoryStorageManagerComponent invManager = RIP_InventoryHelper.GetInventoryManager(m_OwnerEntity);
		if (!invManager)
		{
			m_OnDropFailed.Invoke("Inventory manager not found");
			return false;
		}

		int totalItems = m_aMarkedItems.Count();
		m_OnDropStarted.Invoke(totalItems);

		int deletedCount = 0;
		// Create a copy to avoid modification during iteration
		array<IEntity> itemsToDelete = {};
		itemsToDelete.Copy(m_aMarkedItems);

		foreach (IEntity item : itemsToDelete)
		{
			if (item)
			{
				// Try to delete the item completely
				bool deleted = invManager.TryDeleteItem(item);
				if (deleted)
					deletedCount++;
			}
		}

		// Clear the marked items list
		m_aMarkedItems.Clear();

		m_OnDropCompleted.Invoke(deletedCount);
		return true;
	}
}
