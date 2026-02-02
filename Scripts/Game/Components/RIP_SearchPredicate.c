// Reforger Inventory Plus - Search Predicate
// Custom InventorySearchPredicate for efficient item querying

class RIP_SearchPredicate : InventorySearchPredicate
{
	protected string m_sSearchQuery;
	protected RIP_EItemCategory m_eFilterCategory;

	void RIP_SearchPredicate()
	{
		m_sSearchQuery = string.Empty;
		m_eFilterCategory = RIP_EItemCategory.ALL;
		QueryComponentTypes.Insert(InventoryItemComponent);
	}

	void SetSearchQuery(string query)
	{
		m_sSearchQuery = query;
		m_sSearchQuery.ToLower();
	}

	void SetFilterCategory(RIP_EItemCategory category)
	{
		m_eFilterCategory = category;
	}

	// Override to filter items based on search query and category
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		if (!item)
			return false;

		// Category filter
		if (m_eFilterCategory != RIP_EItemCategory.ALL)
		{
			RIP_EItemCategory itemCategory = RIP_InventoryHelper.ClassifyItem(item);
			if (itemCategory != m_eFilterCategory)
				return false;
		}

		// Text search filter
		if (!m_sSearchQuery.IsEmpty())
		{
			return RIP_InventoryHelper.MatchesSearchQuery(item, m_sSearchQuery);
		}

		return true;
	}
}
