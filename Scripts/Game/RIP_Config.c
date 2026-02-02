// Reforger Inventory Plus - Configuration
// Manages mod settings and user preferences

[BaseContainerProps(configRoot: true)]
class RIP_Config
{
	// Phase 1: Core Features
	[Attribute("true", UIWidgets.CheckBox, desc: "Enable search bar in inventory")]
	bool m_bEnableSearch;

	[Attribute("true", UIWidgets.CheckBox, desc: "Enable visual capacity indicators")]
	bool m_bEnableCapacityBars;

	[Attribute("true", UIWidgets.CheckBox, desc: "Enable quick transfer buttons")]
	bool m_bEnableQuickTransfer;

	[Attribute("true", UIWidgets.CheckBox, desc: "Enable enhanced vicinity view")]
	bool m_bEnableEnhancedVicinity;

	// Phase 2: Quality of Life Features
	[Attribute("true", UIWidgets.CheckBox, desc: "Enable quick drop for rapid item disposal")]
	bool m_bEnableQuickDrop;

	[Attribute("true", UIWidgets.CheckBox, desc: "Enable magazine consolidation")]
	bool m_bEnableMagazineConsolidation;

	[Attribute("3.0", UIWidgets.Slider, desc: "Vicinity detection range (meters)", params: "1.0 10.0 0.5")]
	float m_fVicinityRange;

	[Attribute("0.5", UIWidgets.Slider, desc: "Vicinity refresh rate (seconds)", params: "0.1 2.0 0.1")]
	float m_fVicinityRefreshRate;

	[Attribute("200", UIWidgets.Slider, desc: "Search debounce time (milliseconds)", params: "50 500 50")]
	int m_iSearchDebounceMs;

	[Attribute("10", UIWidgets.Slider, desc: "Transfer batch size (items per frame)", params: "1 50 1")]
	int m_iTransferBatchSize;

	// Singleton instance
	protected static ref RIP_Config s_Instance;

	static RIP_Config GetInstance()
	{
		if (!s_Instance)
		{
			s_Instance = new RIP_Config();
			s_Instance.SetDefaults();
		}
		return s_Instance;
	}

	void SetDefaults()
	{
		// Phase 1 defaults
		m_bEnableSearch = true;
		m_bEnableCapacityBars = true;
		m_bEnableQuickTransfer = true;
		m_bEnableEnhancedVicinity = true;
		m_fVicinityRange = RIP_Constants.VICINITY_RANGE_DEFAULT;
		m_fVicinityRefreshRate = RIP_Constants.VICINITY_REFRESH_RATE;
		m_iSearchDebounceMs = RIP_Constants.SEARCH_DEBOUNCE_MS;
		m_iTransferBatchSize = RIP_Constants.TRANSFER_BATCH_SIZE;

		// Phase 2 defaults
		m_bEnableQuickDrop = true;
		m_bEnableMagazineConsolidation = true;
	}
}
