#include "Timestamp.h"

/// Returns the local time formatted as ISO‑8601 (YYYY‑MM‑DDTHH:MM:SS).
/// If the time isn’t available yet, an empty string is returned.
void formatLocalTime(char timestamp[], size_t size_of_timestamp)
{
    struct tm timeinfo;
    LOGV("Calling getLocalTime");
    if (!getLocalTime(&timeinfo))
    {
        return;
    }

    // Choose a buffer large enough for the format plus the terminating NUL.
    // constexpr std::size_t kBufSize = 20; // "YYYY-MM-DDTHH:MM:SS" = 19 chars + '\0'
    // static char buf[20];

    // strftime returns the number of characters placed in the buffer (excluding NUL).
    // It will be 0 only on failure (e.g., buffer too small), which we guard against.
    LOGV("Calling strftime()");
    if (strftime(timestamp, size_of_timestamp, "%Y-%m-%dT%H:%M:%S", &timeinfo) == 0)
    {
        return;
    }

    LOGV("Returning %s", timestamp);

    return;
}