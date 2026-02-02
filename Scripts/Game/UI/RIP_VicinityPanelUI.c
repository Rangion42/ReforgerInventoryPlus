// Reforger Inventory Plus - Enhanced Vicinity Panel UI
// Larger, scrollable preview of nearby items with category grouping and search integration

class RIP_VicinityPanelUI : ScriptedWidgetComponent
{
	// Widget references
	protected Widget m_wRoot;
	protected Widget m_wItemListContainer;
	protected TextWidget m_wVicinitySummary;
	protected TextWidget m_wRangeIndicator;
	protected Widget m_wScrollArea;

	// Sort buttons
	protected ButtonWidget m_wBtnSortDistance;
	protected ButtonWidget m_wBtnSortCategory;
	protected ButtonWidget m_wBtnSortName;

	// State
	protected RIP_InventoryPlusComponent m_RIPComponent;
	protected bool m_bSortByDistance;
	protected ref array<ref RIP_VicinityItemEntry> m_aItemEntries;

	// Layout resource for individual item entries
	protected const string ITEM_ENTRY_LAYOUT = "{RIP_VicinityItemEntry}";

	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wRoot = w;

		// Find widgets
		m_wItemListContainer = w.FindAnyWidget("RIP_VicinityItemList");
		m_wVicinitySummary = TextWidget.Cast(w.FindAnyWidget("RIP_VicinitySummary"));
		m_wRangeIndicator = TextWidget.Cast(w.FindAnyWidget("RIP_RangeIndicator"));
		m_wScrollArea = w.FindAnyWidget("RIP_VicinityScrollArea");
		m_wBtnSortDistance = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnSortDistance"));
		m_wBtnSortCategory = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnSortCategory"));
		m_wBtnSortName = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnSortName"));

		m_bSortByDistance = true;
		m_aItemEntries = {};

