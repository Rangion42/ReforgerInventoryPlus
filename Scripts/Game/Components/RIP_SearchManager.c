// Reforger Inventory Plus - Search Manager
// Handles search queries, result caching, and aggregate counts
// Runs client-side only - no network overhead

class RIP_SearchResult
{
	IEntity m_Entity;
	string m_sDisplayName;
	RIP_EItemCategory m_eCategory;
	BaseInventoryStorageComponent m_Storage;

	void RIP_SearchResult(IEntity entity, string displayName, RIP_EItemCategory category, BaseInventoryStorageComponent storage)
	{
		m_Entity = entity;
		m_sDisplayName = displayName;
		m_eCategory = category;
		m_Storage = storage;
	}
}

class RIP_AggregateCount
{
	string m_sItemName;
	int m_iCount;
	int m_iCurrentAmmo;  // For ammo items
	int m_iMaxAmmo;      // For ammo items

	void RIP_AggregateCount(string name)
	{
		m_sItemName = name;
		m_iCount = 0;
		m_iCurrentAmmo = 0;
		m_iMaxAmmo = 0;
	}
}

class RIP_SearchManager
{
	protected IEntity m_OwnerEntity;
	protected string m_sCurrentQuery;
	protected RIP_EItemCategory m_eCurrentFilter;
	protected ref array<ref RIP_SearchResult> m_aResults;
	protected ref map<string, ref RIP_AggregateCount> m_mAggregateCounts;
	protected float m_fLastSearchTime;
	protected bool m_bSearchDirty;

	// Event callbacks
	ref ScriptInvoker m_OnSearchResultsUpdated;

	void RIP_SearchManager(IEntity owner)
	{
		m_OwnerEntity = owner;
		m_sCurrentQuery = string.Empty;
		m_eCurrentFilter = RIP_EItemCategory.ALL;
		m_aResults = {};
		m_mAggregateCounts = new map<string, ref RIP_AggregateCount>();
		m_fLastSearchTime = 0;
		m_bSearchDirty = false;
		m_OnSearchResultsUpdated = new ScriptInvoker();
	}

	void ~RIP_SearchManager()
	{
		m_aResults = null;
		m_mAggregateCounts = null;
	}

	// Set the search query text
	void SetSearchQuery(string query)
	{
		if (m_sCurrentQuery == query)
			return;

		m_sCurrentQuery = query;
		m_bSearchDirty = true;
	}

	// Set the category filter
	void SetCategoryFilter(RIP_EItemCategory category)
	{
		if (m_eCurrentFilter == category)
			return;

		m_eCurrentFilter = category;
		m_bSearchDirty = true;
	}

	// Get current search query
	string GetSearchQuery()
	{
		return m_sCurrentQuery;
	}

	// Get current filter category
	RIP_EItemCategory GetCategoryFilter()
	{
		return m_eCurrentFilter;
	}

	// Execute search across all character storages
	// This runs client-side by enumerating local storage contents - no network overhead
	void ExecuteSearch()
	{
		m_aResults.Clear();
		m_mAggregateCounts.Clear();

		SCR_InventoryStorageManagerComponent invManager = RIP_InventoryHelper.GetInventoryManager(m_OwnerEntity);
		if (!invManager)
			return;

		// Get all storages
		array<BaseInventoryStorageComponent> storages = {};
		invManager.GetStorages(storages);

		string lowerQuery = m_sCurrentQuery;
		lowerQuery.ToLower();

		foreach (BaseInventoryStorageComponent storage : storages)
		{
			SearchStorage(storage, lowerQuery);
		}

		// Sort results by category then name
		SortResults();

		m_bSearchDirty = false;
		m_fLastSearchTime = System.GetTickCount();

		// Notify listeners
		m_OnSearchResultsUpdated.Invoke();
	}

	// Search a specific storage
	protected void SearchStorage(BaseInventoryStorageComponent storage, string lowerQuery)
	{
		if (!storage)
			return;

		int slotCount = storage.GetSlotsCount();
		for (int i = 0; i < slotCount; i++)
		{
			IEntity item = storage.Get(i);
			if (!item)
				continue;

			// Apply category filter
			RIP_EItemCategory itemCategory = RIP_InventoryHelper.ClassifyItem(item);
			if (m_eCurrentFilter != RIP_EItemCategory.ALL && itemCategory != m_eCurrentFilter)
				continue;

			// Apply text search filter
			string itemName = RIP_InventoryHelper.GetItemDisplayName(item);
			if (!lowerQuery.IsEmpty())
			{
				string lowerName = itemName;
				lowerName.ToLower();

				string lowerDesc = RIP_InventoryHelper.GetItemDescription(item);
				lowerDesc.ToLower();

				if (!lowerName.Contains(lowerQuery) && !lowerDesc.Contains(lowerQuery))
					continue;
			}

			// Add to results
			RIP_SearchResult result = new RIP_SearchResult(item, itemName, itemCategory, storage);
			m_aResults.Insert(result);

			// Update aggregate counts
			UpdateAggregateCount(item, itemName, itemCategory);

			// Enforce max results
			if (m_aResults.Count() >= RIP_Constants.SEARCH_MAX_RESULTS)
				return;
		}
	}

