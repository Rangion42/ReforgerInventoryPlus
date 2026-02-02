// Reforger Inventory Plus - Main UI Controller
// Coordinates all UI sub-components and manages the inventory overlay lifecycle

class RIP_InventoryPlusUI : ScriptedWidgetComponent
{
	// Sub-component references
	protected ref RIP_SearchBarUI m_SearchBar;
	protected ref RIP_CapacityBarUI m_CapacityBar;
	protected ref RIP_QuickTransferUI m_QuickTransfer;
	protected ref RIP_VicinityPanelUI m_VicinityPanel;

	// Widget references
	protected Widget m_wRoot;
	protected Widget m_wSearchBarContainer;
	protected Widget m_wCapacityBarContainer;
	protected Widget m_wQuickTransferContainer;
	protected Widget m_wVicinityPanelContainer;
	protected Widget m_wOverlayContainer;

	// Layout paths
	static const ResourceName SEARCH_BAR_LAYOUT = "{RIP_SearchBar}";
	static const ResourceName CAPACITY_BAR_LAYOUT = "{RIP_CapacityBar}";
	static const ResourceName QUICK_TRANSFER_LAYOUT = "{RIP_QuickTransferPanel}";
	static const ResourceName VICINITY_PANEL_LAYOUT = "{RIP_VicinityPanel}";

	// State
	protected RIP_InventoryPlusComponent m_RIPComponent;
	protected bool m_bUIInitialized;

	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wRoot = w;
		m_bUIInitialized = false;

		// Find container widgets
		m_wSearchBarContainer = w.FindAnyWidget("RIP_SearchBarSlot");
		m_wCapacityBarContainer = w.FindAnyWidget("RIP_CapacityBarSlot");
		m_wQuickTransferContainer = w.FindAnyWidget("RIP_QuickTransferSlot");
		m_wVicinityPanelContainer = w.FindAnyWidget("RIP_VicinityPanelSlot");
		m_wOverlayContainer = w.FindAnyWidget("RIP_OverlayContainer");

		// Defer initialization until the RIP component is available
		GetGame().GetCallqueue().CallLater(InitializeUI, 100, false);
	}

	override void HandlerDeattached(Widget w)
	{
		if (m_RIPComponent)
			m_RIPComponent.OnInventoryClosed();

		m_bUIInitialized = false;
		super.HandlerDeattached(w);
	}

	// Initialize UI components
	protected void InitializeUI()
	{
		m_RIPComponent = RIP_InventoryPlusComponent.GetInstance();
		if (!m_RIPComponent)
		{
			// Retry if component not ready yet
			GetGame().GetCallqueue().CallLater(InitializeUI, 200, false);
			return;
		}

		RIP_Config config = RIP_Config.GetInstance();

		// Initialize sub-components based on config
		if (config.m_bEnableSearch)
			InitSearchBar();

		if (config.m_bEnableCapacityBars)
			InitCapacityBar();

		if (config.m_bEnableQuickTransfer)
			InitQuickTransfer();

		if (config.m_bEnableEnhancedVicinity)
			InitVicinityPanel();

		// Notify the component that inventory is open
		m_RIPComponent.OnInventoryOpened();
		m_bUIInitialized = true;
	}

	// Initialize search bar
	protected void InitSearchBar()
	{
		if (!m_wSearchBarContainer)
			return;

		// The search bar handler is attached via the layout XML
		m_SearchBar = RIP_SearchBarUI.Cast(
			m_wSearchBarContainer.FindHandler(RIP_SearchBarUI)
		);
	}

	// Initialize capacity bar
	protected void InitCapacityBar()
	{
		if (!m_wCapacityBarContainer)
			return;

		m_CapacityBar = RIP_CapacityBarUI.Cast(
			m_wCapacityBarContainer.FindHandler(RIP_CapacityBarUI)
		);
	}

	// Initialize quick transfer buttons
	protected void InitQuickTransfer()
	{
		if (!m_wQuickTransferContainer)
			return;

		m_QuickTransfer = RIP_QuickTransferUI.Cast(
			m_wQuickTransferContainer.FindHandler(RIP_QuickTransferUI)
		);
	}

	// Initialize vicinity panel
	protected void InitVicinityPanel()
	{
		if (!m_wVicinityPanelContainer)
			return;

		m_VicinityPanel = RIP_VicinityPanelUI.Cast(
			m_wVicinityPanelContainer.FindHandler(RIP_VicinityPanelUI)
		);
	}

	// Focus the search bar input (called by keyboard shortcut)
	void FocusSearch()
	{
		if (m_SearchBar)
			m_SearchBar.FocusSearchInput();
	}

	// Set the target storage for quick transfers
	void SetTransferTarget(BaseInventoryStorageComponent storage)
	{
		if (m_QuickTransfer)
			m_QuickTransfer.SetTargetStorage(storage);
	}

	// Refresh all UI components
	void RefreshAll()
	{
		if (!m_RIPComponent)
			return;

		RIP_SearchManager searchManager = m_RIPComponent.GetSearchManager();
		if (searchManager)
			searchManager.ForceRefresh();

		RIP_WeightIndicatorManager weightManager = m_RIPComponent.GetWeightManager();
		if (weightManager)
			weightManager.ForceUpdate();

		RIP_VicinityManager vicinityManager = m_RIPComponent.GetVicinityManager();
		if (vicinityManager)
			vicinityManager.ForceScan();

		if (m_VicinityPanel)
			m_VicinityPanel.RefreshDisplay();
	}

	// Show/hide the entire overlay
	void SetVisible(bool visible)
	{
		if (m_wRoot)
			m_wRoot.SetVisible(visible);

		if (visible && m_RIPComponent)
			m_RIPComponent.OnInventoryOpened();
		else if (!visible && m_RIPComponent)
			m_RIPComponent.OnInventoryClosed();
	}

	// Check if UI is initialized
	bool IsInitialized()
	{
		return m_bUIInitialized;
	}

	// Get sub-component references
	RIP_SearchBarUI GetSearchBar() { return m_SearchBar; }
	RIP_CapacityBarUI GetCapacityBar() { return m_CapacityBar; }
	RIP_QuickTransferUI GetQuickTransfer() { return m_QuickTransfer; }
	RIP_VicinityPanelUI GetVicinityPanel() { return m_VicinityPanel; }
}