		// Get RIP component and subscribe to vicinity updates
		m_RIPComponent = RIP_InventoryPlusComponent.GetInstance();
		if (m_RIPComponent)
		{
			RIP_VicinityManager vicinityManager = m_RIPComponent.GetVicinityManager();
			if (vicinityManager)
				vicinityManager.m_OnVicinityUpdated.Insert(OnVicinityUpdated);
		}
	}

	override void HandlerDeattached(Widget w)
	{
		if (m_RIPComponent)
		{
			RIP_VicinityManager vicinityManager = m_RIPComponent.GetVicinityManager();
			if (vicinityManager)
				vicinityManager.m_OnVicinityUpdated.Remove(OnVicinityUpdated);
		}

		ClearItemEntries();
		super.HandlerDeattached(w);
	}

	// Button click handler
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_wBtnSortDistance)
		{
			SortByDistance();
			return true;
		}
		if (w == m_wBtnSortCategory)
		{
			SortByCategory();
			return true;
		}
		if (w == m_wBtnSortName)
		{
			SortByName();
			return true;
		}

		return false;
	}

	// Callback when vicinity data is updated
	protected void OnVicinityUpdated()
	{
		RefreshDisplay();
	}

	// Refresh the entire vicinity display
	void RefreshDisplay()
	{
		if (!m_RIPComponent)
			return;

		RIP_VicinityManager vicinityManager = m_RIPComponent.GetVicinityManager();
		if (!vicinityManager)
			return;

		// Update summary text
		if (m_wVicinitySummary)
			m_wVicinitySummary.SetText(vicinityManager.GetVicinitySummary());

		if (m_wRangeIndicator)
			m_wRangeIndicator.SetText(string.Format("%1m", vicinityManager.GetScanRange()));

		// Get items (optionally filtered by active search)
		array<ref RIP_VicinityItem> items = vicinityManager.GetVicinityItems();

		// Apply current search filter if search is active
		RIP_SearchManager searchManager = m_RIPComponent.GetSearchManager();
		if (searchManager && searchManager.HasActiveFilters())
		{
			array<ref RIP_VicinityItem> filteredItems = {};
			vicinityManager.GetFilteredItems(
				searchManager.GetSearchQuery(),
				searchManager.GetCategoryFilter(),
				filteredItems
			);
			items = filteredItems;
		}

		// Sort items
		if (m_bSortByDistance)
			vicinityManager.SortByDistance();
		else
			vicinityManager.SortByCategory();

		// Rebuild item entries
		RebuildItemList(items);
	}

	// Rebuild the item list display
	protected void RebuildItemList(array<ref RIP_VicinityItem> items)
	{
		ClearItemEntries();

		if (!items)
			return;

		RIP_EItemCategory lastCategory = RIP_EItemCategory.NONE;

		foreach (RIP_VicinityItem item : items)
		{
			RIP_VicinityItemEntry entry = new RIP_VicinityItemEntry();
			entry.m_sName = item.m_sDisplayName;
			entry.m_eCategory = item.m_eCategory;
			entry.m_fDistance = item.m_fDistance;
			entry.m_bIsInContainer = item.m_Storage != null;
			entry.m_Entity = item.m_Entity;

			// Check if we need a category header
			if (!m_bSortByDistance && item.m_eCategory != lastCategory)
			{
				entry.m_bShowCategoryHeader = true;
				entry.m_sCategoryName = RIP_Constants.GetCategoryName(item.m_eCategory);
				lastCategory = item.m_eCategory;
			}

			m_aItemEntries.Insert(entry);
		}
	}

	// Clear all item entries
	protected void ClearItemEntries()
	{
		m_aItemEntries.Clear();
	}

	// Sort methods
	void SortByDistance()
	{
		m_bSortByDistance = true;
		RefreshDisplay();
		UpdateSortButtonHighlights();
	}

	void SortByCategory()
	{
		m_bSortByDistance = false;
		RefreshDisplay();
		UpdateSortButtonHighlights();
	}

	void SortByName()
	{
		m_bSortByDistance = false;
		if (m_RIPComponent)
		{
			RIP_VicinityManager vicinityManager = m_RIPComponent.GetVicinityManager();
			if (vicinityManager)
				vicinityManager.SortByCategory();  // Category sort includes alphabetical
		}
		RefreshDisplay();
		UpdateSortButtonHighlights();
	}

	// Update sort button visual state
	protected void UpdateSortButtonHighlights()
	{
		int activeColor = ARGB(255, 100, 180, 255);
		int inactiveColor = ARGB(255, 80, 80, 80);

		if (m_wBtnSortDistance)
			m_wBtnSortDistance.SetColor(m_bSortByDistance ? activeColor : inactiveColor);
		if (m_wBtnSortCategory)
			m_wBtnSortCategory.SetColor(!m_bSortByDistance ? activeColor : inactiveColor);
	}

	// Get item entries for external access
	array<ref RIP_VicinityItemEntry> GetItemEntries()
	{
		return m_aItemEntries;
	}
}

// Data class for a vicinity item display entry
class RIP_VicinityItemEntry
{
	string m_sName;
	RIP_EItemCategory m_eCategory;
	float m_fDistance;
	bool m_bIsInContainer;
	bool m_bShowCategoryHeader;
	string m_sCategoryName;
	IEntity m_Entity;

	void RIP_VicinityItemEntry()
	{
		m_sName = "";
		m_eCategory = RIP_EItemCategory.NONE;
		m_fDistance = 0;
		m_bIsInContainer = false;
		m_bShowCategoryHeader = false;
		m_sCategoryName = "";
	}

	// Get formatted distance string
	string GetDistanceString()
	{
		if (m_fDistance < 1.0)
			return "< 1m";
		return string.Format("%1m", Math.Round(m_fDistance));
	}

	// Get category icon/label text
	string GetCategoryLabel()
	{
		return RIP_Constants.GetCategoryName(m_eCategory);
	}
}
