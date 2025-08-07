#include "time_model.h"
#include "resources.h"

/**
 * @brief Returns the appropriate battery level icon based on voltage.
 * @param battery_voltage The current battery voltage in volts.
 * @return Pointer to the appropriate LVGL image descriptor for the icon.
 */
const lv_img_dsc_t* get_battery_icon(float battery_voltage)
{
    if (battery_voltage > 4.0)
    {
        return &ic_high_battery; // High battery icon
    }
    else if (battery_voltage > 3.7)
    {
        return &ic_medium_battery; // Medium battery icon
    }
    else if (battery_voltage > 3.4)
    {
        return &ic_low_battery; // Low battery icon
    }
    else
    {
        return &ic_critical_battery; // Critical battery icon
    }
}