	// Update aggregate counts for item grouping
	protected void UpdateAggregateCount(IEntity item, string itemName, RIP_EItemCategory category)
	{
		RIP_AggregateCount aggregate;
		if (m_mAggregateCounts.Contains(itemName))
		{
			aggregate = m_mAggregateCounts.Get(itemName);
		}
		else
		{
			aggregate = new RIP_AggregateCount(itemName);
			m_mAggregateCounts.Insert(itemName, aggregate);
		}

		aggregate.m_iCount++;

		// Track ammo counts for magazines
		if (category == RIP_EItemCategory.AMMO)
		{
			aggregate.m_iCurrentAmmo += RIP_InventoryHelper.GetMagazineAmmoCount(item);
			aggregate.m_iMaxAmmo += RIP_InventoryHelper.GetMagazineMaxAmmoCount(item);
		}
	}

	// Sort results by category first, then alphabetically by name
	protected void SortResults()
	{
		// Simple insertion sort (suitable for expected result count)
		int count = m_aResults.Count();
		for (int i = 1; i < count; i++)
		{
			RIP_SearchResult key = m_aResults[i];
			int j = i - 1;

			while (j >= 0 && CompareResults(m_aResults[j], key) > 0)
			{
				m_aResults[j + 1] = m_aResults[j];
				j--;
			}
			m_aResults[j + 1] = key;
		}
	}

	// Compare two search results for sorting
	protected int CompareResults(RIP_SearchResult a, RIP_SearchResult b)
	{
		// Sort by category first
		if (a.m_eCategory != b.m_eCategory)
			return a.m_eCategory - b.m_eCategory;

		// Then alphabetically by name
		return a.m_sDisplayName.Compare(b.m_sDisplayName);
	}

	// Update search if dirty (called from frame update with debouncing)
	void Update(float timeSlice)
	{
		if (!m_bSearchDirty)
			return;

		float currentTime = System.GetTickCount();
		int debounceMs = RIP_Config.GetInstance().m_iSearchDebounceMs;

		if (currentTime - m_fLastSearchTime >= debounceMs)
		{
			ExecuteSearch();
		}
	}

	// Force an immediate refresh (bypass debounce)
	void ForceRefresh()
	{
		m_bSearchDirty = true;
		ExecuteSearch();
	}

	// Clear search and show all items
	void ClearSearch()
	{
		m_sCurrentQuery = string.Empty;
		m_eCurrentFilter = RIP_EItemCategory.ALL;
		m_bSearchDirty = true;
		ExecuteSearch();
	}

	// Get search results
	array<ref RIP_SearchResult> GetResults()
	{
		return m_aResults;
	}

	// Get result count
	int GetResultCount()
	{
		return m_aResults.Count();
	}

	// Get aggregate counts
	map<string, ref RIP_AggregateCount> GetAggregateCounts()
	{
		return m_mAggregateCounts;
	}

	// Get a formatted aggregate summary string
	string GetAggregateSummary()
	{
		string summary = "";
		for (int i = 0; i < m_mAggregateCounts.Count(); i++)
		{
			string name = m_mAggregateCounts.GetKey(i);
			RIP_AggregateCount agg = m_mAggregateCounts.GetElement(i);

			if (summary.Length() > 0)
				summary += "\n";

			if (agg.m_iMaxAmmo > 0)
			{
				summary += string.Format("%1 x%2 (%3/%4)", name, agg.m_iCount, agg.m_iCurrentAmmo, agg.m_iMaxAmmo);
			}
			else
			{
				summary += string.Format("%1 x%2", name, agg.m_iCount);
			}
		}
		return summary;
	}

	// Check if search has active filters
	bool HasActiveFilters()
	{
		return !m_sCurrentQuery.IsEmpty() || m_eCurrentFilter != RIP_EItemCategory.ALL;
	}
}
