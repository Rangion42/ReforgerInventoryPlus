// Reforger Inventory Plus - Search Bar UI Component
// Handles the search input field, category filter buttons, and result display

class RIP_SearchBarUI : ScriptedWidgetComponent
{
	// Widget references
	protected EditBoxWidget m_wSearchInput;
	protected TextWidget m_wResultCount;
	protected TextWidget m_wAggregateSummary;
	protected Widget m_wClearButton;
	protected Widget m_wCategoryButtonsContainer;

	// Category filter buttons
	protected ButtonWidget m_wBtnAll;
	protected ButtonWidget m_wBtnWeapons;
	protected ButtonWidget m_wBtnAmmo;
	protected ButtonWidget m_wBtnMedical;
	protected ButtonWidget m_wBtnEquipment;

	// State
	protected RIP_EItemCategory m_eActiveCategory;
	protected string m_sLastQuery;
	protected RIP_InventoryPlusComponent m_RIPComponent;

	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		// Find child widgets
		m_wSearchInput = EditBoxWidget.Cast(w.FindAnyWidget("RIP_SearchInput"));
		m_wResultCount = TextWidget.Cast(w.FindAnyWidget("RIP_ResultCount"));
		m_wAggregateSummary = TextWidget.Cast(w.FindAnyWidget("RIP_AggregateSummary"));
		m_wClearButton = w.FindAnyWidget("RIP_ClearButton");
		m_wCategoryButtonsContainer = w.FindAnyWidget("RIP_CategoryButtons");

		// Find category buttons
		m_wBtnAll = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnAll"));
		m_wBtnWeapons = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnWeapons"));
		m_wBtnAmmo = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnAmmo"));
		m_wBtnMedical = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnMedical"));
		m_wBtnEquipment = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnEquipment"));

		m_eActiveCategory = RIP_EItemCategory.ALL;
		m_sLastQuery = string.Empty;

		// Subscribe to search result updates
		m_RIPComponent = RIP_InventoryPlusComponent.GetInstance();
		if (m_RIPComponent)
		{
			RIP_SearchManager searchManager = m_RIPComponent.GetSearchManager();
			if (searchManager)
				searchManager.m_OnSearchResultsUpdated.Insert(OnSearchResultsUpdated);
		}

		UpdateCategoryButtonHighlights();
	}

	override void HandlerDeattached(Widget w)
	{
		// Unsubscribe from events
		if (m_RIPComponent)
		{
			RIP_SearchManager searchManager = m_RIPComponent.GetSearchManager();
			if (searchManager)
				searchManager.m_OnSearchResultsUpdated.Remove(OnSearchResultsUpdated);
		}

		super.HandlerDeattached(w);
	}

	// Called when search input text changes
	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		if (w == m_wSearchInput)
		{
			string query = m_wSearchInput.GetText();
			if (query != m_sLastQuery)
			{
				m_sLastQuery = query;
				OnSearchQueryChanged(query);
			}
			return true;
		}
		return false;
	}

	// Called when a button is clicked
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_wClearButton)
		{
			ClearSearch();
			return true;
		}

		if (w == m_wBtnAll) { SetCategoryFilter(RIP_EItemCategory.ALL); return true; }
		if (w == m_wBtnWeapons) { SetCategoryFilter(RIP_EItemCategory.WEAPONS); return true; }
		if (w == m_wBtnAmmo) { SetCategoryFilter(RIP_EItemCategory.AMMO); return true; }
		if (w == m_wBtnMedical) { SetCategoryFilter(RIP_EItemCategory.MEDICAL); return true; }
		if (w == m_wBtnEquipment) { SetCategoryFilter(RIP_EItemCategory.EQUIPMENT); return true; }

		return false;
	}

	// Search query changed handler
	protected void OnSearchQueryChanged(string query)
	{
		if (!m_RIPComponent)
			return;

		m_RIPComponent.Search(query);

		// Show/hide clear button
		if (m_wClearButton)
			m_wClearButton.SetVisible(!query.IsEmpty());
	}

	// Set category filter
	protected void SetCategoryFilter(RIP_EItemCategory category)
	{
		m_eActiveCategory = category;

		if (m_RIPComponent)
			m_RIPComponent.SetSearchCategory(category);

		UpdateCategoryButtonHighlights();
	}

	// Clear search and reset filters
	protected void ClearSearch()
	{
		m_sLastQuery = string.Empty;

		if (m_wSearchInput)
			m_wSearchInput.SetText(string.Empty);

		m_eActiveCategory = RIP_EItemCategory.ALL;

		if (m_RIPComponent)
			m_RIPComponent.ClearSearch();

		if (m_wClearButton)
			m_wClearButton.SetVisible(false);

		UpdateCategoryButtonHighlights();
	}

	// Update category button visual state
	protected void UpdateCategoryButtonHighlights()
	{
		SetButtonActive(m_wBtnAll, m_eActiveCategory == RIP_EItemCategory.ALL);
		SetButtonActive(m_wBtnWeapons, m_eActiveCategory == RIP_EItemCategory.WEAPONS);
		SetButtonActive(m_wBtnAmmo, m_eActiveCategory == RIP_EItemCategory.AMMO);
		SetButtonActive(m_wBtnMedical, m_eActiveCategory == RIP_EItemCategory.MEDICAL);
		SetButtonActive(m_wBtnEquipment, m_eActiveCategory == RIP_EItemCategory.EQUIPMENT);
	}

	// Set button active/inactive visual state
	protected void SetButtonActive(ButtonWidget btn, bool active)
	{
		if (!btn)
			return;

		if (active)
			btn.SetColor(ARGB(255, 100, 180, 255));  // Blue highlight for active
		else
			btn.SetColor(ARGB(255, 80, 80, 80));      // Gray for inactive
	}

	// Callback when search results are updated
	protected void OnSearchResultsUpdated()
	{
		if (!m_RIPComponent)
			return;

		RIP_SearchManager searchManager = m_RIPComponent.GetSearchManager();
		if (!searchManager)
			return;

		// Update result count
		int resultCount = searchManager.GetResultCount();
		if (m_wResultCount)
		{
			if (searchManager.HasActiveFilters())
				m_wResultCount.SetText(string.Format("%1 found", resultCount));
			else
				m_wResultCount.SetText(string.Empty);
		}

		// Update aggregate summary
		if (m_wAggregateSummary)
		{
			string summary = searchManager.GetAggregateSummary();
			m_wAggregateSummary.SetText(summary);
			m_wAggregateSummary.SetVisible(!summary.IsEmpty());
		}
	}

	// Focus the search input (can be called externally)
	void FocusSearchInput()
	{
		if (m_wSearchInput)
			GetGame().GetWorkspace().SetFocusedWidget(m_wSearchInput);
	}
}
