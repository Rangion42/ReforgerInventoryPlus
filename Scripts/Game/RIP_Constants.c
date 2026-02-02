// Reforger Inventory Plus - Constants and Enums
// Shared definitions used across all RIP components

// Item categories for quick transfer and filtering
enum RIP_EItemCategory
{
	NONE = 0,
	WEAPONS = 1,
	AMMO = 2,
	MEDICAL = 3,
	EQUIPMENT = 4,
	ALL = 5
};

// Capacity threshold levels for visual indicators
enum RIP_ECapacityLevel
{
	LOW,      // Under 70% - Green
	MEDIUM,   // 70-90% - Yellow
	HIGH      // Over 90% - Red
};

// Transfer direction
enum RIP_ETransferDirection
{
	TO_INVENTORY,
	TO_VICINITY,
	TO_CONTAINER
};

class RIP_Constants
{
	// Capacity thresholds (percentage 0.0 - 1.0)
	static const float CAPACITY_THRESHOLD_MEDIUM = 0.70;
	static const float CAPACITY_THRESHOLD_HIGH = 0.90;

	// Color definitions for capacity bars (ARGB format)
	static const int COLOR_CAPACITY_LOW = ARGB(255, 76, 175, 80);       // Green
	static const int COLOR_CAPACITY_MEDIUM = ARGB(255, 255, 193, 7);    // Yellow/Amber
	static const int COLOR_CAPACITY_HIGH = ARGB(255, 244, 67, 54);      // Red
	static const int COLOR_CAPACITY_BG = ARGB(128, 48, 48, 48);         // Dark gray background

	// Search settings
	static const int SEARCH_DEBOUNCE_MS = 200;
	static const int SEARCH_MIN_CHARS = 1;
	static const int SEARCH_MAX_RESULTS = 500;

	// Vicinity settings
	static const float VICINITY_RANGE_DEFAULT = 3.0;
	static const float VICINITY_RANGE_EXTENDED = 5.0;
	static const int VICINITY_MAX_ITEMS = 200;
	static const float VICINITY_REFRESH_RATE = 0.5;  // seconds

	// UI settings
	static const float CAPACITY_BAR_HEIGHT = 8.0;
	static const float SEARCH_BAR_HEIGHT = 32.0;

	// Transfer batch size - max items per frame to avoid hitching
	static const int TRANSFER_BATCH_SIZE = 10;

	// Weight display
	static const string WEIGHT_UNIT = "kg";
	static const float WEIGHT_DISPLAY_PRECISION = 0.1;

	// Category display names
	static string GetCategoryName(RIP_EItemCategory category)
	{
		switch (category)
		{
			case RIP_EItemCategory.WEAPONS:   return "Weapons";
			case RIP_EItemCategory.AMMO:      return "Ammo";
			case RIP_EItemCategory.MEDICAL:   return "Medical";
			case RIP_EItemCategory.EQUIPMENT: return "Equipment";
			case RIP_EItemCategory.ALL:       return "All";
			default:                          return "Unknown";
		}
	}

	// Get color for capacity level
	static int GetCapacityColor(float ratio)
	{
		if (ratio >= CAPACITY_THRESHOLD_HIGH)
			return COLOR_CAPACITY_HIGH;
		else if (ratio >= CAPACITY_THRESHOLD_MEDIUM)
			return COLOR_CAPACITY_MEDIUM;
		else
			return COLOR_CAPACITY_LOW;
	}

	// Get capacity level enum from ratio
	static RIP_ECapacityLevel GetCapacityLevel(float ratio)
	{
		if (ratio >= CAPACITY_THRESHOLD_HIGH)
			return RIP_ECapacityLevel.HIGH;
		else if (ratio >= CAPACITY_THRESHOLD_MEDIUM)
			return RIP_ECapacityLevel.MEDIUM;
		else
			return RIP_ECapacityLevel.LOW;
	}
}
